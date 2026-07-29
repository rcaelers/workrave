// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcCoreServiceImpl.hh"

#include <exception>

#include "rpc/RequestInterceptor.hh"

#include "rpc/Duration.hh"



namespace workrave::core::rpc
{


CoreService::CoreService(Core &impl)
  : impl_(impl)
  , service_descriptor_anchor_(&::descriptor_table_RpcCore_2eproto)
{
}



::grpc::Status CoreService::IsActive(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::core::IsActiveRequest *request,
                                                            ::workrave::core::IsActiveResponse *response)
{
  try
    {



      auto rpc_result = impl_.is_user_active();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.CoreService", "IsActive", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreService::IsTaking(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::core::IsTakingRequest *request,
                                                            ::workrave::core::IsTakingResponse *response)
{
  try
    {



      auto rpc_result = impl_.is_taking();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.CoreService", "IsTaking", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreService::ForceBreak(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::core::ForceBreakRequest *request,
                                                            ::workrave::core::ForceBreakResponse *response)
{
  try
    {


      ::workrave::utils::Flags<::workrave::BreakHint> local_break_hint;

      for (int i = 0; i < request->break_hint_size(); ++i) { local_break_hint |= static_cast<workrave::BreakHint>(request->break_hint(i)); }


      impl_.force_break(static_cast<workrave::BreakId>(request->id()), local_break_hint);


      ::rpc::intercept_request({"workrave.CoreService", "ForceBreak", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreService::GetActiveOperationMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::core::GetActiveOperationModeRequest *request,
                                                            ::workrave::core::GetActiveOperationModeResponse *response)
{
  try
    {



      auto rpc_result = impl_.get_active_operation_mode();

      response->set_result(static_cast<::workrave::core::OperationMode>(rpc_result));



      ::rpc::intercept_request({"workrave.CoreService", "GetActiveOperationMode", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreService::GetOperationMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::core::GetOperationModeRequest *request,
                                                            ::workrave::core::GetOperationModeResponse *response)
{
  try
    {



      auto rpc_result = impl_.get_regular_operation_mode();

      response->set_result(static_cast<::workrave::core::OperationMode>(rpc_result));



      ::rpc::intercept_request({"workrave.CoreService", "GetOperationMode", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreService::IsOperationModeAnOverride(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::core::IsOperationModeAnOverrideRequest *request,
                                                            ::workrave::core::IsOperationModeAnOverrideResponse *response)
{
  try
    {



      auto rpc_result = impl_.is_operation_mode_an_override();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.CoreService", "IsOperationModeAnOverride", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreService::SetOperationMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::core::SetOperationModeRequest *request,
                                                            ::workrave::core::SetOperationModeResponse *response)
{
  try
    {



      impl_.set_operation_mode(static_cast<workrave::OperationMode>(request->mode()));


      ::rpc::intercept_request({"workrave.CoreService", "SetOperationMode", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreService::SetOperationModeFor(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::core::SetOperationModeForRequest *request,
                                                            ::workrave::core::SetOperationModeForResponse *response)
{
  try
    {



      impl_.set_operation_mode_for(static_cast<workrave::OperationMode>(request->mode()), std::chrono::duration_cast<std::chrono::minutes>(::rpc::parse_duration(request->duration())));


      ::rpc::intercept_request({"workrave.CoreService", "SetOperationModeFor", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreService::GetUsageMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::core::GetUsageModeRequest *request,
                                                            ::workrave::core::GetUsageModeResponse *response)
{
  try
    {



      auto rpc_result = impl_.get_usage_mode();

      response->set_result(static_cast<::workrave::core::UsageMode>(rpc_result));



      ::rpc::intercept_request({"workrave.CoreService", "GetUsageMode", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreService::SetUsageMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::core::SetUsageModeRequest *request,
                                                            ::workrave::core::SetUsageModeResponse *response)
{
  try
    {



      impl_.set_usage_mode(static_cast<workrave::UsageMode>(request->mode()));


      ::rpc::intercept_request({"workrave.CoreService", "SetUsageMode", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status CoreService::ReportActivity(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::core::ReportActivityRequest *request,
                                                            ::workrave::core::ReportActivityResponse *response)
{
  try
    {



      impl_.report_external_activity(request->who(), request->act());


      ::rpc::intercept_request({"workrave.CoreService", "ReportActivity", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}



::grpc::Status CoreService::OperationModeChanged(::grpc::ServerContext *context,
                                                            const ::workrave::core::OperationModeChangedRequest */*request*/,
                                                            ::grpc::ServerWriter<::workrave::core::OperationModeChangedEvent> *writer)
{

  ::rpc::EventQueue<::workrave::core::OperationModeChangedEvent> queue;
  auto connection = impl_.signal_operation_mode_changed().connect(
    [&queue](workrave::OperationMode value)
    {
      ::workrave::core::OperationModeChangedEvent event;

      event.set_value(static_cast<::workrave::core::OperationMode>(value));

      queue.push(event);
    });

  ::workrave::core::OperationModeChangedEvent event;
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


::grpc::Status CoreService::UsageModeChanged(::grpc::ServerContext *context,
                                                            const ::workrave::core::UsageModeChangedRequest */*request*/,
                                                            ::grpc::ServerWriter<::workrave::core::UsageModeChangedEvent> *writer)
{

  ::rpc::EventQueue<::workrave::core::UsageModeChangedEvent> queue;
  auto connection = impl_.signal_usage_mode_changed().connect(
    [&queue](workrave::UsageMode value)
    {
      ::workrave::core::UsageModeChangedEvent event;

      event.set_value(static_cast<::workrave::core::UsageMode>(value));

      queue.push(event);
    });

  ::workrave::core::UsageModeChangedEvent event;
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
