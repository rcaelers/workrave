// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcKeyedServiceImpl.hh"

#include <exception>

#include "rpc/RequestInterceptor.hh"



::grpc::Status WidgetService::GetValue(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::GetValueRequest *request,
                                                            ::workrave::GetValueResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<WidgetId>(request->id()));



      auto rpc_result = impl_.get_value();

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.WidgetService", "GetValue", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WidgetService::SetValue(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::SetValueRequest *request,
                                                            ::workrave::SetValueResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<WidgetId>(request->id()));



      impl_.set_value(request->v());


      ::rpc::intercept_request({"workrave.WidgetService", "SetValue", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}



::grpc::Status WidgetService::ValueChanged(::grpc::ServerContext *context,
                                                            const ::workrave::ValueChangedRequest *request,
                                                            ::grpc::ServerWriter<::workrave::ValueChangedEvent> *writer)
{

  auto &impl_ = registry_.resolve(static_cast<WidgetId>(request->id()));

  ::rpc::EventQueue<::workrave::ValueChangedEvent> queue;
  auto connection = impl_.signal_value_changed().connect(
    [&queue](int32_t value)
    {
      ::workrave::ValueChangedEvent event;

      event.set_value(value);

      queue.push(event);
    });

  ::workrave::ValueChangedEvent event;
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

