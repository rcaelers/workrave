// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcBreakServiceImpl.hh"

#include <exception>

#include "rpc/RequestInterceptor.hh"



namespace workrave::core::rpc
{


BreakServiceServiceImpl::BreakServiceServiceImpl(::rpc::InstanceRegistry<workrave::BreakId, Break> &registry)
  : registry_(registry)
  , service_descriptor_anchor_(&::descriptor_table_RpcBreak_2eproto)
{
}



::grpc::Status BreakServiceServiceImpl::GetName(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::GetNameRequest *request,
                                                            ::workrave::rpc::breaks::GetNameResponse *response)
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


::grpc::Status BreakServiceServiceImpl::IsEnabled(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::IsEnabledRequest *request,
                                                            ::workrave::rpc::breaks::IsEnabledResponse *response)
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


::grpc::Status BreakServiceServiceImpl::IsTimerRunning(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::IsTimerRunningRequest *request,
                                                            ::workrave::rpc::breaks::IsTimerRunningResponse *response)
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


::grpc::Status BreakServiceServiceImpl::GetTimerElapsed(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::GetTimerElapsedRequest *request,
                                                            ::workrave::rpc::breaks::GetTimerElapsedResponse *response)
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


::grpc::Status BreakServiceServiceImpl::GetTimerIdle(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::GetTimerIdleRequest *request,
                                                            ::workrave::rpc::breaks::GetTimerIdleResponse *response)
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


::grpc::Status BreakServiceServiceImpl::GetAutoReset(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::GetAutoResetRequest *request,
                                                            ::workrave::rpc::breaks::GetAutoResetResponse *response)
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


::grpc::Status BreakServiceServiceImpl::IsAutoResetEnabled(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::IsAutoResetEnabledRequest *request,
                                                            ::workrave::rpc::breaks::IsAutoResetEnabledResponse *response)
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


::grpc::Status BreakServiceServiceImpl::GetLimit(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::GetLimitRequest *request,
                                                            ::workrave::rpc::breaks::GetLimitResponse *response)
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


::grpc::Status BreakServiceServiceImpl::IsLimitEnabled(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::IsLimitEnabledRequest *request,
                                                            ::workrave::rpc::breaks::IsLimitEnabledResponse *response)
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


::grpc::Status BreakServiceServiceImpl::IsTaking(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::IsTakingRequest *request,
                                                            ::workrave::rpc::breaks::IsTakingResponse *response)
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


::grpc::Status BreakServiceServiceImpl::IsMaxPreludesReached(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::IsMaxPreludesReachedRequest *request,
                                                            ::workrave::rpc::breaks::IsMaxPreludesReachedResponse *response)
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


::grpc::Status BreakServiceServiceImpl::PostponeBreak(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::PostponeBreakRequest *request,
                                                            ::workrave::rpc::breaks::PostponeBreakResponse *response)
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


::grpc::Status BreakServiceServiceImpl::SkipBreak(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::SkipBreakRequest *request,
                                                            ::workrave::rpc::breaks::SkipBreakResponse *response)
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


::grpc::Status BreakServiceServiceImpl::IsActive(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::IsActiveRequest *request,
                                                            ::workrave::rpc::breaks::IsActiveResponse *response)
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


::grpc::Status BreakServiceServiceImpl::GetTimerRemaining(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::GetTimerRemainingRequest *request,
                                                            ::workrave::rpc::breaks::GetTimerRemainingResponse *response)
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


::grpc::Status BreakServiceServiceImpl::GetTimerOverdue(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::GetTimerOverdueRequest *request,
                                                            ::workrave::rpc::breaks::GetTimerOverdueResponse *response)
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


::grpc::Status BreakServiceServiceImpl::GetBreakState(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::breaks::GetBreakStateRequest *request,
                                                            ::workrave::rpc::breaks::GetBreakStateResponse *response)
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



::grpc::Status BreakServiceServiceImpl::BreakEvent(::grpc::ServerContext *context,
                                                            const ::workrave::rpc::breaks::BreakEventRequest *request,
                                                            ::grpc::ServerWriter<::workrave::rpc::breaks::BreakEventEvent> *writer)
{

  auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));

  ::rpc::EventQueue<::workrave::rpc::breaks::BreakEventEvent> queue;
  auto connection = impl_.signal_break_event().connect(
    [&queue](workrave::BreakEvent value)
    {
      ::workrave::rpc::breaks::BreakEventEvent event;

      event.set_value(static_cast<::workrave::rpc::breaks::BreakEvent>(value));

      queue.push(event);
    });

  ::workrave::rpc::breaks::BreakEventEvent event;
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


::grpc::Status BreakServiceServiceImpl::BreakStateChanged(::grpc::ServerContext *context,
                                                            const ::workrave::rpc::breaks::BreakStateChangedRequest *request,
                                                            ::grpc::ServerWriter<::workrave::rpc::breaks::BreakStateChangedEvent> *writer)
{

  auto &impl_ = registry_.resolve(static_cast<workrave::BreakId>(request->id()));

  ::rpc::EventQueue<::workrave::rpc::breaks::BreakStateChangedEvent> queue;
  auto connection = impl_.signal_break_stage_changed().connect(
    [&queue](BreakStage value)
    {
      ::workrave::rpc::breaks::BreakStateChangedEvent event;

      event.set_value(static_cast<::workrave::rpc::breaks::BreakStage>(value));

      queue.push(event);
    });

  ::workrave::rpc::breaks::BreakStateChangedEvent event;
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
