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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "rpc/InstanceRegistry.hh"
#include "rpc/RequestInterceptor.hh"
#include "rpc/RpcServer.hh"

#include "RpcKeyed.grpc.pb.h"
#include "RpcKeyedServer.hh"
#include "RpcKeyedServiceImpl.hh"
#include "RpcTest.grpc.pb.h"
#include "RpcTestServer.hh"
#include "RpcTestServiceImpl.hh"

namespace
{
  // Client and server coexist in one process against a loopback address —
  // unlike DBus, gRPC needs no system bus daemon to talk to itself.
  class RpcTest : public ::testing::Test
  {
  protected:
    RpcTest()
      : impl(server_object)
    {
      rpc::ServerConfig config;
      config.listen_address = "127.0.0.1:0";
      rpc_server = std::make_unique<rpc::RpcServer>(config);
      rpc_server->register_service(impl);
      rpc_server->start();

      auto channel = grpc::CreateChannel("127.0.0.1:" + std::to_string(rpc_server->bound_port()),
                                         grpc::InsecureChannelCredentials());
      stub = workrave::TestService::NewStub(channel);
    }

    ~RpcTest() override
    {
      rpc_server->shutdown();
    }

    RpcTestServer server_object;
    TestServiceServiceImpl impl;
    std::unique_ptr<rpc::RpcServer> rpc_server;
    std::unique_ptr<workrave::TestService::Stub> stub;
  };
} // namespace

TEST_F(RpcTest, ping_calls_real_method)
{
  grpc::ClientContext ctx;
  workrave::PingRequest request;
  request.set_message("hello");
  workrave::PingResponse response;

  grpc::Status status = stub->Ping(&ctx, request, &response);
  ASSERT_TRUE(status.ok());
  EXPECT_EQ(response.result(), "hello pong");
}

TEST_F(RpcTest, interceptor_observes_successful_generated_calls)
{
  std::string observed_service;
  std::string observed_method;
  std::string observed_message;
  int call_count = 0;
  auto registration = rpc::register_request_interceptor([&](const rpc::RequestInfo &info) {
    observed_service = info.service;
    observed_method = info.method;
    const auto *request = dynamic_cast<const workrave::PingRequest *>(&info.request);
    ASSERT_NE(request, nullptr);
    observed_message = request->message();
    ++call_count;
  });

  grpc::ClientContext first_context;
  workrave::PingRequest request;
  request.set_message("intercept me");
  workrave::PingResponse response;

  ASSERT_TRUE(stub->Ping(&first_context, request, &response).ok());
  EXPECT_EQ(observed_service, "workrave.TestService");
  EXPECT_EQ(observed_method, "Ping");
  EXPECT_EQ(observed_message, "intercept me");
  EXPECT_EQ(call_count, 1);

  registration.reset();
  grpc::ClientContext second_context;
  ASSERT_TRUE(stub->Ping(&second_context, request, &response).ok());
  EXPECT_EQ(call_count, 1);
}

TEST_F(RpcTest, interceptor_failure_does_not_change_the_rpc_result)
{
  auto registration = rpc::register_request_interceptor(
    [](const rpc::RequestInfo &) { throw std::runtime_error("observer failed"); });

  grpc::ClientContext context;
  workrave::PingRequest request;
  request.set_message("still succeeds");
  workrave::PingResponse response;

  ASSERT_TRUE(stub->Ping(&context, request, &response).ok());
  EXPECT_EQ(response.result(), "still succeeds pong");
}

TEST_F(RpcTest, shutdown_rejects_subsequent_calls)
{
  grpc::ClientContext first_context;
  workrave::PingRequest request;
  request.set_message("before shutdown");
  workrave::PingResponse first_response;

  ASSERT_TRUE(stub->Ping(&first_context, request, &first_response).ok());

  rpc_server->shutdown();

  grpc::ClientContext second_context;
  second_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(1));
  workrave::PingResponse second_response;
  EXPECT_FALSE(stub->Ping(&second_context, request, &second_response).ok());
}

TEST_F(RpcTest, add_calls_real_method)
{
  grpc::ClientContext ctx;
  workrave::AddRequest request;
  request.set_a(3);
  request.set_b(4);
  workrave::AddResponse response;

  grpc::Status status = stub->Add(&ctx, request, &response);
  ASSERT_TRUE(status.ok());
  EXPECT_EQ(response.result(), 7);
}

TEST_F(RpcTest, set_flag_reaches_real_object)
{
  EXPECT_FALSE(server_object.get_flag());

  grpc::ClientContext ctx;
  workrave::SetFlagRequest request;
  request.set_value(true);
  workrave::SetFlagResponse response;

  grpc::Status status = stub->SetFlag(&ctx, request, &response);
  ASSERT_TRUE(status.ok());

  // Proves the RPC executed the real, unmodified C++ method server-side —
  // not just that a stub echoed something back over the wire.
  EXPECT_TRUE(server_object.get_flag());
}

TEST_F(RpcTest, get_mode_out_ref_roundtrips)
{
  server_object.set_mode_for_test(TestMode::Active);

  grpc::ClientContext ctx;
  workrave::GetModeRequest request;
  workrave::GetModeResponse response;

  grpc::Status status = stub->GetMode(&ctx, request, &response);
  ASSERT_TRUE(status.ok());
  EXPECT_TRUE(response.result());
  EXPECT_EQ(response.mode(), workrave::TestMode::TEST_MODE_ACTIVE);
}

