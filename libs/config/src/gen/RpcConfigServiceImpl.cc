// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcConfigServiceImpl.hh"

#include <exception>

#include "rpc/RequestInterceptor.hh"



namespace workrave::config::rpc
{


ConfigServiceServiceImpl::ConfigServiceServiceImpl(workrave::config::IConfigurator &impl)
  : impl_(impl)
  , service_descriptor_anchor_(&::descriptor_table_RpcConfig_2eproto)
{
}



::grpc::Status ConfigServiceServiceImpl::RemoveKey(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::RemoveKeyRequest *request,
                                                            ::workrave::rpc::config::RemoveKeyResponse *response)
{
  try
    {



      impl_.remove_key(request->key());


      ::rpc::intercept_request({"workrave.ConfigService", "RemoveKey", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::RenameKey(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::RenameKeyRequest *request,
                                                            ::workrave::rpc::config::RenameKeyResponse *response)
{
  try
    {



      impl_.rename_key(request->key(), request->new_key());


      ::rpc::intercept_request({"workrave.ConfigService", "RenameKey", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::HasUserValue(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::HasUserValueRequest *request,
                                                            ::workrave::rpc::config::HasUserValueResponse *response)
{
  try
    {



      auto rpc_result = impl_.has_user_value(request->key());

      response->set_result(rpc_result);



      ::rpc::intercept_request({"workrave.ConfigService", "HasUserValue", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::GetString(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::GetStringRequest *request,
                                                            ::workrave::rpc::config::GetStringResponse *response)
{
  try
    {


      std::string local_out{};


      auto rpc_result = impl_.get_value(request->key(), local_out);

      response->set_result(rpc_result);



      response->set_out(local_out);

      ::rpc::intercept_request({"workrave.ConfigService", "GetString", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::GetBool(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::GetBoolRequest *request,
                                                            ::workrave::rpc::config::GetBoolResponse *response)
{
  try
    {


      bool local_out{};


      auto rpc_result = impl_.get_value(request->key(), local_out);

      response->set_result(rpc_result);



      response->set_out(local_out);

      ::rpc::intercept_request({"workrave.ConfigService", "GetBool", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::GetInt(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::GetIntRequest *request,
                                                            ::workrave::rpc::config::GetIntResponse *response)
{
  try
    {


      int32_t local_out{};


      auto rpc_result = impl_.get_value(request->key(), local_out);

      response->set_result(rpc_result);



      response->set_out(local_out);

      ::rpc::intercept_request({"workrave.ConfigService", "GetInt", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::GetInt64(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::GetInt64Request *request,
                                                            ::workrave::rpc::config::GetInt64Response *response)
{
  try
    {


      int64_t local_out{};


      auto rpc_result = impl_.get_value(request->key(), local_out);

      response->set_result(rpc_result);



      response->set_out(local_out);

      ::rpc::intercept_request({"workrave.ConfigService", "GetInt64", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::GetDouble(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::GetDoubleRequest *request,
                                                            ::workrave::rpc::config::GetDoubleResponse *response)
{
  try
    {


      double local_out{};


      auto rpc_result = impl_.get_value(request->key(), local_out);

      response->set_result(rpc_result);



      response->set_out(local_out);

      ::rpc::intercept_request({"workrave.ConfigService", "GetDouble", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::GetStringWithDefault(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::GetStringWithDefaultRequest *request,
                                                            ::workrave::rpc::config::GetStringWithDefaultResponse *response)
{
  try
    {


      std::string local_out{};


      impl_.get_value_with_default(request->key(), local_out, request->s());


      response->set_out(local_out);

      ::rpc::intercept_request({"workrave.ConfigService", "GetStringWithDefault", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::GetBoolWithDefault(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::GetBoolWithDefaultRequest *request,
                                                            ::workrave::rpc::config::GetBoolWithDefaultResponse *response)
{
  try
    {


      bool local_out{};


      impl_.get_value_with_default(request->key(), local_out, request->def());


      response->set_out(local_out);

      ::rpc::intercept_request({"workrave.ConfigService", "GetBoolWithDefault", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::GetIntWithDefault(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::GetIntWithDefaultRequest *request,
                                                            ::workrave::rpc::config::GetIntWithDefaultResponse *response)
{
  try
    {


      int32_t local_out{};


      impl_.get_value_with_default(request->key(), local_out, request->def());


      response->set_out(local_out);

      ::rpc::intercept_request({"workrave.ConfigService", "GetIntWithDefault", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::GetInt64WithDefault(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::GetInt64WithDefaultRequest *request,
                                                            ::workrave::rpc::config::GetInt64WithDefaultResponse *response)
{
  try
    {


      int64_t local_out{};


      impl_.get_value_with_default(request->key(), local_out, request->def());


      response->set_out(local_out);

      ::rpc::intercept_request({"workrave.ConfigService", "GetInt64WithDefault", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::GetDoubleWithDefault(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::GetDoubleWithDefaultRequest *request,
                                                            ::workrave::rpc::config::GetDoubleWithDefaultResponse *response)
{
  try
    {


      double local_out{};


      impl_.get_value_with_default(request->key(), local_out, request->def());


      response->set_out(local_out);

      ::rpc::intercept_request({"workrave.ConfigService", "GetDoubleWithDefault", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::SetString(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::SetStringRequest *request,
                                                            ::workrave::rpc::config::SetStringResponse *response)
{
  try
    {



      impl_.set_value(request->key(), request->v(), static_cast<workrave::config::ConfigFlags>(request->flags()));


      ::rpc::intercept_request({"workrave.ConfigService", "SetString", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::SetInt(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::SetIntRequest *request,
                                                            ::workrave::rpc::config::SetIntResponse *response)
{
  try
    {



      impl_.set_value(request->key(), request->v(), static_cast<workrave::config::ConfigFlags>(request->flags()));


      ::rpc::intercept_request({"workrave.ConfigService", "SetInt", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::SetInt64(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::SetInt64Request *request,
                                                            ::workrave::rpc::config::SetInt64Response *response)
{
  try
    {



      impl_.set_value(request->key(), request->v(), static_cast<workrave::config::ConfigFlags>(request->flags()));


      ::rpc::intercept_request({"workrave.ConfigService", "SetInt64", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::SetBool(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::SetBoolRequest *request,
                                                            ::workrave::rpc::config::SetBoolResponse *response)
{
  try
    {



      impl_.set_value(request->key(), request->v(), static_cast<workrave::config::ConfigFlags>(request->flags()));


      ::rpc::intercept_request({"workrave.ConfigService", "SetBool", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}


::grpc::Status ConfigServiceServiceImpl::SetDouble(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::rpc::config::SetDoubleRequest *request,
                                                            ::workrave::rpc::config::SetDoubleResponse *response)
{
  try
    {



      impl_.set_value(request->key(), request->v(), static_cast<workrave::config::ConfigFlags>(request->flags()));


      ::rpc::intercept_request({"workrave.ConfigService", "SetDouble", *request});
    }
  catch (const std::exception &e)
    {
      return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, e.what());
    }
  return ::grpc::Status::OK;
}




} // namespace workrave::config::rpc
