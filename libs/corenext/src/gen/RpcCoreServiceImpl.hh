// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcCore.grpc.pb.h"
#include "Core.hh"


#include "rpc/EventQueue.hh"



namespace workrave::core::rpc
{
class CoreServiceServiceImpl final : public ::workrave::rpc::CoreService::Service
{
public:

  explicit CoreServiceServiceImpl(Core &impl);



  ::grpc::Status ForceBreak(::grpc::ServerContext *context,
                                 const ::workrave::rpc::core::ForceBreakRequest *request,
                                 ::workrave::rpc::core::ForceBreakResponse *response) override;

  ::grpc::Status IsActive(::grpc::ServerContext *context,
                                 const ::workrave::rpc::core::IsActiveRequest *request,
                                 ::workrave::rpc::core::IsActiveResponse *response) override;

  ::grpc::Status IsTaking(::grpc::ServerContext *context,
                                 const ::workrave::rpc::core::IsTakingRequest *request,
                                 ::workrave::rpc::core::IsTakingResponse *response) override;

  ::grpc::Status GetActiveOperationMode(::grpc::ServerContext *context,
                                 const ::workrave::rpc::core::GetActiveOperationModeRequest *request,
                                 ::workrave::rpc::core::GetActiveOperationModeResponse *response) override;

  ::grpc::Status GetOperationMode(::grpc::ServerContext *context,
                                 const ::workrave::rpc::core::GetOperationModeRequest *request,
                                 ::workrave::rpc::core::GetOperationModeResponse *response) override;

  ::grpc::Status IsOperationModeAnOverride(::grpc::ServerContext *context,
                                 const ::workrave::rpc::core::IsOperationModeAnOverrideRequest *request,
                                 ::workrave::rpc::core::IsOperationModeAnOverrideResponse *response) override;

  ::grpc::Status SetOperationMode(::grpc::ServerContext *context,
                                 const ::workrave::rpc::core::SetOperationModeRequest *request,
                                 ::workrave::rpc::core::SetOperationModeResponse *response) override;

  ::grpc::Status SetOperationModeFor(::grpc::ServerContext *context,
                                 const ::workrave::rpc::core::SetOperationModeForRequest *request,
                                 ::workrave::rpc::core::SetOperationModeForResponse *response) override;

  ::grpc::Status GetUsageMode(::grpc::ServerContext *context,
                                 const ::workrave::rpc::core::GetUsageModeRequest *request,
                                 ::workrave::rpc::core::GetUsageModeResponse *response) override;

  ::grpc::Status SetUsageMode(::grpc::ServerContext *context,
                                 const ::workrave::rpc::core::SetUsageModeRequest *request,
                                 ::workrave::rpc::core::SetUsageModeResponse *response) override;

  ::grpc::Status ReportActivity(::grpc::ServerContext *context,
                                 const ::workrave::rpc::core::ReportActivityRequest *request,
                                 ::workrave::rpc::core::ReportActivityResponse *response) override;



  ::grpc::Status OperationModeChanged(::grpc::ServerContext *context,
                                 const ::workrave::rpc::core::OperationModeChangedRequest *request,
                                 ::grpc::ServerWriter<::workrave::rpc::core::OperationModeChangedEvent> *writer) override;

  ::grpc::Status UsageModeChanged(::grpc::ServerContext *context,
                                 const ::workrave::rpc::core::UsageModeChangedRequest *request,
                                 ::grpc::ServerWriter<::workrave::rpc::core::UsageModeChangedEvent> *writer) override;


private:

  Core &impl_;


  [[maybe_unused]] const void *const service_descriptor_anchor_;
};

} // namespace workrave::core::rpc
