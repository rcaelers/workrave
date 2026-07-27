// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's DBus backend from an annotated C++ header.
#pragma once

#include <memory>
#include <string>

#include "dbus/IDBus.hh"

#include "dbus_struct_sequence.hh"


class org_workrave_TestInterface2
{
public:
  virtual ~org_workrave_TestInterface2() = default;
  static org_workrave_TestInterface2 *instance(std::shared_ptr<::workrave::dbus::IDBus> dbus);

};
void init_org_workrave_TestInterface2(std::shared_ptr<::workrave::dbus::IDBus> dbus);
