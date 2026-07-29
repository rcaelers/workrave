// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcDbusStructSeq.grpc.pb.h"
#include "dbus_struct_sequence.hh"



class DBusFixture2Service final : public ::workrave::test::DBusFixture2Service::Service
{
public:

  explicit DBusFixture2Service(RpcDBusFixture2 &impl)
    : impl_(impl)
  {
  }



  ::grpc::Status SetPoint(::grpc::ServerContext *context,
                                 const ::workrave::test::SetPointRequest *request,
                                 ::workrave::test::SetPointResponse *response) override;

  ::grpc::Status GetPoint(::grpc::ServerContext *context,
                                 const ::workrave::test::GetPointRequest *request,
                                 ::workrave::test::GetPointResponse *response) override;

  ::grpc::Status SetTags(::grpc::ServerContext *context,
                                 const ::workrave::test::SetTagsRequest *request,
                                 ::workrave::test::SetTagsResponse *response) override;

  ::grpc::Status GetPoints(::grpc::ServerContext *context,
                                 const ::workrave::test::GetPointsRequest *request,
                                 ::workrave::test::GetPointsResponse *response) override;




private:

  RpcDBusFixture2 &impl_;

};