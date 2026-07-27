// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's DBus backend from an annotated C++ header.
#pragma once

#include <memory>
#include <string>

#include "dbus/IDBus.hh"

#include "Core.hh"


namespace workrave::core::rpc
{

class org_workrave_CoreInterface
{
public:
  virtual ~org_workrave_CoreInterface() = default;
  static org_workrave_CoreInterface *instance(std::shared_ptr<::workrave::dbus::IDBus> dbus);

  virtual void OperationModeChanged(const std::string &path, workrave::OperationMode value) = 0;

  virtual void UsageModeChanged(const std::string &path, workrave::UsageMode value) = 0;

};
void init_org_workrave_CoreInterface(std::shared_ptr<::workrave::dbus::IDBus> dbus);

} // namespace workrave::core::rpc
