// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcDbusStructSeqServiceImpl.hh"

#include <exception>



::grpc::Status DBusFixture2ServiceServiceImpl::SetPoint(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::SetPointRequest *request,
                                                            ::workrave::test::SetPointResponse *response)
{
  try
    {


      Point local_p{};

      local_p.x = request->p().x();

      local_p.y = request->p().y();


      impl_.set_point(local_p);


    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status DBusFixture2ServiceServiceImpl::GetPoint(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::GetPointRequest *request,
                                                            ::workrave::test::GetPointResponse *response)
{
  try
    {



      auto rpc_result = impl_.get_point();

      auto *rpc_msg_0 = response->mutable_result();

      rpc_msg_0->set_x(rpc_result.x); rpc_msg_0->set_y(rpc_result.y);



    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status DBusFixture2ServiceServiceImpl::SetTags(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::SetTagsRequest *request,
                                                            ::workrave::test::SetTagsResponse *response)
{
  try
    {


      std::vector<int> local_tags{};

      for (const auto &rpc_wire_0 : request->tags()) { int32_t rpc_item_0{}; rpc_item_0 = rpc_wire_0; local_tags.push_back(rpc_item_0); }


      impl_.set_tags(local_tags);


    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status DBusFixture2ServiceServiceImpl::GetPoints(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::GetPointsRequest *request,
                                                            ::workrave::test::GetPointsResponse *response)
{
  try
    {



      auto rpc_result = impl_.get_points();

      for (const auto &rpc_item_0 : rpc_result) { auto *rpc_elem_0 = response->add_result(); rpc_elem_0->set_x(rpc_item_0.x); rpc_elem_0->set_y(rpc_item_0.y); }



    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


