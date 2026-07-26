// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcNestedServiceImpl.hh"

#include <exception>



::grpc::Status NestedServiceServiceImpl::SetTimerData(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::SetTimerDataRequest *request,
                                                            ::workrave::test::SetTimerDataResponse *response)
{
  try
    {


      RpcNestedFixture::TimerData local_data{};

      local_data.bar_text = request->data().bar_text();

      local_data.slot = request->data().slot();

      local_data.bar_primary_val = request->data().bar_primary_val();


      impl_.set_timer_data(local_data);


    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status NestedServiceServiceImpl::GetMenu(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::GetMenuRequest *request,
                                                            ::workrave::test::GetMenuResponse *response)
{
  try
    {


      std::list<RpcNestedFixture::MenuItem> local_out{};


      impl_.get_menu(local_out);


      for (const auto &rpc_item_0 : local_out) { auto *rpc_elem_0 = response->add_out(); rpc_elem_0->set_text(rpc_item_0.text); rpc_elem_0->set_command(rpc_item_0.command); rpc_elem_0->set_flags(rpc_item_0.flags); }

    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


