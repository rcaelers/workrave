// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcEnumNames.grpc.pb.h"
#include "enum_names.hh"



class EnumNamesService final : public ::workrave::test::EnumNamesService::Service
{
public:

  explicit EnumNamesService(RpcEnumNamesFixture &impl)
    : impl_(impl)
  {
  }



  ::grpc::Status SetOperationMode(::grpc::ServerContext *context,
                                 const ::workrave::test::SetOperationModeRequest *request,
                                 ::workrave::test::SetOperationModeResponse *response) override;




private:

  RpcEnumNamesFixture &impl_;

};