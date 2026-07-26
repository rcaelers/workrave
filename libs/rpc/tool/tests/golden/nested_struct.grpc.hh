// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcNested.grpc.pb.h"
#include "nested_struct.hh"



class NestedServiceServiceImpl final : public ::workrave::test::NestedService::Service
{
public:

  explicit NestedServiceServiceImpl(RpcNestedFixture &impl)
    : impl_(impl)
  {
  }



  ::grpc::Status SetTimerData(::grpc::ServerContext *context,
                                 const ::workrave::test::SetTimerDataRequest *request,
                                 ::workrave::test::SetTimerDataResponse *response) override;

  ::grpc::Status GetMenu(::grpc::ServerContext *context,
                                 const ::workrave::test::GetMenuRequest *request,
                                 ::workrave::test::GetMenuResponse *response) override;




private:

  RpcNestedFixture &impl_;

};