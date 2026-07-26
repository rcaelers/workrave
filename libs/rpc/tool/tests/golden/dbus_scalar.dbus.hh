// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's DBus backend from an annotated C++ header.
#pragma once

#include <memory>
#include <string>

#include "dbus/IDBus.hh"

#include "dbus_scalar.hh"

class org_workrave_TestInterface
{
public:
  virtual ~org_workrave_TestInterface() = default;
  static org_workrave_TestInterface *instance(std::shared_ptr<::workrave::dbus::IDBus> dbus);

  virtual void ModeChanged(const std::string &path, TestMode value) = 0;

};