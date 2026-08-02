// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcBreakServiceImpl.hh"

#include <exception>

#include "rpc/RequestInterceptor.hh"



namespace workrave::core::rpc
{


BreakService::BreakService(::rpc::InstanceRegistry<workrave::BreakId, Break> &registry)
  : registry_(registry)
  , service_descriptor_anchor_(&::descriptor_table_RpcBreak_2eproto)
{
}



::grpc::Status BreakService::GetName(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::GetNameRequest *request,
                                                            ::workrave::breaks::GetNameResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.get_name();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "GetName", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::IsEnabled(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::IsEnabledRequest *request,
                                                            ::workrave::breaks::IsEnabledResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.is_enabled();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "IsEnabled", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::IsTimerRunning(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::IsTimerRunningRequest *request,
                                                            ::workrave::breaks::IsTimerRunningResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.is_running();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "IsTimerRunning", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::IsTaking(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::IsTakingRequest *request,
                                                            ::workrave::breaks::IsTakingResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.is_taking();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "IsTaking", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::IsMaxPreludesReached(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::IsMaxPreludesReachedRequest *request,
                                                            ::workrave::breaks::IsMaxPreludesReachedResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.is_max_preludes_reached();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "IsMaxPreludesReached", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::IsActive(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::IsActiveRequest *request,
                                                            ::workrave::breaks::IsActiveResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.is_active();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "IsActive", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::GetTimerElapsed(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::GetTimerElapsedRequest *request,
                                                            ::workrave::breaks::GetTimerElapsedResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.get_elapsed_time();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "GetTimerElapsed", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::GetTimerIdle(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::GetTimerIdleRequest *request,
                                                            ::workrave::breaks::GetTimerIdleResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.get_elapsed_idle_time();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "GetTimerIdle", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::GetAutoReset(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::GetAutoResetRequest *request,
                                                            ::workrave::breaks::GetAutoResetResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.get_auto_reset();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "GetAutoReset", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::IsAutoResetEnabled(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::IsAutoResetEnabledRequest *request,
                                                            ::workrave::breaks::IsAutoResetEnabledResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.is_auto_reset_enabled();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "IsAutoResetEnabled", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::GetLimit(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::GetLimitRequest *request,
                                                            ::workrave::breaks::GetLimitResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.get_limit();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "GetLimit", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::IsLimitEnabled(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::IsLimitEnabledRequest *request,
                                                            ::workrave::breaks::IsLimitEnabledResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.is_limit_enabled();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "IsLimitEnabled", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::GetTimerOverdue(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::GetTimerOverdueRequest *request,
                                                            ::workrave::breaks::GetTimerOverdueResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.get_total_overdue_time();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "GetTimerOverdue", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::PostponeBreak(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::PostponeBreakRequest *request,
                                                            ::workrave::breaks::PostponeBreakResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      impl_.postpone_break();


      ::rpc::intercept_request({"workrave.BreakService", "PostponeBreak", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::SkipBreak(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::SkipBreakRequest *request,
                                                            ::workrave::breaks::SkipBreakResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      impl_.skip_break();


      ::rpc::intercept_request({"workrave.BreakService", "SkipBreak", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::GetTimerRemaining(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::GetTimerRemainingRequest *request,
                                                            ::workrave::breaks::GetTimerRemainingResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.get_timer_remaining();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "GetTimerRemaining", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status BreakService::GetBreakState(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::breaks::GetBreakStateRequest *request,
                                                            ::workrave::breaks::GetBreakStateResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));



      auto rpc_result = impl_.get_break_stage();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.BreakService", "GetBreakState", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}



::grpc::Status BreakService::BreakEvent(::grpc::ServerContext *context,
                                                            const ::workrave::breaks::BreakEventRequest *request,
                                                            ::grpc::ServerWriter<::workrave::breaks::BreakEventEvent> *writer)
{

  auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));

  ::rpc::EventQueue<::workrave::breaks::BreakEventEvent> queue;
  auto connection = impl_.signal_break_event().connect(
    [&queue](workrave::BreakEvent value)
    {
      ::workrave::breaks::BreakEventEvent event;

      event.set_value(static_cast<::workrave::breaks::BreakEvent>(value));

      queue.push(event);
    });

  ::workrave::breaks::BreakEventEvent event;
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


::grpc::Status BreakService::BreakStateChanged(::grpc::ServerContext *context,
                                                            const ::workrave::breaks::BreakStateChangedRequest *request,
                                                            ::grpc::ServerWriter<::workrave::breaks::BreakStateChangedEvent> *writer)
{

  auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));

  ::rpc::EventQueue<::workrave::breaks::BreakStateChangedEvent> queue;
  auto connection = impl_.signal_break_stage_changed().connect(
    [&queue](BreakStage value)
    {
      ::workrave::breaks::BreakStateChangedEvent event;

      event.set_value(static_cast<::workrave::breaks::BreakStage>(value));

      queue.push(event);
    });

  ::workrave::breaks::BreakStateChangedEvent event;
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
