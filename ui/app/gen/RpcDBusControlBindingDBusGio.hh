// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's GDBus backend from an annotated C++ header.
#pragma once

#include <string>
#include <string_view>


#include "rpc/dbus/GioInterface.hh"
#include "rpc/dbus/GioServer.hh"

#include "Menus.hh"


namespace workrave::ui::rpc
{

class org_workrave_ControlInterface final : public ::workrave::rpc::dbus::GioInterface
{
public:
  org_workrave_ControlInterface(::workrave::rpc::dbus::GioServer &server,
              std::string path,
              Menus &implementation);
  ~org_workrave_ControlInterface() override = default;

  [[nodiscard]] std::string_view name() const noexcept override;
  [[nodiscard]] std::string_view introspection() const noexcept override;
  void dispatch(std::string_view method,
                GVariant *parameters,
                GDBusMethodInvocation *invocation) override;

private:

  void dispatch_OpenMain(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_Preferences(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_ReadingMode(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_Statistics(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_Exercises(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_RestBreak(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_Quit(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_About(GVariant *parameters, GDBusMethodInvocation *invocation);




  std::string path_;
  Menus &implementation_;

};

} // namespace workrave::ui::rpc
