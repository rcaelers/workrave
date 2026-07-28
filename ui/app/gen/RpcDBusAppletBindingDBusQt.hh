// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's QtDBus backend from an annotated C++ header.
#pragma once

#include <string>
#include <string_view>

#include <vector>

#include <boost/signals2/connection.hpp>


#include "rpc/dbus/QtInterface.hh"
#include "rpc/dbus/QtServer.hh"

#include "GenericDBusApplet.hh"


namespace workrave::ui::rpc
{

class org_workrave_AppletInterface final : public ::workrave::rpc::dbus::QtInterface
{
public:
  org_workrave_AppletInterface(::workrave::rpc::dbus::QtServer &server,
              std::string path,
              GenericDBusApplet &implementation);
  ~org_workrave_AppletInterface() override = default;

  [[nodiscard]] std::string_view name() const noexcept override;
  [[nodiscard]] std::string_view introspection() const noexcept override;
  bool dispatch(const QDBusMessage &message, const QDBusConnection &connection) override;

private:

  void dispatch_Embed(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_Command(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_MenuAction(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_ButtonClicked(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetMenu(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetTrayIconEnabled(const QDBusMessage &message, const QDBusConnection &connection);


  void emit_TimersUpdated(GenericDBusApplet::TimerData micro, GenericDBusApplet::TimerData rest, GenericDBusApplet::TimerData daily);

  void emit_MenuUpdated(std::list<GenericDBusApplet::MenuItem> menuitems);

  void emit_MenuItemUpdated(GenericDBusApplet::MenuItem menuitem);

  void emit_TrayIconUpdated(bool enabled);



  ::workrave::rpc::dbus::QtServer &server_;

  std::string path_;
  GenericDBusApplet &implementation_;

  std::vector<boost::signals2::scoped_connection> signal_connections_;

};

} // namespace workrave::ui::rpc
