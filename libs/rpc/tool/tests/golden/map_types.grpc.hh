// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcMapTypes.grpc.pb.h"
#include "map_types.hh"



class MapTypesService final : public ::workrave::test::MapTypesService::Service
{
public:

  explicit MapTypesService(RpcMapTypesFixture &impl)
    : impl_(impl)
  {
  }



  ::grpc::Status SetCounters(::grpc::ServerContext *context,
                                 const ::workrave::test::SetCountersRequest *request,
                                 ::workrave::test::SetCountersResponse *response) override;

  ::grpc::Status GetMenuByAction(::grpc::ServerContext *context,
                                 const ::workrave::test::GetMenuByActionRequest *request,
                                 ::workrave::test::GetMenuByActionResponse *response) override;




private:

  RpcMapTypesFixture &impl_;

};