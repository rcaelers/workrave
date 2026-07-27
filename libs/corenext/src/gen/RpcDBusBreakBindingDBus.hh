// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's DBus backend from an annotated C++ header.
#pragma once

#include <memory>
#include <string>

#include "dbus/IDBus.hh"

#include "Break.hh"


namespace workrave::core::rpc
{

class org_workrave_BreakInterface
{
public:
  virtual ~org_workrave_BreakInterface() = default;
  static org_workrave_BreakInterface *instance(std::shared_ptr<::workrave::dbus::IDBus> dbus);

  virtual void BreakEvent(const std::string &path, workrave::BreakEvent value) = 0;

  virtual void BreakStateChanged(const std::string &path, BreakStage value) = 0;

};
void init_org_workrave_BreakInterface(std::shared_ptr<::workrave::dbus::IDBus> dbus);

} // namespace workrave::core::rpc
