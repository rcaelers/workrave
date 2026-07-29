// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#include "RpcConfigServiceImpl.hh"

#include <exception>

#include "rpc/RequestInterceptor.hh"



namespace workrave::config::rpc
{


ConfigService::ConfigService(workrave::config::IConfigurator &impl)
  : impl_(impl)
  , service_descriptor_anchor_(&::descriptor_table_RpcConfig_2eproto)
{
}



::grpc::Status ConfigService::RemoveKey(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::RemoveKeyRequest *request,
                                                            ::workrave::config::RemoveKeyResponse *response)
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


::grpc::Status ConfigService::RenameKey(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::RenameKeyRequest *request,
                                                            ::workrave::config::RenameKeyResponse *response)
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


::grpc::Status ConfigService::HasUserValue(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::HasUserValueRequest *request,
                                                            ::workrave::config::HasUserValueResponse *response)
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


::grpc::Status ConfigService::GetString(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::GetStringRequest *request,
                                                            ::workrave::config::GetStringResponse *response)
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


::grpc::Status ConfigService::GetBool(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::GetBoolRequest *request,
                                                            ::workrave::config::GetBoolResponse *response)
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


::grpc::Status ConfigService::GetInt(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::GetIntRequest *request,
                                                            ::workrave::config::GetIntResponse *response)
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


::grpc::Status ConfigService::GetInt64(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::GetInt64Request *request,
                                                            ::workrave::config::GetInt64Response *response)
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


::grpc::Status ConfigService::GetDouble(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::GetDoubleRequest *request,
                                                            ::workrave::config::GetDoubleResponse *response)
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


::grpc::Status ConfigService::GetStringWithDefault(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::GetStringWithDefaultRequest *request,
                                                            ::workrave::config::GetStringWithDefaultResponse *response)
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


::grpc::Status ConfigService::GetBoolWithDefault(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::GetBoolWithDefaultRequest *request,
                                                            ::workrave::config::GetBoolWithDefaultResponse *response)
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


::grpc::Status ConfigService::GetIntWithDefault(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::GetIntWithDefaultRequest *request,
                                                            ::workrave::config::GetIntWithDefaultResponse *response)
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


::grpc::Status ConfigService::GetInt64WithDefault(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::GetInt64WithDefaultRequest *request,
                                                            ::workrave::config::GetInt64WithDefaultResponse *response)
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


::grpc::Status ConfigService::GetDoubleWithDefault(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::GetDoubleWithDefaultRequest *request,
                                                            ::workrave::config::GetDoubleWithDefaultResponse *response)
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


::grpc::Status ConfigService::SetString(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::SetStringRequest *request,
                                                            ::workrave::config::SetStringResponse *response)
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


::grpc::Status ConfigService::SetInt(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::SetIntRequest *request,
                                                            ::workrave::config::SetIntResponse *response)
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


::grpc::Status ConfigService::SetInt64(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::SetInt64Request *request,
                                                            ::workrave::config::SetInt64Response *response)
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


::grpc::Status ConfigService::SetBool(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::SetBoolRequest *request,
                                                            ::workrave::config::SetBoolResponse *response)
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


::grpc::Status ConfigService::SetDouble(::grpc::ServerContext * /*context*/,
                                                            const ::workrave::config::SetDoubleRequest *request,
                                                            ::workrave::config::SetDoubleResponse *response)
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
