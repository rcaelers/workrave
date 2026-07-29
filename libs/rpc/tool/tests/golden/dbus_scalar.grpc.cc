// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcDbusScalarServiceImpl.hh"

#include <exception>

#include "rpc/RequestInterceptor.hh"



::grpc::Status DBusFixtureService::Ping(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::PingRequest *request,
                                                            ::workrave::test::PingResponse *response)
{
  try
    {



      auto rpc_result = impl_.ping(request->message());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.test.DBusFixtureService", "Ping", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status DBusFixtureService::GetMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::GetModeRequest *request,
                                                            ::workrave::test::GetModeResponse *response)
{
  try
    {



      auto rpc_result = impl_.get_mode();

      response->set_result(static_cast<::workrave::test::TestMode>(rpc_result));



      ::rpc::intercept_request({"workrave.test.DBusFixtureService", "GetMode", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status DBusFixtureService::SetMode(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::SetModeRequest *request,
                                                            ::workrave::test::SetModeResponse *response)
{
  try
    {



      impl_.set_mode(static_cast<TestMode>(request->mode()));


      ::rpc::intercept_request({"workrave.test.DBusFixtureService", "SetMode", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}



::grpc::Status DBusFixtureService::ModeChanged(::grpc::ServerContext *context,
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

