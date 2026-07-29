// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcDbusScalar.grpc.pb.h"
#include "dbus_scalar.hh"


#include "rpc/EventQueue.hh"


class DBusFixtureService final : public ::workrave::test::DBusFixtureService::Service
{
public:

  explicit DBusFixtureService(RpcDBusFixture &impl)
    : impl_(impl)
  {
  }



  ::grpc::Status Ping(::grpc::ServerContext *context,
                                 const ::workrave::test::PingRequest *request,
                                 ::workrave::test::PingResponse *response) override;

  ::grpc::Status GetMode(::grpc::ServerContext *context,
                                 const ::workrave::test::GetModeRequest *request,
                                 ::workrave::test::GetModeResponse *response) override;

  ::grpc::Status SetMode(::grpc::ServerContext *context,
                                 const ::workrave::test::SetModeRequest *request,
                                 ::workrave::test::SetModeResponse *response) override;



  ::grpc::Status ModeChanged(::grpc::ServerContext *context,
                                 const ::workrave::test::ModeChangedRequest *request,
                                 ::grpc::ServerWriter<::workrave::test::ModeChangedEvent> *writer) override;


private:

  RpcDBusFixture &impl_;

};