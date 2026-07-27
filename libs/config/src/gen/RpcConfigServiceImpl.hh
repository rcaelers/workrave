// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcConfig.grpc.pb.h"
#include "config/IConfigurator.hh"




namespace workrave::config::rpc
{
class ConfigServiceServiceImpl final : public ::workrave::rpc::ConfigService::Service
{
public:

  explicit ConfigServiceServiceImpl(workrave::config::IConfigurator &impl);



  ::grpc::Status RemoveKey(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::RemoveKeyRequest *request,
                                 ::workrave::rpc::config::RemoveKeyResponse *response) override;

  ::grpc::Status RenameKey(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::RenameKeyRequest *request,
                                 ::workrave::rpc::config::RenameKeyResponse *response) override;

  ::grpc::Status HasUserValue(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::HasUserValueRequest *request,
                                 ::workrave::rpc::config::HasUserValueResponse *response) override;

  ::grpc::Status GetString(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::GetStringRequest *request,
                                 ::workrave::rpc::config::GetStringResponse *response) override;

  ::grpc::Status GetBool(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::GetBoolRequest *request,
                                 ::workrave::rpc::config::GetBoolResponse *response) override;

  ::grpc::Status GetInt(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::GetIntRequest *request,
                                 ::workrave::rpc::config::GetIntResponse *response) override;

  ::grpc::Status GetInt64(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::GetInt64Request *request,
                                 ::workrave::rpc::config::GetInt64Response *response) override;

  ::grpc::Status GetDouble(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::GetDoubleRequest *request,
                                 ::workrave::rpc::config::GetDoubleResponse *response) override;

  ::grpc::Status GetStringWithDefault(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::GetStringWithDefaultRequest *request,
                                 ::workrave::rpc::config::GetStringWithDefaultResponse *response) override;

  ::grpc::Status GetBoolWithDefault(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::GetBoolWithDefaultRequest *request,
                                 ::workrave::rpc::config::GetBoolWithDefaultResponse *response) override;

  ::grpc::Status GetIntWithDefault(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::GetIntWithDefaultRequest *request,
                                 ::workrave::rpc::config::GetIntWithDefaultResponse *response) override;

  ::grpc::Status GetInt64WithDefault(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::GetInt64WithDefaultRequest *request,
                                 ::workrave::rpc::config::GetInt64WithDefaultResponse *response) override;

  ::grpc::Status GetDoubleWithDefault(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::GetDoubleWithDefaultRequest *request,
                                 ::workrave::rpc::config::GetDoubleWithDefaultResponse *response) override;

  ::grpc::Status SetString(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::SetStringRequest *request,
                                 ::workrave::rpc::config::SetStringResponse *response) override;

  ::grpc::Status SetInt(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::SetIntRequest *request,
                                 ::workrave::rpc::config::SetIntResponse *response) override;

  ::grpc::Status SetInt64(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::SetInt64Request *request,
                                 ::workrave::rpc::config::SetInt64Response *response) override;

  ::grpc::Status SetBool(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::SetBoolRequest *request,
                                 ::workrave::rpc::config::SetBoolResponse *response) override;

  ::grpc::Status SetDouble(::grpc::ServerContext *context,
                                 const ::workrave::rpc::config::SetDoubleRequest *request,
                                 ::workrave::rpc::config::SetDoubleResponse *response) override;




private:

  workrave::config::IConfigurator &impl_;


  [[maybe_unused]] const void *const service_descriptor_anchor_;
};

} // namespace workrave::config::rpc
