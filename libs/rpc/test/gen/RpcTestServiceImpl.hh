// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcTest.grpc.pb.h"
#include "RpcTestServer.hh"


#include "rpc/EventQueue.hh"


class TestService final : public ::workrave::TestService::Service
{
public:

  explicit TestService(RpcTestServer &impl)
    : impl_(impl)
  {
  }



  ::grpc::Status Ping(::grpc::ServerContext *context,
                                 const ::workrave::PingRequest *request,
                                 ::workrave::PingResponse *response) override;

  ::grpc::Status Add(::grpc::ServerContext *context,
                                 const ::workrave::AddRequest *request,
                                 ::workrave::AddResponse *response) override;

  ::grpc::Status SetFlag(::grpc::ServerContext *context,
                                 const ::workrave::SetFlagRequest *request,
                                 ::workrave::SetFlagResponse *response) override;

  ::grpc::Status GetMode(::grpc::ServerContext *context,
                                 const ::workrave::GetModeRequest *request,
                                 ::workrave::GetModeResponse *response) override;

  ::grpc::Status Greet(::grpc::ServerContext *context,
                                 const ::workrave::GreetRequest *request,
                                 ::workrave::GreetResponse *response) override;



  ::grpc::Status ModeChanged(::grpc::ServerContext *context,
                                 const ::workrave::ModeChangedRequest *request,
                                 ::grpc::ServerWriter<::workrave::ModeChangedEvent> *writer) override;


private:

  RpcTestServer &impl_;

};