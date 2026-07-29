// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcDurationFlagsServiceImpl.hh"

#include <exception>

#include "rpc/RequestInterceptor.hh"

#include "rpc/Duration.hh"



::grpc::Status DurationFlagsServiceServiceImpl::SetTimeout(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::SetTimeoutRequest *request,
                                                            ::workrave::test::SetTimeoutResponse *response)
{
  try
    {



      impl_.set_timeout(std::chrono::duration_cast<std::chrono::minutes>(::rpc::parse_duration(request->duration())));


      ::rpc::intercept_request({"workrave.test.DurationFlagsService", "SetTimeout", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status DurationFlagsServiceServiceImpl::SetPermissions(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::SetPermissionsRequest *request,
                                                            ::workrave::test::SetPermissionsResponse *response)
{
  try
    {


      testutil::Flags<testutil::Perm> local_perms;

      for (int i = 0; i < request->perms_size(); ++i) { local_perms |= static_cast<testutil::Perm>(request->perms(i)); }


      impl_.set_permissions(local_perms);


      ::rpc::intercept_request({"workrave.test.DurationFlagsService", "SetPermissions", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


