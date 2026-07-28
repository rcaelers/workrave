// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's QtDBus backend from an annotated C++ header.
#pragma once

#include <string>
#include <string_view>


#include "rpc/dbus/QtInterface.hh"
#include "rpc/dbus/QtServer.hh"

#include "Menus.hh"


namespace workrave::ui::rpc
{

class org_workrave_ControlInterface final : public ::workrave::rpc::dbus::QtInterface
{
public:
  org_workrave_ControlInterface(::workrave::rpc::dbus::QtServer &server,
              std::string path,
              Menus &implementation);
  ~org_workrave_ControlInterface() override = default;

  [[nodiscard]] std::string_view name() const noexcept override;
  [[nodiscard]] std::string_view introspection() const noexcept override;
  bool dispatch(const QDBusMessage &message, const QDBusConnection &connection) override;

private:

  void dispatch_OpenMain(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_Preferences(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_ReadingMode(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_Statistics(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_Exercises(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_RestBreak(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_Quit(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_About(const QDBusMessage &message, const QDBusConnection &connection);




  std::string path_;
  Menus &implementation_;

};

} // namespace workrave::ui::rpc
