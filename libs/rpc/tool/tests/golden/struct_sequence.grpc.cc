// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcStructSeqServiceImpl.hh"

#include <exception>

#include "rpc/RequestInterceptor.hh"



::grpc::Status StructSeqServiceServiceImpl::SetTimerData(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::SetTimerDataRequest *request,
                                                            ::workrave::test::SetTimerDataResponse *response)
{
  try
    {


      TimerData local_data{};

      local_data.bar_text = request->data().bar_text();

      local_data.slot = request->data().slot();

      local_data.bar_primary_val = request->data().bar_primary_val();


      impl_.set_timer_data(local_data);


      ::rpc::intercept_request({"workrave.test.StructSeqService", "SetTimerData", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status StructSeqServiceServiceImpl::GetTimerData(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::GetTimerDataRequest *request,
                                                            ::workrave::test::GetTimerDataResponse *response)
{
  try
    {



      auto rpc_result = impl_.get_timer_data();

      auto *rpc_msg_0 = response->mutable_result();

      rpc_msg_0->set_bar_text(rpc_result.bar_text); rpc_msg_0->set_slot(rpc_result.slot); rpc_msg_0->set_bar_primary_val(rpc_result.bar_primary_val);



      ::rpc::intercept_request({"workrave.test.StructSeqService", "GetTimerData", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status StructSeqServiceServiceImpl::GetMenu(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::GetMenuRequest *request,
                                                            ::workrave::test::GetMenuResponse *response)
{
  try
    {


      std::list<MenuItem> local_out{};


      impl_.get_menu(local_out);


      for (const auto &rpc_item_0 : local_out) { auto *rpc_elem_0 = response->add_out(); rpc_elem_0->set_text(rpc_item_0.text); rpc_elem_0->set_command(rpc_item_0.command); rpc_elem_0->set_flags(rpc_item_0.flags); }

      ::rpc::intercept_request({"workrave.test.StructSeqService", "GetMenu", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status StructSeqServiceServiceImpl::SetTags(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::SetTagsRequest *request,
                                                            ::workrave::test::SetTagsResponse *response)
{
  try
    {


      std::vector<int> local_tags{};

      for (const auto &rpc_wire_0 : request->tags()) { int32_t rpc_item_0{}; rpc_item_0 = rpc_wire_0; local_tags.push_back(rpc_item_0); }


      impl_.set_tags(local_tags);


      ::rpc::intercept_request({"workrave.test.StructSeqService", "SetTags", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}



::grpc::Status StructSeqServiceServiceImpl::TimerUpdated(::grpc::ServerContext *context,
                                                            const ::workrave::test::TimerUpdatedRequest */*request*/,
                                                            ::grpc::ServerWriter<::workrave::test::TimerUpdatedEvent> *writer)
{

  ::rpc::EventQueue<::workrave::test::TimerUpdatedEvent> queue;
  auto connection = impl_.signal_timer_updated().connect(
    [&queue](TimerData value)
    {
      ::workrave::test::TimerUpdatedEvent event;

      auto *rpc_msg_0 = event.mutable_value();

      rpc_msg_0->set_bar_text(value.bar_text); rpc_msg_0->set_slot(value.slot); rpc_msg_0->set_bar_primary_val(value.bar_primary_val);

      queue.push(event);
    });

  ::workrave::test::TimerUpdatedEvent event;
  while (queue.wait_and_pop(event, context))
    {
      if (!writer->Write(event))
        {
          break;
        }
    }

  connection.disconnect();
  return ::grpc::Status::OK;
}

