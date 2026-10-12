// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcWindowPreview.grpc.pb.h"
#include "WindowPreview.hh"




namespace workrave::ui::rpc
{
class WindowPreviewService final : public ::workrave::rpc::WindowPreviewService::Service
{
public:

  explicit WindowPreviewService(WindowPreview &impl);



  ::grpc::Status ShowBreak(::grpc::ServerContext *context,
                                 const ::workrave::preview::ShowBreakRequest *request,
                                 ::workrave::preview::ShowBreakResponse *response) override;

  ::grpc::Status ShowPrelude(::grpc::ServerContext *context,
                                 const ::workrave::preview::ShowPreludeRequest *request,
                                 ::workrave::preview::ShowPreludeResponse *response) override;

  ::grpc::Status ShowWindow(::grpc::ServerContext *context,
                                 const ::workrave::preview::ShowWindowRequest *request,
                                 ::workrave::preview::ShowWindowResponse *response) override;

  ::grpc::Status ShowMenu(::grpc::ServerContext *context,
                                 const ::workrave::preview::ShowMenuRequest *request,
                                 ::workrave::preview::ShowMenuResponse *response) override;

  ::grpc::Status SetBackdrop(::grpc::ServerContext *context,
                                 const ::workrave::preview::SetBackdropRequest *request,
                                 ::workrave::preview::SetBackdropResponse *response) override;

  ::grpc::Status SetTimer(::grpc::ServerContext *context,
                                 const ::workrave::preview::SetTimerRequest *request,
                                 ::workrave::preview::SetTimerResponse *response) override;

  ::grpc::Status SetAppearance(::grpc::ServerContext *context,
                                 const ::workrave::preview::SetAppearanceRequest *request,
                                 ::workrave::preview::SetAppearanceResponse *response) override;

  ::grpc::Status SetButtonLock(::grpc::ServerContext *context,
                                 const ::workrave::preview::SetButtonLockRequest *request,
                                 ::workrave::preview::SetButtonLockResponse *response) override;

  ::grpc::Status SetModes(::grpc::ServerContext *context,
                                 const ::workrave::preview::SetModesRequest *request,
                                 ::workrave::preview::SetModesResponse *response) override;

  ::grpc::Status SetBoolSetting(::grpc::ServerContext *context,
                                 const ::workrave::preview::SetBoolSettingRequest *request,
                                 ::workrave::preview::SetBoolSettingResponse *response) override;

  ::grpc::Status SetIntSetting(::grpc::ServerContext *context,
                                 const ::workrave::preview::SetIntSettingRequest *request,
                                 ::workrave::preview::SetIntSettingResponse *response) override;

  ::grpc::Status SetStringSetting(::grpc::ServerContext *context,
                                 const ::workrave::preview::SetStringSettingRequest *request,
                                 ::workrave::preview::SetStringSettingResponse *response) override;

  ::grpc::Status GetLastError(::grpc::ServerContext *context,
                                 const ::workrave::preview::GetLastErrorRequest *request,
                                 ::workrave::preview::GetLastErrorResponse *response) override;

  ::grpc::Status Hide(::grpc::ServerContext *context,
                                 const ::workrave::preview::HideRequest *request,
                                 ::workrave::preview::HideResponse *response) override;

  ::grpc::Status Quit(::grpc::ServerContext *context,
                                 const ::workrave::preview::QuitRequest *request,
                                 ::workrave::preview::QuitResponse *response) override;

  ::grpc::Status GetAppliedSequence(::grpc::ServerContext *context,
                                 const ::workrave::preview::GetAppliedSequenceRequest *request,
                                 ::workrave::preview::GetAppliedSequenceResponse *response) override;

  ::grpc::Status WaitForAppliedSequence(::grpc::ServerContext *context,
                                 const ::workrave::preview::WaitForAppliedSequenceRequest *request,
                                 ::workrave::preview::WaitForAppliedSequenceResponse *response) override;




private:

  WindowPreview &impl_;


  [[maybe_unused]] const void *const service_descriptor_anchor_;
};

} // namespace workrave::ui::rpc
