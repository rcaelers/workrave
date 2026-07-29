// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcKeyed.grpc.pb.h"
#include "keyed.hh"

#include "rpc/InstanceRegistry.hh"


#include "rpc/EventQueue.hh"


class WidgetService final : public ::workrave::test::WidgetService::Service
{
public:

  // RpcKeyedFixture has multiple live instances; the caller supplies a
  // registry that instances register themselves into (see rpc::InstanceRegistry),
  // and each RPC resolves its target from the request's `id` field — the gRPC
  // analog of DBus's per-object-path routing.
  explicit WidgetService(::rpc::InstanceRegistry<WidgetId, RpcKeyedFixture> &registry)
    : registry_(registry)
  {
  }



  ::grpc::Status GetValue(::grpc::ServerContext *context,
                                 const ::workrave::test::GetValueRequest *request,
                                 ::workrave::test::GetValueResponse *response) override;

  ::grpc::Status SetValue(::grpc::ServerContext *context,
                                 const ::workrave::test::SetValueRequest *request,
                                 ::workrave::test::SetValueResponse *response) override;



  ::grpc::Status ValueChanged(::grpc::ServerContext *context,
                                 const ::workrave::test::ValueChangedRequest *request,
                                 ::grpc::ServerWriter<::workrave::test::ValueChangedEvent> *writer) override;


private:

  ::rpc::InstanceRegistry<WidgetId, RpcKeyedFixture> &registry_;

};