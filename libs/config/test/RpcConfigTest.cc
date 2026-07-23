// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

// Live end-to-end test of the gRPC ConfigService generated from the real,
// unmodified workrave::config::IConfigurator interface (see @rpc tags in
// libs/config/include/config/IConfigurator.hh). Unlike libs/rpc/test's
// fixture, this drives the actual Configurator/IniConfigurator classes used
// in production.

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "rpc/RpcServer.hh"

#include "Configurator.hh"
#include "IniConfigurator.hh"
#include "RpcConfig.grpc.pb.h"
#include "RpcConfigServiceImpl.hh"

namespace
{
  class RpcConfigTest : public ::testing::Test
  {
  protected:
    RpcConfigTest()
      : configurator(new IniConfigurator())
      , impl(configurator)
    {
      rpc::ServerConfig config;
      config.listen_address = "127.0.0.1:0";
      rpc_server = std::make_unique<rpc::RpcServer>(config);
      rpc_server->register_service(impl);
      rpc_server->start();

      auto channel = grpc::CreateChannel("127.0.0.1:" + std::to_string(rpc_server->bound_port()),
                                         grpc::InsecureChannelCredentials());
      stub = workrave::rpc::ConfigService::NewStub(channel);
    }

    ~RpcConfigTest() override
    {
      rpc_server->shutdown();
    }

    Configurator configurator;
    ConfigServiceServiceImpl impl;
    std::unique_ptr<rpc::RpcServer> rpc_server;
    std::unique_ptr<workrave::rpc::ConfigService::Stub> stub;
  };
} // namespace

TEST_F(RpcConfigTest, set_get_string_roundtrip)
{
  {
    grpc::ClientContext ctx;
    workrave::rpc::SetStringRequest request;
    request.set_key("test/rpc/string");
    request.set_v("hello");
    workrave::rpc::SetStringResponse response;
    grpc::Status status = stub->SetString(&ctx, request, &response);
    ASSERT_TRUE(status.ok());
  }
  {
    grpc::ClientContext ctx;
    workrave::rpc::GetStringRequest request;
    request.set_key("test/rpc/string");
    workrave::rpc::GetStringResponse response;
    grpc::Status status = stub->GetString(&ctx, request, &response);
    ASSERT_TRUE(status.ok());
    EXPECT_TRUE(response.result());
    EXPECT_EQ(response.out(), "hello");
  }

  // Proves the RPC reached the real, unmodified Configurator — not just the
  // generated adapter echoing something back.
  std::string direct;
  EXPECT_TRUE(configurator.get_value("test/rpc/string", direct));
  EXPECT_EQ(direct, "hello");
}

TEST_F(RpcConfigTest, set_get_int_roundtrip)
{
  {
    grpc::ClientContext ctx;
    workrave::rpc::SetIntRequest request;
    request.set_key("test/rpc/int");
    request.set_v(42);
    workrave::rpc::SetIntResponse response;
    grpc::Status status = stub->SetInt(&ctx, request, &response);
    ASSERT_TRUE(status.ok());
  }
  {
    grpc::ClientContext ctx;
    workrave::rpc::GetIntRequest request;
    request.set_key("test/rpc/int");
    workrave::rpc::GetIntResponse response;
    grpc::Status status = stub->GetInt(&ctx, request, &response);
    ASSERT_TRUE(status.ok());
    EXPECT_TRUE(response.result());
    EXPECT_EQ(response.out(), 42);
  }
}

TEST_F(RpcConfigTest, set_get_bool_roundtrip)
{
  {
    grpc::ClientContext ctx;
    workrave::rpc::SetBoolRequest request;
    request.set_key("test/rpc/bool");
    request.set_v(true);
    workrave::rpc::SetBoolResponse response;
    grpc::Status status = stub->SetBool(&ctx, request, &response);
    ASSERT_TRUE(status.ok());
  }
  {
    grpc::ClientContext ctx;
    workrave::rpc::GetBoolRequest request;
    request.set_key("test/rpc/bool");
    workrave::rpc::GetBoolResponse response;
    grpc::Status status = stub->GetBool(&ctx, request, &response);
    ASSERT_TRUE(status.ok());
    EXPECT_TRUE(response.result());
    EXPECT_TRUE(response.out());
  }
}

TEST_F(RpcConfigTest, get_missing_key_reports_not_found)
{
  grpc::ClientContext ctx;
  workrave::rpc::GetStringRequest request;
  request.set_key("test/rpc/does-not-exist");
  workrave::rpc::GetStringResponse response;
  grpc::Status status = stub->GetString(&ctx, request, &response);
  ASSERT_TRUE(status.ok());
  EXPECT_FALSE(response.result());
}

TEST_F(RpcConfigTest, has_user_value_and_remove_key_reach_the_real_object)
{
  {
    grpc::ClientContext ctx;
    workrave::rpc::HasUserValueRequest request;
    request.set_key("test/rpc/removable");
    workrave::rpc::HasUserValueResponse response;
    grpc::Status status = stub->HasUserValue(&ctx, request, &response);
    ASSERT_TRUE(status.ok());
    EXPECT_FALSE(response.result());
  }
  {
    grpc::ClientContext ctx;
    workrave::rpc::SetStringRequest request;
    request.set_key("test/rpc/removable");
    request.set_v("temporary");
    workrave::rpc::SetStringResponse response;
    ASSERT_TRUE(stub->SetString(&ctx, request, &response).ok());
  }
  {
    grpc::ClientContext ctx;
    workrave::rpc::HasUserValueRequest request;
    request.set_key("test/rpc/removable");
    workrave::rpc::HasUserValueResponse response;
    grpc::Status status = stub->HasUserValue(&ctx, request, &response);
    ASSERT_TRUE(status.ok());
    EXPECT_TRUE(response.result());
  }
  {
    grpc::ClientContext ctx;
    workrave::rpc::RemoveKeyRequest request;
    request.set_key("test/rpc/removable");
    workrave::rpc::RemoveKeyResponse response;
    ASSERT_TRUE(stub->RemoveKey(&ctx, request, &response).ok());
  }

  // Proves RemoveKey reached the real, unmodified Configurator.
  std::string direct;
  EXPECT_FALSE(configurator.get_value("test/rpc/removable", direct));
}

TEST_F(RpcConfigTest, get_string_with_default_falls_back_when_unset)
{
  grpc::ClientContext ctx;
  workrave::rpc::GetStringWithDefaultRequest request;
  request.set_key("test/rpc/does-not-exist");
  request.set_s("fallback");
  workrave::rpc::GetStringWithDefaultResponse response;
  grpc::Status status = stub->GetStringWithDefault(&ctx, request, &response);
  ASSERT_TRUE(status.ok());
  EXPECT_EQ(response.out(), "fallback");
}
