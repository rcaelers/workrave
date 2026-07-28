// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "RpcDBusServer.hh"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Core.hh"
#include "RpcDBusBreakBindingDBus.hh"
#include "RpcDBusConfigBindingDBus.hh"
#include "RpcDBusCoreBindingDBus.hh"
#include "core/CoreConfig.hh"
#if defined(WORKRAVE_RPC_DBUS_GIO)
#  include "rpc/dbus/GioServer.hh"
#else
#  include "rpc/dbus/QtServer.hh"
#endif

namespace generated = workrave::core::rpc;
namespace rpc_dbus = workrave::rpc::dbus;

#if defined(WORKRAVE_RPC_DBUS_GIO)
using NativeDBusServer = rpc_dbus::GioServer;
#else
using NativeDBusServer = rpc_dbus::QtServer;
#endif

namespace
{
  constexpr const char *dbus_service_name = "org.workrave.Workrave";
  constexpr const char *dbus_root_path = "/org/workrave/Workrave";
}

struct RpcDBusServer::Impl
{
  Impl(Core &core, workrave::config::IConfigurator &configurator)
    : server(NativeDBusServer::session())
  {
    if (!server->is_available())
      {
        throw std::runtime_error("RPC DBus session bus is unavailable");
      }
    server->request_name(dbus_service_name);

    const std::string core_path = std::string(dbus_root_path) + "/Core";
    core_binding = std::make_shared<generated::org_workrave_CoreInterface>(*server, core_path, core);
    config_binding =
      std::make_shared<generated::org_workrave_ConfigInterface>(*server, core_path, configurator);
    registrations.emplace_back(server->register_interface(core_path, core_binding));
    registrations.emplace_back(server->register_interface(core_path, config_binding));

    for (workrave::BreakId id = workrave::BREAK_ID_MICRO_BREAK; id < workrave::BREAK_ID_SIZEOF; id++)
      {
        auto break_controller = std::dynamic_pointer_cast<Break>(core.get_break(id));
        if (!break_controller)
          {
            throw std::runtime_error("Core returned an incompatible Break implementation");
          }

        const std::string break_path = std::string(dbus_root_path) + "/Break/" + CoreConfig::get_break_name(id);
        auto binding =
          std::make_shared<generated::org_workrave_BreakInterface>(*server, break_path, *break_controller);
        registrations.emplace_back(server->register_interface(break_path, binding));

        break_bindings.emplace_back(std::move(binding));
      }
  }

  std::unique_ptr<NativeDBusServer> server;
  std::shared_ptr<generated::org_workrave_CoreInterface> core_binding;
  std::shared_ptr<generated::org_workrave_ConfigInterface> config_binding;
  std::vector<std::shared_ptr<generated::org_workrave_BreakInterface>> break_bindings;
  std::vector<rpc_dbus::Registration> registrations;
};

RpcDBusServer::RpcDBusServer(Core &core, workrave::config::IConfigurator &configurator)
  : impl_(std::make_unique<Impl>(core, configurator))
{
}

RpcDBusServer::~RpcDBusServer() = default;
