// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcTestServiceImpl.hh"

#include <exception>

#include "rpc/RequestInterceptor.hh"



::grpc::Status TestService::Ping(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::PingRequest *request,
                                                            ::workrave::PingResponse *response)
{
  try
    {



      auto rpc_result = impl_.ping(request->message());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.TestService", "Ping", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status TestService::Add(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::AddRequest *request,
                                                            ::workrave::AddResponse *response)
{
  try
    {



      auto rpc_result = impl_.add(request->a(), request->b());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.TestService", "Add", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status TestService::SetFlag(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::SetFlagRequest *request,
                                                            ::workrave::SetFlagResponse *response)
{
  try
    {



      impl_.set_flag(request->value());


      ::rpc::intercept_request({"workrave.TestService", "SetFlag", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status TestService::GetMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::GetModeRequest *request,
                                                            ::workrave::GetModeResponse *response)
{
  try
    {


      TestMode local_mode{};


      auto rpc_result = impl_.get_mode(local_mode);

      response->set_result(rpc_result);



      response->set_mode(static_cast<::workrave::TestMode>(local_mode));

      ::rpc::intercept_request({"workrave.TestService", "GetMode", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status TestService::Greet(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::GreetRequest *request,
                                                            ::workrave::GreetResponse *response)
{
  try
    {



      auto rpc_result = impl_.greet(request->name());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.TestService", "Greet", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}



::grpc::Status TestService::ModeChanged(::grpc::ServerContext *context,
                                                            const ::workrave::ModeChangedRequest */*request*/,
                                                            ::grpc::ServerWriter<::workrave::ModeChangedEvent> *writer)
{

  ::rpc::EventQueue<::workrave::ModeChangedEvent> queue;
  auto connection = impl_.signal_mode_changed().connect(
    [&queue](TestMode value)
    {
      ::workrave::ModeChangedEvent event;

      event.set_value(static_cast<::workrave::TestMode>(value));

      queue.push(event);
    });

  ::workrave::ModeChangedEvent event;
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

