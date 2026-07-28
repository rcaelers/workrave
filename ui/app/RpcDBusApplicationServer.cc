// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "RpcDBusApplicationServer.hh"

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "GenericDBusApplet.hh"
#include "Menus.hh"
#include "RpcDBusAppletBindingDBus.hh"
#include "RpcDBusControlBindingDBus.hh"
#if defined(WORKRAVE_RPC_DBUS_GIO)
#  include "rpc/dbus/GioServer.hh"
#else
#  include "rpc/dbus/QtServer.hh"
#endif

namespace generated = workrave::ui::rpc;
namespace rpc_dbus = workrave::rpc::dbus;

#if defined(WORKRAVE_RPC_DBUS_GIO)
using NativeDBusServer = rpc_dbus::GioServer;
#else
using NativeDBusServer = rpc_dbus::QtServer;
#endif

namespace
{
  constexpr const char *dbus_ui_path = "/org/workrave/Workrave/UI";
}

struct RpcDBusApplicationServer::Impl
{
  explicit Impl(Menus &menus)
    : server(NativeDBusServer::session())
  {
    if (!server->is_available())
      {
        throw std::runtime_error("RPC DBus session bus is unavailable");
      }
    binding = std::make_shared<generated::org_workrave_ControlInterface>(*server, dbus_ui_path, menus);
    registration = server->register_interface(dbus_ui_path, binding);
  }

  ~Impl()
  {
    unregister_applet();
  }

  void register_applet(GenericDBusApplet &applet)
  {
    unregister_applet();
    applet_binding = std::make_shared<generated::org_workrave_AppletInterface>(*server, dbus_ui_path, applet);
    applet_registration = server->register_interface(dbus_ui_path, applet_binding);
    applet.set_name_watcher([this](std::string name, std::function<void(bool)> callback) {
      return server->watch_name(std::move(name), std::move(callback));
    });
    applet_implementation = &applet;
    applet.publish_state();
  }

  void unregister_applet()
  {
    if (applet_implementation != nullptr)
      {
        applet_implementation->set_name_watcher({});
        applet_implementation = nullptr;
      }
    applet_registration.reset();
    applet_binding.reset();
  }

  std::unique_ptr<NativeDBusServer> server;
  std::shared_ptr<generated::org_workrave_ControlInterface> binding;
  rpc_dbus::Registration registration;
  GenericDBusApplet *applet_implementation{nullptr};
  std::shared_ptr<generated::org_workrave_AppletInterface> applet_binding;
  rpc_dbus::Registration applet_registration;
};

RpcDBusApplicationServer::RpcDBusApplicationServer(Menus &menus)
  : impl_(std::make_unique<Impl>(menus))
{
}

RpcDBusApplicationServer::~RpcDBusApplicationServer() = default;

void
RpcDBusApplicationServer::register_applet(GenericDBusApplet &applet)
{
  impl_->register_applet(applet);
}

void
RpcDBusApplicationServer::unregister_applet()
{
  impl_->unregister_applet();
}
