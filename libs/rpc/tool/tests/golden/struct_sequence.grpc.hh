// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcStructSeq.grpc.pb.h"
#include "struct_sequence.hh"


#include "rpc/EventQueue.hh"


class StructSeqServiceServiceImpl final : public ::workrave::test::StructSeqService::Service
{
public:

  explicit StructSeqServiceServiceImpl(RpcStructSeqFixture &impl)
    : impl_(impl)
  {
  }



  ::grpc::Status SetTimerData(::grpc::ServerContext *context,
                                 const ::workrave::test::SetTimerDataRequest *request,
                                 ::workrave::test::SetTimerDataResponse *response) override;

  ::grpc::Status GetTimerData(::grpc::ServerContext *context,
                                 const ::workrave::test::GetTimerDataRequest *request,
                                 ::workrave::test::GetTimerDataResponse *response) override;

  ::grpc::Status GetMenu(::grpc::ServerContext *context,
                                 const ::workrave::test::GetMenuRequest *request,
                                 ::workrave::test::GetMenuResponse *response) override;

  ::grpc::Status SetTags(::grpc::ServerContext *context,
                                 const ::workrave::test::SetTagsRequest *request,
                                 ::workrave::test::SetTagsResponse *response) override;



  ::grpc::Status TimerUpdated(::grpc::ServerContext *context,
                                 const ::workrave::test::TimerUpdatedRequest *request,
                                 ::grpc::ServerWriter<::workrave::test::TimerUpdatedEvent> *writer) override;


private:

  RpcStructSeqFixture &impl_;

};