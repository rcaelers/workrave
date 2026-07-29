// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen from an annotated C++ header. Re-run codegen
// instead of hand-editing this file; it will be overwritten on the next build.
#pragma once

#include "RpcConfig.grpc.pb.h"
#include "config/IConfigurator.hh"




namespace workrave::config::rpc
{
class ConfigService final : public ::workrave::rpc::ConfigService::Service
{
public:

  explicit ConfigService(workrave::config::IConfigurator &impl);



  ::grpc::Status RemoveKey(::grpc::ServerContext *context,
                                 const ::workrave::config::RemoveKeyRequest *request,
                                 ::workrave::config::RemoveKeyResponse *response) override;

  ::grpc::Status RenameKey(::grpc::ServerContext *context,
                                 const ::workrave::config::RenameKeyRequest *request,
                                 ::workrave::config::RenameKeyResponse *response) override;

  ::grpc::Status HasUserValue(::grpc::ServerContext *context,
                                 const ::workrave::config::HasUserValueRequest *request,
                                 ::workrave::config::HasUserValueResponse *response) override;

  ::grpc::Status GetString(::grpc::ServerContext *context,
                                 const ::workrave::config::GetStringRequest *request,
                                 ::workrave::config::GetStringResponse *response) override;

  ::grpc::Status GetBool(::grpc::ServerContext *context,
                                 const ::workrave::config::GetBoolRequest *request,
                                 ::workrave::config::GetBoolResponse *response) override;

  ::grpc::Status GetInt(::grpc::ServerContext *context,
                                 const ::workrave::config::GetIntRequest *request,
                                 ::workrave::config::GetIntResponse *response) override;

  ::grpc::Status GetInt64(::grpc::ServerContext *context,
                                 const ::workrave::config::GetInt64Request *request,
                                 ::workrave::config::GetInt64Response *response) override;

  ::grpc::Status GetDouble(::grpc::ServerContext *context,
                                 const ::workrave::config::GetDoubleRequest *request,
                                 ::workrave::config::GetDoubleResponse *response) override;

  ::grpc::Status GetStringWithDefault(::grpc::ServerContext *context,
                                 const ::workrave::config::GetStringWithDefaultRequest *request,
                                 ::workrave::config::GetStringWithDefaultResponse *response) override;

  ::grpc::Status GetBoolWithDefault(::grpc::ServerContext *context,
                                 const ::workrave::config::GetBoolWithDefaultRequest *request,
                                 ::workrave::config::GetBoolWithDefaultResponse *response) override;

  ::grpc::Status GetIntWithDefault(::grpc::ServerContext *context,
                                 const ::workrave::config::GetIntWithDefaultRequest *request,
                                 ::workrave::config::GetIntWithDefaultResponse *response) override;

  ::grpc::Status GetInt64WithDefault(::grpc::ServerContext *context,
                                 const ::workrave::config::GetInt64WithDefaultRequest *request,
                                 ::workrave::config::GetInt64WithDefaultResponse *response) override;

  ::grpc::Status GetDoubleWithDefault(::grpc::ServerContext *context,
                                 const ::workrave::config::GetDoubleWithDefaultRequest *request,
                                 ::workrave::config::GetDoubleWithDefaultResponse *response) override;

  ::grpc::Status SetString(::grpc::ServerContext *context,
                                 const ::workrave::config::SetStringRequest *request,
                                 ::workrave::config::SetStringResponse *response) override;

  ::grpc::Status SetInt(::grpc::ServerContext *context,
                                 const ::workrave::config::SetIntRequest *request,
                                 ::workrave::config::SetIntResponse *response) override;

  ::grpc::Status SetInt64(::grpc::ServerContext *context,
                                 const ::workrave::config::SetInt64Request *request,
                                 ::workrave::config::SetInt64Response *response) override;

  ::grpc::Status SetBool(::grpc::ServerContext *context,
                                 const ::workrave::config::SetBoolRequest *request,
                                 ::workrave::config::SetBoolResponse *response) override;

  ::grpc::Status SetDouble(::grpc::ServerContext *context,
                                 const ::workrave::config::SetDoubleRequest *request,
                                 ::workrave::config::SetDoubleResponse *response) override;




private:

  workrave::config::IConfigurator &impl_;


  [[maybe_unused]] const void *const service_descriptor_anchor_;
};

} // namespace workrave::config::rpc
