// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "RpcDBusServer.hh"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <boost/signals2.hpp>

#include "Core.hh"
#include "RpcDBusBreakBindingDBus.hh"
#include "RpcDBusConfigBindingDBus.hh"
#include "RpcDBusCoreBindingDBus.hh"
#include "core/CoreConfig.hh"

namespace generated = workrave::core::rpc;

namespace
{
  constexpr const char *dbus_root_path = "/org/workrave/Workrave";
}

struct RpcDBusServer::Impl
{
  Impl(Core &core, workrave::config::IConfigurator &configurator, std::shared_ptr<workrave::dbus::IDBus> primary_bus)
    : bus(std::move(primary_bus))
  {
    if (!bus)
      {
        throw std::invalid_argument("RPC DBus requires a bus instance");
      }
    generated::init_org_workrave_CoreInterface(bus);
    generated::init_org_workrave_BreakInterface(bus);
    generated::init_org_workrave_ConfigInterface(bus);

    const std::string core_path = std::string(dbus_root_path) + "/Core";
    bus->connect(core_path, "org.workrave.CoreInterface", &core);
    bus->connect(core_path, "org.workrave.ConfigInterface", &configurator);
    bus->register_object_path(core_path);

    auto *core_interface = generated::org_workrave_CoreInterface::instance(bus);
    if (core_interface == nullptr)
      {
        throw std::runtime_error("generated Core DBus binding was not registered");
      }
    signal_connections.emplace_back(core.signal_operation_mode_changed().connect(
      [core_interface, core_path](workrave::OperationMode mode) { core_interface->OperationModeChanged(core_path, mode); }));
    signal_connections.emplace_back(core.signal_usage_mode_changed().connect(
      [core_interface, core_path](workrave::UsageMode mode) { core_interface->UsageModeChanged(core_path, mode); }));

    auto *break_interface = generated::org_workrave_BreakInterface::instance(bus);
    if (break_interface == nullptr)
      {
        throw std::runtime_error("generated Break DBus binding was not registered");
      }

    for (workrave::BreakId id = workrave::BREAK_ID_MICRO_BREAK; id < workrave::BREAK_ID_SIZEOF; id++)
      {
        auto break_controller = std::dynamic_pointer_cast<Break>(core.get_break(id));
        if (!break_controller)
          {
            throw std::runtime_error("Core returned an incompatible Break implementation");
          }

        const std::string break_path = std::string(dbus_root_path) + "/Break/" + CoreConfig::get_break_name(id);
        bus->connect(break_path, "org.workrave.BreakInterface", break_controller.get());
        bus->register_object_path(break_path);

        signal_connections.emplace_back(
          break_controller->signal_break_stage_changed().connect([break_interface, break_path](BreakStage state) {
            break_interface->BreakStateChanged(break_path, state);
          }));
        signal_connections.emplace_back(break_controller->signal_break_event().connect(
          [break_interface, break_path](workrave::BreakEvent event) { break_interface->BreakEvent(break_path, event); }));
      }
  }

  std::shared_ptr<workrave::dbus::IDBus> bus;
  std::vector<boost::signals2::scoped_connection> signal_connections;
};

RpcDBusServer::RpcDBusServer(Core &core,
                             workrave::config::IConfigurator &configurator,
                             std::shared_ptr<workrave::dbus::IDBus> bus)
  : impl_(std::make_unique<Impl>(core, configurator, std::move(bus)))
{
}

RpcDBusServer::~RpcDBusServer() = default;
