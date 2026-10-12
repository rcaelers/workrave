// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcWindowPreviewServiceImpl.hh"

#include <exception>

#include "rpc/RequestInterceptor.hh"



namespace workrave::ui::rpc
{


WindowPreviewService::WindowPreviewService(WindowPreview &impl)
  : impl_(impl)
  , service_descriptor_anchor_(&::descriptor_table_RpcWindowPreview_2eproto)
{
}



::grpc::Status WindowPreviewService::ShowBreak(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::ShowBreakRequest *request,
                                                            ::workrave::preview::ShowBreakResponse *response)
{
  try
    {



      auto rpc_result = impl_.show_break(static_cast<PreviewBreakType>(request->id()), request->progress(), request->duration(), request->skippable(), request->postponable());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "ShowBreak", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::ShowPrelude(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::ShowPreludeRequest *request,
                                                            ::workrave::preview::ShowPreludeResponse *response)
{
  try
    {



      auto rpc_result = impl_.show_prelude(static_cast<PreviewBreakType>(request->id()), static_cast<workrave::IApp::PreludeStage>(request->stage()), static_cast<workrave::IApp::PreludeProgressText>(request->text()), request->progress(), request->duration());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "ShowPrelude", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::ShowWindow(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::ShowWindowRequest *request,
                                                            ::workrave::preview::ShowWindowResponse *response)
{
  try
    {



      auto rpc_result = impl_.show_window(static_cast<PreviewWindowType>(request->window()), request->page(), request->panel());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "ShowWindow", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::ShowMenu(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::ShowMenuRequest *request,
                                                            ::workrave::preview::ShowMenuResponse *response)
{
  try
    {



      auto rpc_result = impl_.show_menu(request->tray(), request->submenu(), request->x(), request->y());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "ShowMenu", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::SetBackdrop(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::SetBackdropRequest *request,
                                                            ::workrave::preview::SetBackdropResponse *response)
{
  try
    {



      auto rpc_result = impl_.set_backdrop(request->color());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "SetBackdrop", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::SetTimer(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::SetTimerRequest *request,
                                                            ::workrave::preview::SetTimerResponse *response)
{
  try
    {



      auto rpc_result = impl_.set_timer(static_cast<PreviewBreakType>(request->id()), request->elapsed(), request->limit(), request->idle(), request->duration(), request->enabled());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "SetTimer", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::SetAppearance(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::SetAppearanceRequest *request,
                                                            ::workrave::preview::SetAppearanceResponse *response)
{
  try
    {



      auto rpc_result = impl_.set_appearance(static_cast<PreviewBlockMode>(request->block_mode()), request->user_active(), request->exercise_index(), request->exercise_elapsed());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "SetAppearance", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::SetButtonLock(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::SetButtonLockRequest *request,
                                                            ::workrave::preview::SetButtonLockResponse *response)
{
  try
    {



      auto rpc_result = impl_.set_button_lock(request->can_skip(), request->can_postpone(), request->value(), request->maximum());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "SetButtonLock", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::SetModes(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::SetModesRequest *request,
                                                            ::workrave::preview::SetModesResponse *response)
{
  try
    {



      auto rpc_result = impl_.set_modes(static_cast<workrave::OperationMode>(request->operation_mode()), static_cast<workrave::UsageMode>(request->usage_mode()));

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "SetModes", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::SetBoolSetting(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::SetBoolSettingRequest *request,
                                                            ::workrave::preview::SetBoolSettingResponse *response)
{
  try
    {



      auto rpc_result = impl_.set_bool_setting(request->key(), request->value());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "SetBoolSetting", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::SetIntSetting(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::SetIntSettingRequest *request,
                                                            ::workrave::preview::SetIntSettingResponse *response)
{
  try
    {



      auto rpc_result = impl_.set_int_setting(request->key(), request->value());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "SetIntSetting", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::SetStringSetting(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::SetStringSettingRequest *request,
                                                            ::workrave::preview::SetStringSettingResponse *response)
{
  try
    {



      auto rpc_result = impl_.set_string_setting(request->key(), request->value());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "SetStringSetting", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::GetLastError(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::GetLastErrorRequest *request,
                                                            ::workrave::preview::GetLastErrorResponse *response)
{
  try
    {



      auto rpc_result = impl_.get_last_error();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "GetLastError", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::Hide(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::HideRequest *request,
                                                            ::workrave::preview::HideResponse *response)
{
  try
    {



      auto rpc_result = impl_.hide();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "Hide", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::Quit(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::QuitRequest *request,
                                                            ::workrave::preview::QuitResponse *response)
{
  try
    {



      auto rpc_result = impl_.quit();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "Quit", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::GetAppliedSequence(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::GetAppliedSequenceRequest *request,
                                                            ::workrave::preview::GetAppliedSequenceResponse *response)
{
  try
    {



      auto rpc_result = impl_.get_applied_sequence();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "GetAppliedSequence", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WindowPreviewService::WaitForAppliedSequence(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::preview::WaitForAppliedSequenceRequest *request,
                                                            ::workrave::preview::WaitForAppliedSequenceResponse *response)
{
  try
    {



      auto rpc_result = impl_.wait_for_applied_sequence(request->sequence(), request->after_sequence());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WindowPreviewService", "WaitForAppliedSequence", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}




} // namespace workrave::ui::rpc
