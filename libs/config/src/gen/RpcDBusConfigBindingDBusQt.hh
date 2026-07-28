// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's QtDBus backend from an annotated C++ header.
#pragma once

#include <string>
#include <string_view>


#include "rpc/dbus/QtInterface.hh"
#include "rpc/dbus/QtServer.hh"

#include "config/IConfigurator.hh"


namespace workrave::core::rpc
{

class org_workrave_ConfigInterface final : public ::workrave::rpc::dbus::QtInterface
{
public:
  org_workrave_ConfigInterface(::workrave::rpc::dbus::QtServer &server,
              std::string path,
              workrave::config::IConfigurator &implementation);
  ~org_workrave_ConfigInterface() override = default;

  [[nodiscard]] std::string_view name() const noexcept override;
  [[nodiscard]] std::string_view introspection() const noexcept override;
  bool dispatch(const QDBusMessage &message, const QDBusConnection &connection) override;

private:

  void dispatch_RemoveKey(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_RenameKey(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_HasUserValue(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetString(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetBool(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetInt(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetInt64(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetDouble(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetStringWithDefault(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetBoolWithDefault(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetIntWithDefault(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetInt64WithDefault(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_GetDoubleWithDefault(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_SetString(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_SetInt(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_SetInt64(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_SetBool(const QDBusMessage &message, const QDBusConnection &connection);

  void dispatch_SetDouble(const QDBusMessage &message, const QDBusConnection &connection);




  std::string path_;
  workrave::config::IConfigurator &implementation_;

};

} // namespace workrave::core::rpc
