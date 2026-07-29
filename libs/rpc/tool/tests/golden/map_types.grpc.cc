// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcMapTypesServiceImpl.hh"

#include <exception>

#include "rpc/RequestInterceptor.hh"



::grpc::Status MapTypesServiceServiceImpl::SetCounters(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::SetCountersRequest *request,
                                                            ::workrave::test::SetCountersResponse *response)
{
  try
    {


      std::map<std::string, int> local_counters{};

      for (const auto &rpc_kv_0 : request->counters()) { int32_t rpc_val_0{}; rpc_val_0 = rpc_kv_0.second; local_counters.emplace(rpc_kv_0.first, rpc_val_0); }


      impl_.set_counters(local_counters);


      ::rpc::intercept_request({"workrave.test.MapTypesService", "SetCounters", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status MapTypesServiceServiceImpl::GetMenuByAction(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::GetMenuByActionRequest *request,
                                                            ::workrave::test::GetMenuByActionResponse *response)
{
  try
    {


      std::map<std::string, MenuItem> local_out{};


      impl_.get_menu_by_action(local_out);


      for (const auto &rpc_kv_0 : local_out) { auto &rpc_map_val_0 = (*response->mutable_out())[rpc_kv_0.first]; rpc_map_val_0.set_text(rpc_kv_0.second.text); rpc_map_val_0.set_command(rpc_kv_0.second.command); }

      ::rpc::intercept_request({"workrave.test.MapTypesService", "GetMenuByAction", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


