// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcKeyed.grpc.pb.h"
#include "RpcKeyedServer.hh"

#include "rpc/InstanceRegistry.hh"


#include "rpc/EventQueue.hh"


class WidgetService final : public ::workrave::WidgetService::Service
{
public:

  // RpcKeyedServer has multiple live instances; the caller supplies a
  // registry that instances register themselves into (see rpc::InstanceRegistry),
  // and each RPC resolves its target from the request's `id` field — the gRPC
  // analog of DBus's per-object-path routing.
  explicit WidgetService(::rpc::InstanceRegistry<WidgetId, RpcKeyedServer> &registry)
    : registry_(registry)
  {
  }



  ::grpc::Status GetValue(::grpc::ServerContext *context,
                                 const ::workrave::GetValueRequest *request,
                                 ::workrave::GetValueResponse *response) override;

  ::grpc::Status SetValue(::grpc::ServerContext *context,
                                 const ::workrave::SetValueRequest *request,
                                 ::workrave::SetValueResponse *response) override;



  ::grpc::Status ValueChanged(::grpc::ServerContext *context,
                                 const ::workrave::ValueChangedRequest *request,
                                 ::grpc::ServerWriter<::workrave::ValueChangedEvent> *writer) override;


private:

  ::rpc::InstanceRegistry<WidgetId, RpcKeyedServer> &registry_;

};