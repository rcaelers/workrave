// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

class Menus;
class GenericDBusApplet;

// Owns the generated application-level DBus endpoints. Core, Break, and
// Config endpoints are owned separately by RpcDBusServer in CoreNext.
class RpcDBusApplicationServer
{
public:
  explicit RpcDBusApplicationServer(Menus &menus);
  ~RpcDBusApplicationServer();

  RpcDBusApplicationServer(const RpcDBusApplicationServer &) = delete;
  RpcDBusApplicationServer &operator=(const RpcDBusApplicationServer &) = delete;

  void register_applet(GenericDBusApplet &applet);
  void unregister_applet();

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
