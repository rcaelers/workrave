// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <type_traits>

#include "RpcBreak.grpc.pb.h"
#include "RpcConfig.grpc.pb.h"
#include "RpcCore.grpc.pb.h"

TEST(RpcServiceNames, all_services_share_the_workrave_package)
{
  EXPECT_STREQ(workrave::rpc::ConfigService::service_full_name(), "workrave.ConfigService");
  EXPECT_STREQ(workrave::rpc::BreakService::service_full_name(), "workrave.BreakService");
  EXPECT_STREQ(workrave::rpc::CoreService::service_full_name(), "workrave.CoreService");
}

TEST(RpcServiceNames, generated_payload_types_are_isolated_by_api)
{
  static_assert(std::is_class_v<workrave::rpc::config::SetStringResponse>);
  static_assert(std::is_class_v<workrave::rpc::breaks::GetNameResponse>);
  static_assert(std::is_class_v<workrave::rpc::core::GetActiveOperationModeResponse>);
}
