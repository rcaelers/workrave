// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's GDBus backend from an annotated C++ header.
#pragma once

#include <string>
#include <string_view>


#include "rpc/dbus/GioInterface.hh"
#include "rpc/dbus/GioServer.hh"

#include "config/IConfigurator.hh"


namespace workrave::core::rpc
{

class org_workrave_ConfigInterface final : public ::workrave::rpc::dbus::GioInterface
{
public:
  org_workrave_ConfigInterface(::workrave::rpc::dbus::GioServer &server,
              std::string path,
              workrave::config::IConfigurator &implementation);
  ~org_workrave_ConfigInterface() override = default;

  [[nodiscard]] std::string_view name() const noexcept override;
  [[nodiscard]] std::string_view introspection() const noexcept override;
  void dispatch(std::string_view method,
                GVariant *parameters,
                GDBusMethodInvocation *invocation) override;

private:

  void dispatch_RemoveKey(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_RenameKey(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_HasUserValue(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetString(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetBool(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetInt(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetInt64(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetDouble(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetStringWithDefault(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetBoolWithDefault(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetIntWithDefault(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetInt64WithDefault(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetDoubleWithDefault(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_SetString(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_SetInt(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_SetInt64(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_SetBool(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_SetDouble(GVariant *parameters, GDBusMethodInvocation *invocation);




  std::string path_;
  workrave::config::IConfigurator &implementation_;

};

} // namespace workrave::core::rpc
