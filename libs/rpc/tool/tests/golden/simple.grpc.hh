// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcTest.grpc.pb.h"
#include "simple.hh"


#include "rpc/EventQueue.hh"


class TestService final : public ::workrave::test::TestService::Service
{
public:

  explicit TestService(RpcTestServer &impl)
    : impl_(impl)
  {
  }



  ::grpc::Status Ping(::grpc::ServerContext *context,
                                 const ::workrave::test::PingRequest *request,
                                 ::workrave::test::PingResponse *response) override;

  ::grpc::Status Add(::grpc::ServerContext *context,
                                 const ::workrave::test::AddRequest *request,
                                 ::workrave::test::AddResponse *response) override;

  ::grpc::Status SetFlag(::grpc::ServerContext *context,
                                 const ::workrave::test::SetFlagRequest *request,
                                 ::workrave::test::SetFlagResponse *response) override;

  ::grpc::Status GetMode(::grpc::ServerContext *context,
                                 const ::workrave::test::GetModeRequest *request,
                                 ::workrave::test::GetModeResponse *response) override;

  ::grpc::Status Greet(::grpc::ServerContext *context,
                                 const ::workrave::test::GreetRequest *request,
                                 ::workrave::test::GreetResponse *response) override;



  ::grpc::Status ModeChanged(::grpc::ServerContext *context,
                                 const ::workrave::test::ModeChangedRequest *request,
                                 ::grpc::ServerWriter<::workrave::test::ModeChangedEvent> *writer) override;


private:

  RpcTestServer &impl_;

};