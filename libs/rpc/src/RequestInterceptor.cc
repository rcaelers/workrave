// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rpc/RequestInterceptor.hh"

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <utility>
#include <vector>

namespace rpc
{
  namespace detail
  {
    struct RequestInterceptorEntry
    {
      explicit RequestInterceptorEntry(RequestInterceptor request_interceptor)
        : interceptor(std::move(request_interceptor))
      {
      }

      RequestInterceptor interceptor;
      std::mutex mutex;
      std::condition_variable idle;
      std::size_t active_callbacks{0};
      bool enabled{true};
    };
  } // namespace detail

  namespace
  {
    struct InterceptorRegistry
    {
      std::mutex mutex;
      std::vector<std::weak_ptr<detail::RequestInterceptorEntry>> entries;
    };

    auto registry() -> InterceptorRegistry &
    {
      static InterceptorRegistry instance;
      return instance;
    }
  } // namespace

  RequestInterceptorRegistration::RequestInterceptorRegistration(std::shared_ptr<detail::RequestInterceptorEntry> entry)
    : entry_(std::move(entry))
  {
  }

  RequestInterceptorRegistration::~RequestInterceptorRegistration()
  {
    reset();
  }

  RequestInterceptorRegistration::RequestInterceptorRegistration(RequestInterceptorRegistration &&other) noexcept
    : entry_(std::move(other.entry_))
  {
  }

  RequestInterceptorRegistration &RequestInterceptorRegistration::operator=(RequestInterceptorRegistration &&other) noexcept
  {
    if (this != &other)
      {
        reset();
        entry_ = std::move(other.entry_);
      }
    return *this;
  }

  void RequestInterceptorRegistration::reset()
  {
    if (!entry_)
      {
        return;
      }

    auto &interceptors = registry();
    {
      std::scoped_lock lock(interceptors.mutex);
      std::erase_if(interceptors.entries, [entry = entry_](const auto &candidate) {
        const auto registered = candidate.lock();
        return !registered || registered == entry;
      });
    }

    {
      std::unique_lock lock(entry_->mutex);
      entry_->enabled = false;
      entry_->idle.wait(lock, [this] { return entry_->active_callbacks == 0; });
    }
    entry_.reset();
  }

  RequestInterceptorRegistration::operator bool() const noexcept
  {
    return entry_ != nullptr;
  }

  RequestInterceptorRegistration register_request_interceptor(RequestInterceptor interceptor)
  {
    auto entry = std::make_shared<detail::RequestInterceptorEntry>(std::move(interceptor));
    auto &interceptors = registry();
    {
      std::scoped_lock lock(interceptors.mutex);
      interceptors.entries.emplace_back(entry);
    }
    return RequestInterceptorRegistration{std::move(entry)};
  }

  void intercept_request(const RequestInfo &request) noexcept
  {
    std::vector<std::shared_ptr<detail::RequestInterceptorEntry>> entries;
    auto &interceptors = registry();
    {
      std::scoped_lock lock(interceptors.mutex);
      std::erase_if(interceptors.entries, [](const auto &entry) { return entry.expired(); });
      entries.reserve(interceptors.entries.size());
      for (const auto &entry: interceptors.entries)
        {
          if (auto active = entry.lock())
            {
              entries.push_back(std::move(active));
            }
        }
    }

    for (const auto &entry: entries)
      {
        {
          std::scoped_lock lock(entry->mutex);
          if (!entry->enabled)
            {
              continue;
            }
          ++entry->active_callbacks;
        }

        try
          {
            entry->interceptor(request);
          }
        catch (...)
          {
            // Shadowing, diagnostics, and other interception users must never
            // alter a successful call to the real implementation.
          }

        {
          std::scoped_lock lock(entry->mutex);
          --entry->active_callbacks;
          if (entry->active_callbacks == 0)
            {
              entry->idle.notify_all();
            }
        }
      }
  }
} // namespace rpc