TEST_F(RpcTest, greet_const_ref_param)
{
  grpc::ClientContext ctx;
  workrave::GreetRequest request;
  request.set_name("world");
  workrave::GreetResponse response;

  grpc::Status status = stub->Greet(&ctx, request, &response);
  ASSERT_TRUE(status.ok());
  EXPECT_EQ(response.result(), "hello, world");
}

TEST_F(RpcTest, mode_changed_signal_streams_real_events)
{
  // The gRPC analog of a DBus signal — see rpc::EventQueue and
  // libs/corenext/src/Core.hh's @rpc.signal-annotated
  // signal_operation_mode_changed() for the real-world use this proves out.
  grpc::ClientContext ctx;
  workrave::ModeChangedRequest request;
  std::unique_ptr<grpc::ClientReaderInterface<workrave::ModeChangedEvent>> reader = stub->ModeChanged(&ctx, request);

  // Give the server handler time to reach signal().connect() before firing —
  // otherwise the signal could fire before anything is subscribed.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  server_object.fire_mode_changed_for_test(TestMode::Active);

  workrave::ModeChangedEvent event;
  ASSERT_TRUE(reader->Read(&event));
  EXPECT_EQ(event.value(), workrave::TestMode::TEST_MODE_ACTIVE);

  // A second firing must produce a second event on the same stream.
  server_object.fire_mode_changed_for_test(TestMode::Idle);
  ASSERT_TRUE(reader->Read(&event));
  EXPECT_EQ(event.value(), workrave::TestMode::TEST_MODE_IDLE);

  ctx.TryCancel();
  (void)reader->Finish();
}

namespace
{
  // Proves the `keyed_by` mechanism: three live RpcKeyedServer instances
  // registered under distinct WidgetIds, all served by a single
  // WidgetServiceServiceImpl — the gRPC analog of DBus routing a call to one
  // of several objects sharing an interface via its object path. Real usage:
  // Workrave's three Break instances (see libs/corenext/src/Break.hh).
  class RpcKeyedTest : public ::testing::Test
  {
  protected:
    RpcKeyedTest()
      : first(1)
      , second(2)
      , third(3)
      , impl(registry)
    {
      registry.register_instance(WidgetId::First, first);
      registry.register_instance(WidgetId::Second, second);
      registry.register_instance(WidgetId::Third, third);

      rpc::ServerConfig config;
      config.listen_address = "127.0.0.1:0";
      rpc_server = std::make_unique<rpc::RpcServer>(config);
      rpc_server->register_service(impl);
      rpc_server->start();

      auto channel = grpc::CreateChannel("127.0.0.1:" + std::to_string(rpc_server->bound_port()),
                                         grpc::InsecureChannelCredentials());
      stub = workrave::WidgetService::NewStub(channel);
    }

    ~RpcKeyedTest() override
    {
      rpc_server->shutdown();
      registry.unregister_instance(WidgetId::First);
      registry.unregister_instance(WidgetId::Second);
      registry.unregister_instance(WidgetId::Third);
    }

    RpcKeyedServer first;
    RpcKeyedServer second;
    RpcKeyedServer third;
    rpc::InstanceRegistry<WidgetId, RpcKeyedServer> registry;
    WidgetServiceServiceImpl impl;
    std::unique_ptr<rpc::RpcServer> rpc_server;
    std::unique_ptr<workrave::WidgetService::Stub> stub;
  };
} // namespace

TEST_F(RpcKeyedTest, get_value_routes_to_correct_instance)
{
  auto get_value = [this](workrave::WidgetId id) {
    grpc::ClientContext ctx;
    workrave::GetValueRequest request;
    request.set_id(id);
    workrave::GetValueResponse response;
    grpc::Status status = stub->GetValue(&ctx, request, &response);
    EXPECT_TRUE(status.ok());
    return response.result();
  };

  EXPECT_EQ(get_value(workrave::WidgetId::WIDGET_ID_FIRST), 1);
  EXPECT_EQ(get_value(workrave::WidgetId::WIDGET_ID_SECOND), 2);
  EXPECT_EQ(get_value(workrave::WidgetId::WIDGET_ID_THIRD), 3);
}

TEST_F(RpcKeyedTest, set_value_only_touches_the_targeted_instance)
{
  grpc::ClientContext ctx;
  workrave::SetValueRequest request;
  request.set_id(workrave::WidgetId::WIDGET_ID_SECOND);
  request.set_v(99);
  workrave::SetValueResponse response;
  grpc::Status status = stub->SetValue(&ctx, request, &response);
  ASSERT_TRUE(status.ok());

  // Reached the real `second` object, and left the other two untouched —
  // proves resolution isn't just returning the first/any registered instance.
  EXPECT_EQ(second.get_value(), 99);
  EXPECT_EQ(first.get_value(), 1);
  EXPECT_EQ(third.get_value(), 3);
}

TEST_F(RpcKeyedTest, value_changed_signal_routes_to_subscribed_instance_only)
{
  // Subscribe to `second` only.
  grpc::ClientContext ctx;
  workrave::ValueChangedRequest request;
  request.set_id(workrave::WidgetId::WIDGET_ID_SECOND);
  std::unique_ptr<grpc::ClientReaderInterface<workrave::ValueChangedEvent>> reader = stub->ValueChanged(&ctx, request);

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Fire on `first` and `third` too — must NOT show up on this stream.
  first.fire_value_changed_for_test(111);
  third.fire_value_changed_for_test(333);
  second.fire_value_changed_for_test(222);

  workrave::ValueChangedEvent event;
  ASSERT_TRUE(reader->Read(&event));
  EXPECT_EQ(event.value(), 222);

  ctx.TryCancel();
  (void)reader->Finish();
}
