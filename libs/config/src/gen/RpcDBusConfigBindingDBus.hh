// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's DBus backend from an annotated C++ header.
#pragma once

#include <memory>
#include <string>

#include "dbus/IDBus.hh"

#include "config/IConfigurator.hh"


namespace workrave::core::rpc
{

class org_workrave_ConfigInterface
{
public:
  virtual ~org_workrave_ConfigInterface() = default;
  static org_workrave_ConfigInterface *instance(std::shared_ptr<::workrave::dbus::IDBus> dbus);

};
void init_org_workrave_ConfigInterface(std::shared_ptr<::workrave::dbus::IDBus> dbus);

} // namespace workrave::core::rpc
