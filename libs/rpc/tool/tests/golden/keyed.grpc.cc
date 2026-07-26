// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcKeyedServiceImpl.hh"

#include <exception>



::grpc::Status WidgetServiceServiceImpl::GetValue(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::GetValueRequest *request,
                                                            ::workrave::test::GetValueResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<WidgetId>(request->id()));



      auto rpc_result = impl_.get_value();

      response->set_result(rpc_result);



    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status WidgetServiceServiceImpl::SetValue(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::test::SetValueRequest *request,
                                                            ::workrave::test::SetValueResponse *response)
{
  try
    {

      auto &impl_ = registry_.resolve(static_cast<WidgetId>(request->id()));



      impl_.set_value(request->v());


    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}



::grpc::Status WidgetServiceServiceImpl::ValueChanged(::grpc::ServerContext *context,
                                                            const ::workrave::test::ValueChangedRequest *request,
                                                            ::grpc::ServerWriter<::workrave::test::ValueChangedEvent> *writer)
{

  auto &impl_ = registry_.resolve(static_cast<WidgetId>(request->id()));

  ::rpc::EventQueue<::workrave::test::ValueChangedEvent> queue;
  auto connection = impl_.signal_value_changed().connect(
    [&queue](int32_t value)
    {
      ::workrave::test::ValueChangedEvent event;

      event.set_value(value);

      queue.push(event);
    });

  ::workrave::test::ValueChangedEvent event;
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

