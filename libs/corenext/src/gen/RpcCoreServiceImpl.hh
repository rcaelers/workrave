// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcCore.grpc.pb.h"
#include "Core.hh"


#include "rpc/EventQueue.hh"



namespace workrave::core::rpc
{
class CoreService final : public ::workrave::rpc::CoreService::Service
{
public:

  explicit CoreService(Core &impl);



  ::grpc::Status ForceBreak(::grpc::ServerContext *context,
                                 const ::workrave::core::ForceBreakRequest *request,
                                 ::workrave::core::ForceBreakResponse *response) override;

  ::grpc::Status IsActive(::grpc::ServerContext *context,
                                 const ::workrave::core::IsActiveRequest *request,
                                 ::workrave::core::IsActiveResponse *response) override;

  ::grpc::Status IsTaking(::grpc::ServerContext *context,
                                 const ::workrave::core::IsTakingRequest *request,
                                 ::workrave::core::IsTakingResponse *response) override;

  ::grpc::Status GetActiveOperationMode(::grpc::ServerContext *context,
                                 const ::workrave::core::GetActiveOperationModeRequest *request,
                                 ::workrave::core::GetActiveOperationModeResponse *response) override;

  ::grpc::Status GetOperationMode(::grpc::ServerContext *context,
                                 const ::workrave::core::GetOperationModeRequest *request,
                                 ::workrave::core::GetOperationModeResponse *response) override;

  ::grpc::Status IsOperationModeAnOverride(::grpc::ServerContext *context,
                                 const ::workrave::core::IsOperationModeAnOverrideRequest *request,
                                 ::workrave::core::IsOperationModeAnOverrideResponse *response) override;

  ::grpc::Status SetOperationMode(::grpc::ServerContext *context,
                                 const ::workrave::core::SetOperationModeRequest *request,
                                 ::workrave::core::SetOperationModeResponse *response) override;

  ::grpc::Status SetOperationModeFor(::grpc::ServerContext *context,
                                 const ::workrave::core::SetOperationModeForRequest *request,
                                 ::workrave::core::SetOperationModeForResponse *response) override;

  ::grpc::Status GetUsageMode(::grpc::ServerContext *context,
                                 const ::workrave::core::GetUsageModeRequest *request,
                                 ::workrave::core::GetUsageModeResponse *response) override;

  ::grpc::Status SetUsageMode(::grpc::ServerContext *context,
                                 const ::workrave::core::SetUsageModeRequest *request,
                                 ::workrave::core::SetUsageModeResponse *response) override;

  ::grpc::Status ReportActivity(::grpc::ServerContext *context,
                                 const ::workrave::core::ReportActivityRequest *request,
                                 ::workrave::core::ReportActivityResponse *response) override;



  ::grpc::Status OperationModeChanged(::grpc::ServerContext *context,
                                 const ::workrave::core::OperationModeChangedRequest *request,
                                 ::grpc::ServerWriter<::workrave::core::OperationModeChangedEvent> *writer) override;

  ::grpc::Status UsageModeChanged(::grpc::ServerContext *context,
                                 const ::workrave::core::UsageModeChangedRequest *request,
                                 ::grpc::ServerWriter<::workrave::core::UsageModeChangedEvent> *writer) override;


private:

  Core &impl_;


  [[maybe_unused]] const void *const service_descriptor_anchor_;
};

} // namespace workrave::core::rpc
