// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcTestServiceImpl.hh"

#include <exception>

#include "rpc/RequestInterceptor.hh"



::grpc::Status TestServiceServiceImpl::Ping(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::PingRequest *request,
                                                            ::workrave::test::PingResponse *response)
{
  try
    {



      auto rpc_result = impl_.ping(request->message());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.test.TestService", "Ping", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status TestServiceServiceImpl::Add(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::AddRequest *request,
                                                            ::workrave::test::AddResponse *response)
{
  try
    {



      auto rpc_result = impl_.add(request->a(), request->b());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.test.TestService", "Add", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status TestServiceServiceImpl::SetFlag(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::SetFlagRequest *request,
                                                            ::workrave::test::SetFlagResponse *response)
{
  try
    {



      impl_.set_flag(request->value());


      ::rpc::intercept_request({"workrave.test.TestService", "SetFlag", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status TestServiceServiceImpl::GetMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::GetModeRequest *request,
                                                            ::workrave::test::GetModeResponse *response)
{
  try
    {


      TestMode local_mode{};


      auto rpc_result = impl_.get_mode(local_mode);

      response->set_result(rpc_result);



      response->set_mode(static_cast<::workrave::test::TestMode>(local_mode));

      ::rpc::intercept_request({"workrave.test.TestService", "GetMode", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status TestServiceServiceImpl::Greet(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::GreetRequest *request,
                                                            ::workrave::test::GreetResponse *response)
{
  try
    {



      auto rpc_result = impl_.greet(request->name());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.test.TestService", "Greet", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}



::grpc::Status TestServiceServiceImpl::ModeChanged(::grpc::ServerContext *context,
                                                            const ::workrave::test::ModeChangedRequest */*request*/,
                                                            ::grpc::ServerWriter<::workrave::test::ModeChangedEvent> *writer)
{

  ::rpc::EventQueue<::workrave::test::ModeChangedEvent> queue;
  auto connection = impl_.signal_mode_changed().connect(
    [&queue](TestMode value)
    {
      ::workrave::test::ModeChangedEvent event;

      event.set_value(static_cast<::workrave::test::TestMode>(value));

      queue.push(event);
    });

  ::workrave::test::ModeChangedEvent event;
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

