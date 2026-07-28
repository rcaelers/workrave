// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's GDBus backend from an annotated C++ header.
#pragma once

#include <string>
#include <string_view>

#include <vector>

#include <boost/signals2/connection.hpp>


#include "rpc/dbus/GioInterface.hh"
#include "rpc/dbus/GioServer.hh"

#include "Core.hh"


namespace workrave::core::legacy_rpc
{

class org_workrave_CoreInterface final : public ::workrave::rpc::dbus::GioInterface
{
public:
  org_workrave_CoreInterface(::workrave::rpc::dbus::GioServer &server,
              std::string path,
              Core &implementation);
  ~org_workrave_CoreInterface() override = default;

  [[nodiscard]] std::string_view name() const noexcept override;
  [[nodiscard]] std::string_view introspection() const noexcept override;
  void dispatch(std::string_view method,
                GVariant *parameters,
                GDBusMethodInvocation *invocation) override;

private:

  void dispatch_IsActive(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_IsTaking(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_ForceBreak(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetActiveOperationMode(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetOperationMode(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_IsOperationModeAnOverride(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_SetOperationMode(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_SetOperationModeFor(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetUsageMode(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_SetUsageMode(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_ReportActivity(GVariant *parameters, GDBusMethodInvocation *invocation);


  void emit_OperationModeChanged(workrave::OperationMode value);

  void emit_UsageModeChanged(workrave::UsageMode value);



  ::workrave::rpc::dbus::GioServer &server_;

  std::string path_;
  Core &implementation_;

  std::vector<boost::signals2::scoped_connection> signal_connections_;

};

} // namespace workrave::core::legacy_rpc
