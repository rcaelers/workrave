// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcBreak.grpc.pb.h"
#include "Break.hh"

#include "rpc/InstanceRegistry.hh"


#include "rpc/EventQueue.hh"



namespace workrave::core::rpc
{
class BreakService final : public ::workrave::rpc::BreakService::Service
{
public:

  // Break has multiple live instances; the caller supplies a
  // registry that instances register themselves into (see rpc::InstanceRegistry),
  // and each RPC resolves its target from the request's `id` field — the gRPC
  // analog of DBus's per-object-path routing.
  explicit BreakService(::rpc::InstanceRegistry<workrave::BreakId, Break> &registry);



  ::grpc::Status GetName(::grpc::ServerContext *context,
                                 const ::workrave::breaks::GetNameRequest *request,
                                 ::workrave::breaks::GetNameResponse *response) override;

  ::grpc::Status IsEnabled(::grpc::ServerContext *context,
                                 const ::workrave::breaks::IsEnabledRequest *request,
                                 ::workrave::breaks::IsEnabledResponse *response) override;

  ::grpc::Status IsTimerRunning(::grpc::ServerContext *context,
                                 const ::workrave::breaks::IsTimerRunningRequest *request,
                                 ::workrave::breaks::IsTimerRunningResponse *response) override;

  ::grpc::Status IsTaking(::grpc::ServerContext *context,
                                 const ::workrave::breaks::IsTakingRequest *request,
                                 ::workrave::breaks::IsTakingResponse *response) override;

  ::grpc::Status IsMaxPreludesReached(::grpc::ServerContext *context,
                                 const ::workrave::breaks::IsMaxPreludesReachedRequest *request,
                                 ::workrave::breaks::IsMaxPreludesReachedResponse *response) override;

  ::grpc::Status IsActive(::grpc::ServerContext *context,
                                 const ::workrave::breaks::IsActiveRequest *request,
                                 ::workrave::breaks::IsActiveResponse *response) override;

  ::grpc::Status GetTimerElapsed(::grpc::ServerContext *context,
                                 const ::workrave::breaks::GetTimerElapsedRequest *request,
                                 ::workrave::breaks::GetTimerElapsedResponse *response) override;

  ::grpc::Status GetTimerIdle(::grpc::ServerContext *context,
                                 const ::workrave::breaks::GetTimerIdleRequest *request,
                                 ::workrave::breaks::GetTimerIdleResponse *response) override;

  ::grpc::Status GetAutoReset(::grpc::ServerContext *context,
                                 const ::workrave::breaks::GetAutoResetRequest *request,
                                 ::workrave::breaks::GetAutoResetResponse *response) override;

  ::grpc::Status IsAutoResetEnabled(::grpc::ServerContext *context,
                                 const ::workrave::breaks::IsAutoResetEnabledRequest *request,
                                 ::workrave::breaks::IsAutoResetEnabledResponse *response) override;

  ::grpc::Status GetLimit(::grpc::ServerContext *context,
                                 const ::workrave::breaks::GetLimitRequest *request,
                                 ::workrave::breaks::GetLimitResponse *response) override;

  ::grpc::Status IsLimitEnabled(::grpc::ServerContext *context,
                                 const ::workrave::breaks::IsLimitEnabledRequest *request,
                                 ::workrave::breaks::IsLimitEnabledResponse *response) override;

  ::grpc::Status GetTimerRemaining(::grpc::ServerContext *context,
                                 const ::workrave::breaks::GetTimerRemainingRequest *request,
                                 ::workrave::breaks::GetTimerRemainingResponse *response) override;

  ::grpc::Status GetTimerOverdue(::grpc::ServerContext *context,
                                 const ::workrave::breaks::GetTimerOverdueRequest *request,
                                 ::workrave::breaks::GetTimerOverdueResponse *response) override;

  ::grpc::Status PostponeBreak(::grpc::ServerContext *context,
                                 const ::workrave::breaks::PostponeBreakRequest *request,
                                 ::workrave::breaks::PostponeBreakResponse *response) override;

  ::grpc::Status SkipBreak(::grpc::ServerContext *context,
                                 const ::workrave::breaks::SkipBreakRequest *request,
                                 ::workrave::breaks::SkipBreakResponse *response) override;

  ::grpc::Status GetBreakState(::grpc::ServerContext *context,
                                 const ::workrave::breaks::GetBreakStateRequest *request,
                                 ::workrave::breaks::GetBreakStateResponse *response) override;



  ::grpc::Status BreakEvent(::grpc::ServerContext *context,
                                 const ::workrave::breaks::BreakEventRequest *request,
                                 ::grpc::ServerWriter<::workrave::breaks::BreakEventEvent> *writer) override;

  ::grpc::Status BreakStateChanged(::grpc::ServerContext *context,
                                 const ::workrave::breaks::BreakStateChangedRequest *request,
                                 ::grpc::ServerWriter<::workrave::breaks::BreakStateChangedEvent> *writer) override;


private:

  ::rpc::InstanceRegistry<workrave::BreakId, Break> &registry_;


  [[maybe_unused]] const void *const service_descriptor_anchor_;
};

} // namespace workrave::core::rpc
