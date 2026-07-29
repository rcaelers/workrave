// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string_view>

namespace google::protobuf
{
  class Message;
}

namespace rpc
{
  struct RequestInfo
  {
    std::string_view service;
    std::string_view method;
    const google::protobuf::Message &request;
  };

  using RequestInterceptor = std::function<void(const RequestInfo &)>;

  namespace detail
  {
    struct RequestInterceptorEntry;
  }

  // Owns one process-local interceptor registration. Destroying or resetting
  // it waits for callbacks already in progress, so callbacks may safely refer
  // to state owned by the registration's holder.
  class RequestInterceptorRegistration
  {
  public:
    RequestInterceptorRegistration() = default;
    ~RequestInterceptorRegistration();

    RequestInterceptorRegistration(RequestInterceptorRegistration &&other) noexcept;
    RequestInterceptorRegistration &operator=(RequestInterceptorRegistration &&other) noexcept;

    RequestInterceptorRegistration(const RequestInterceptorRegistration &) = delete;
    RequestInterceptorRegistration &operator=(const RequestInterceptorRegistration &) = delete;

    void reset();
    [[nodiscard]] explicit operator bool() const noexcept;

  private:
    explicit RequestInterceptorRegistration(std::shared_ptr<detail::RequestInterceptorEntry> entry);

    std::shared_ptr<detail::RequestInterceptorEntry> entry_;

    friend RequestInterceptorRegistration register_request_interceptor(RequestInterceptor interceptor);
  };

  [[nodiscard]] RequestInterceptorRegistration register_request_interceptor(RequestInterceptor interceptor);

  // Called by generated adapters after the real C++ method completed
  // successfully. Interceptor failures never change the primary RPC result.
  void intercept_request(const RequestInfo &request) noexcept;
} // namespace rpc
