// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcDurationFlags.grpc.pb.h"
#include "duration_flags.hh"



class DurationFlagsServiceServiceImpl final : public ::workrave::test::DurationFlagsService::Service
{
public:

  explicit DurationFlagsServiceServiceImpl(RpcDurationFlagsFixture &impl)
    : impl_(impl)
  {
  }



  ::grpc::Status SetTimeout(::grpc::ServerContext *context,
                                 const ::workrave::test::SetTimeoutRequest *request,
                                 ::workrave::test::SetTimeoutResponse *response) override;

  ::grpc::Status SetPermissions(::grpc::ServerContext *context,
                                 const ::workrave::test::SetPermissionsRequest *request,
                                 ::workrave::test::SetPermissionsResponse *response) override;




private:

  RpcDurationFlagsFixture &impl_;

};