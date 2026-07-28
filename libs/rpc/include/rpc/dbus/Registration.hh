// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <utility>

namespace workrave::rpc::dbus
{
  class Registration
  {
  public:
    Registration() = default;

    explicit Registration(std::function<void()> release)
      : release_(std::move(release))
    {
    }

    ~Registration()
    {
      reset();
    }

    Registration(const Registration &) = delete;
    Registration &operator=(const Registration &) = delete;

    Registration(Registration &&other) noexcept
      : release_(std::exchange(other.release_, {}))
    {
    }

    Registration &operator=(Registration &&other) noexcept
    {
      if (this != &other)
        {
          reset();
          release_ = std::exchange(other.release_, {});
        }
      return *this;
    }

    explicit operator bool() const noexcept
    {
      return static_cast<bool>(release_);
    }

    void reset() noexcept
    {
      auto release = std::exchange(release_, {});
      if (release)
        {
          try
            {
              release();
            }
          catch (...)
            {
              // Registration teardown must remain safe from destructors.
            }
        }
    }

  private:
    std::function<void()> release_;
  };
}
