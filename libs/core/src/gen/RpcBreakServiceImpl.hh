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
class BreakServiceServiceImpl final : public ::workrave::rpc::BreakService::Service
{
public:

  // Break has multiple live instances; the caller supplies a
  // registry that instances register themselves into (see rpc::InstanceRegistry),
  // and each RPC resolves its target from the request's `id` field — the gRPC
  // analog of DBus's per-object-path routing.
  explicit BreakServiceServiceImpl(::rpc::InstanceRegistry<workrave::BreakId, Break> &registry);



  ::grpc::Status GetName(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::GetNameRequest *request,
                                 ::workrave::rpc::breaks::GetNameResponse *response) override;

  ::grpc::Status IsEnabled(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::IsEnabledRequest *request,
                                 ::workrave::rpc::breaks::IsEnabledResponse *response) override;

  ::grpc::Status IsTimerRunning(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::IsTimerRunningRequest *request,
                                 ::workrave::rpc::breaks::IsTimerRunningResponse *response) override;

  ::grpc::Status GetTimerElapsed(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::GetTimerElapsedRequest *request,
                                 ::workrave::rpc::breaks::GetTimerElapsedResponse *response) override;

  ::grpc::Status GetTimerIdle(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::GetTimerIdleRequest *request,
                                 ::workrave::rpc::breaks::GetTimerIdleResponse *response) override;

  ::grpc::Status GetAutoReset(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::GetAutoResetRequest *request,
                                 ::workrave::rpc::breaks::GetAutoResetResponse *response) override;

  ::grpc::Status IsAutoResetEnabled(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::IsAutoResetEnabledRequest *request,
                                 ::workrave::rpc::breaks::IsAutoResetEnabledResponse *response) override;

  ::grpc::Status GetLimit(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::GetLimitRequest *request,
                                 ::workrave::rpc::breaks::GetLimitResponse *response) override;

  ::grpc::Status IsLimitEnabled(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::IsLimitEnabledRequest *request,
                                 ::workrave::rpc::breaks::IsLimitEnabledResponse *response) override;

  ::grpc::Status IsTaking(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::IsTakingRequest *request,
                                 ::workrave::rpc::breaks::IsTakingResponse *response) override;

  ::grpc::Status IsMaxPreludesReached(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::IsMaxPreludesReachedRequest *request,
                                 ::workrave::rpc::breaks::IsMaxPreludesReachedResponse *response) override;

  ::grpc::Status PostponeBreak(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::PostponeBreakRequest *request,
                                 ::workrave::rpc::breaks::PostponeBreakResponse *response) override;

  ::grpc::Status SkipBreak(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::SkipBreakRequest *request,
                                 ::workrave::rpc::breaks::SkipBreakResponse *response) override;

  ::grpc::Status IsActive(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::IsActiveRequest *request,
                                 ::workrave::rpc::breaks::IsActiveResponse *response) override;

  ::grpc::Status GetTimerRemaining(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::GetTimerRemainingRequest *request,
                                 ::workrave::rpc::breaks::GetTimerRemainingResponse *response) override;

  ::grpc::Status GetTimerOverdue(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::GetTimerOverdueRequest *request,
                                 ::workrave::rpc::breaks::GetTimerOverdueResponse *response) override;

  ::grpc::Status GetBreakState(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::GetBreakStateRequest *request,
                                 ::workrave::rpc::breaks::GetBreakStateResponse *response) override;



  ::grpc::Status BreakEvent(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::BreakEventRequest *request,
                                 ::grpc::ServerWriter<::workrave::rpc::breaks::BreakEventEvent> *writer) override;

  ::grpc::Status BreakStateChanged(::grpc::ServerContext *context,
                                 const ::workrave::rpc::breaks::BreakStateChangedRequest *request,
                                 ::grpc::ServerWriter<::workrave::rpc::breaks::BreakStateChangedEvent> *writer) override;


private:

  ::rpc::InstanceRegistry<workrave::BreakId, Break> &registry_;


  [[maybe_unused]] const void *const service_descriptor_anchor_;
};

} // namespace workrave::core::rpc
