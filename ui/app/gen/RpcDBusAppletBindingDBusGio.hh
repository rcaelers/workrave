// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's GDBus backend from an annotated C++ header.
#pragma once

#include <string>
#include <string_view>

#include <vector>

#include <boost/signals2/connection.hpp>


#include "rpc/dbus/GioInterface.hh"
#include "rpc/dbus/GioServer.hh"

#include "GenericDBusApplet.hh"


namespace workrave::ui::rpc
{

class org_workrave_AppletInterface final : public ::workrave::rpc::dbus::GioInterface
{
public:
  org_workrave_AppletInterface(::workrave::rpc::dbus::GioServer &server,
              std::string path,
              GenericDBusApplet &implementation);
  ~org_workrave_AppletInterface() override = default;

  [[nodiscard]] std::string_view name() const noexcept override;
  [[nodiscard]] std::string_view introspection() const noexcept override;
  void dispatch(std::string_view method,
                GVariant *parameters,
                GDBusMethodInvocation *invocation) override;

private:

  void dispatch_Embed(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_Command(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_MenuAction(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_ButtonClicked(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetMenu(GVariant *parameters, GDBusMethodInvocation *invocation);

  void dispatch_GetTrayIconEnabled(GVariant *parameters, GDBusMethodInvocation *invocation);


  void emit_TimersUpdated(GenericDBusApplet::TimerData micro, GenericDBusApplet::TimerData rest, GenericDBusApplet::TimerData daily);

  void emit_MenuUpdated(std::list<GenericDBusApplet::MenuItem> menuitems);

  void emit_MenuItemUpdated(GenericDBusApplet::MenuItem menuitem);

  void emit_TrayIconUpdated(bool enabled);



  ::workrave::rpc::dbus::GioServer &server_;

  std::string path_;
  GenericDBusApplet &implementation_;

  std::vector<boost::signals2::scoped_connection> signal_connections_;

};

} // namespace workrave::ui::rpc
