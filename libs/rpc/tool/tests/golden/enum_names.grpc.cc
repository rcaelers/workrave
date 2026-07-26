// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcEnumNamesServiceImpl.hh"

#include <exception>



::grpc::Status EnumNamesServiceServiceImpl::SetOperationMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::SetOperationModeRequest *request,
                                                            ::workrave::test::SetOperationModeResponse *response)
{
  try
    {



      impl_.set_operation_mode(static_cast<OperationMode>(request->mode()));


    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


