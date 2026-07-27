// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcCoreServiceImpl.hh"

#include <exception>

#include "rpc/Duration.hh"



namespace workrave::core::rpc
{


CoreServiceServiceImpl::CoreServiceServiceImpl(Core &impl)
  : impl_(impl)
  , service_descriptor_anchor_(&::descriptor_table_RpcCore_2eproto)
{
}



::grpc::Status CoreServiceServiceImpl::ForceBreak(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::core::ForceBreakRequest *request,
                                                            ::workrave::rpc::core::ForceBreakResponse *response)
{
  try
    {


      workrave::utils::Flags<workrave::BreakHint> local_break_hint;

      for (int i = 0; i < request->break_hint_size(); ++i) { local_break_hint |= static_cast<workrave::BreakHint>(request->break_hint(i)); }


      impl_.force_break(static_cast<workrave::BreakId>(request->id()), local_break_hint);


    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreServiceServiceImpl::IsActive(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::core::IsActiveRequest *request,
                                                            ::workrave::rpc::core::IsActiveResponse *response)
{
  try
    {



      auto rpc_result = impl_.is_user_active();

      response->set_result(rpc_result);



    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreServiceServiceImpl::IsTaking(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::core::IsTakingRequest *request,
                                                            ::workrave::rpc::core::IsTakingResponse *response)
{
  try
    {



      auto rpc_result = impl_.is_taking();

      response->set_result(rpc_result);



    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreServiceServiceImpl::GetActiveOperationMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::core::GetActiveOperationModeRequest *request,
                                                            ::workrave::rpc::core::GetActiveOperationModeResponse *response)
{
  try
    {



      auto rpc_result = impl_.get_active_operation_mode();

      response->set_result(static_cast<::workrave::rpc::core::OperationMode>(rpc_result));



    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreServiceServiceImpl::GetOperationMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::core::GetOperationModeRequest *request,
                                                            ::workrave::rpc::core::GetOperationModeResponse *response)
{
  try
    {



      auto rpc_result = impl_.get_regular_operation_mode();

      response->set_result(static_cast<::workrave::rpc::core::OperationMode>(rpc_result));



    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreServiceServiceImpl::IsOperationModeAnOverride(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::core::IsOperationModeAnOverrideRequest *request,
                                                            ::workrave::rpc::core::IsOperationModeAnOverrideResponse *response)
{
  try
    {



      auto rpc_result = impl_.is_operation_mode_an_override();

      response->set_result(rpc_result);



    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreServiceServiceImpl::SetOperationMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::core::SetOperationModeRequest *request,
                                                            ::workrave::rpc::core::SetOperationModeResponse *response)
{
  try
    {



      impl_.set_operation_mode(static_cast<workrave::OperationMode>(request->mode()));


    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreServiceServiceImpl::SetOperationModeFor(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::core::SetOperationModeForRequest *request,
                                                            ::workrave::rpc::core::SetOperationModeForResponse *response)
{
  try
    {



      impl_.set_operation_mode_for(static_cast<workrave::OperationMode>(request->mode()), std::chrono::duration_cast<std::chrono::minutes>(::rpc::parse_duration(request->duration())));


    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreServiceServiceImpl::GetUsageMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::core::GetUsageModeRequest *request,
                                                            ::workrave::rpc::core::GetUsageModeResponse *response)
{
  try
    {



      auto rpc_result = impl_.get_usage_mode();

      response->set_result(static_cast<::workrave::rpc::core::UsageMode>(rpc_result));



    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreServiceServiceImpl::SetUsageMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::core::SetUsageModeRequest *request,
                                                            ::workrave::rpc::core::SetUsageModeResponse *response)
{
  try
    {



      impl_.set_usage_mode(static_cast<workrave::UsageMode>(request->mode()));


    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreServiceServiceImpl::ReportActivity(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::core::ReportActivityRequest *request,
                                                            ::workrave::rpc::core::ReportActivityResponse *response)
{
  try
    {



      impl_.report_external_activity(request->who(), request->act());


    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}



::grpc::Status CoreServiceServiceImpl::OperationModeChanged(::grpc::ServerContext *context,
                                                            const ::workrave::rpc::core::OperationModeChangedRequest */*request*/,
                                                            ::grpc::ServerWriter<::workrave::rpc::core::OperationModeChangedEvent> *writer)
{

  ::rpc::EventQueue<::workrave::rpc::core::OperationModeChangedEvent> queue;
  auto connection = impl_.signal_operation_mode_changed().connect(
    [&queue](workrave::OperationMode value)
    {
      ::workrave::rpc::core::OperationModeChangedEvent event;

      event.set_value(static_cast<::workrave::rpc::core::OperationMode>(value));

      queue.push(event);
    });

  ::workrave::rpc::core::OperationModeChangedEvent event;
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


::grpc::Status CoreServiceServiceImpl::UsageModeChanged(::grpc::ServerContext *context,
                                                            const ::workrave::rpc::core::UsageModeChangedRequest */*request*/,
                                                            ::grpc::ServerWriter<::workrave::rpc::core::UsageModeChangedEvent> *writer)
{

  ::rpc::EventQueue<::workrave::rpc::core::UsageModeChangedEvent> queue;
  auto connection = impl_.signal_usage_mode_changed().connect(
    [&queue](workrave::UsageMode value)
    {
      ::workrave::rpc::core::UsageModeChangedEvent event;

      event.set_value(static_cast<::workrave::rpc::core::UsageMode>(value));

      queue.push(event);
    });

  ::workrave::rpc::core::UsageModeChangedEvent event;
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



} // namespace workrave::core::rpc
