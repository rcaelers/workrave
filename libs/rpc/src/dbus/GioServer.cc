// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rpc/dbus/GioServer.hh"

#include <map>
#include <memory>
#include <stdexcept>
#include <utility>

#include "rpc/dbus/Error.hh"
#include "rpc/dbus/GioInterface.hh"

namespace workrave::rpc::dbus
{
  namespace
  {
    std::string take_gerror_message(GError *error, std::string fallback)
    {
      if (error == nullptr)
        {
          return fallback;
        }
      std::string message = error->message;
      g_error_free(error);
      return message;
    }
  }

  class GioServer::State : public std::enable_shared_from_this<GioServer::State>
  {
  public:
    struct InterfaceRegistration
    {
      std::shared_ptr<GioInterface> interface;
      GDBusNodeInfo *node_info{nullptr};
      guint registration_id{0};

      ~InterfaceRegistration()
      {
        if (node_info != nullptr)
          {
            g_dbus_node_info_unref(node_info);
          }
      }
    };

    explicit State(GDBusConnection *connection)
      : connection_(connection)
    {
    }

    ~State()
    {
      for (const auto &[key, registration]: registrations_)
        {
          (void)key;
          g_dbus_connection_unregister_object(connection_, registration->registration_id);
        }
      registrations_.clear();

      if (connection_ != nullptr)
        {
          g_object_unref(connection_);
        }
    }

    Registration register_interface(std::string path, std::shared_ptr<GioInterface> interface)
    {
      if (!interface)
        {
          throw std::invalid_argument("Cannot register a null DBus interface");
        }

      const std::string interface_name(interface->name());
      const auto key = std::make_pair(path, interface_name);
      if (registrations_.contains(key))
        {
          throw Error(std::string(error_names::failed), "DBus interface is already registered: " + interface_name);
        }

      std::string xml = "<node>";
      xml.append(interface->introspection());
      xml.append("</node>");
      GError *error = nullptr;
      GDBusNodeInfo *node_info = g_dbus_node_info_new_for_xml(xml.c_str(), &error);
      if (node_info == nullptr)
        {
          throw Error(std::string(error_names::failed), take_gerror_message(error, "Invalid DBus introspection XML"));
        }

      GDBusInterfaceInfo *interface_info = g_dbus_node_info_lookup_interface(node_info, interface_name.c_str());
      if (interface_info == nullptr)
        {
          g_dbus_node_info_unref(node_info);
          throw Error(std::string(error_names::failed),
                      "Introspection XML does not contain interface " + interface_name);
        }

      auto registration = std::make_unique<InterfaceRegistration>();
      registration->interface = std::move(interface);
      registration->node_info = node_info;
      registration->registration_id = g_dbus_connection_register_object(connection_,
                                                                          path.c_str(),
                                                                          interface_info,
                                                                          &interface_vtable_,
                                                                          registration.get(),
                                                                          nullptr,
                                                                          &error);
      if (registration->registration_id == 0)
        {
          throw Error(std::string(error_names::failed),
                      take_gerror_message(error, "Unable to register DBus object"));
        }

      registrations_.emplace(key, std::move(registration));
      std::weak_ptr<State> weak_state = shared_from_this();
      return Registration([weak_state, path = std::move(path), interface_name] {
        if (const auto state = weak_state.lock())
          {
            state->unregister_interface(path, interface_name);
          }
      });
    }

    void unregister_interface(std::string_view path, std::string_view interface_name) noexcept
    {
      const auto key = std::make_pair(std::string(path), std::string(interface_name));
      const auto found = registrations_.find(key);
      if (found == registrations_.end())
        {
          return;
        }
      g_dbus_connection_unregister_object(connection_, found->second->registration_id);
      registrations_.erase(found);
    }

    static void on_method_call(GDBusConnection *connection,
                               const gchar *sender,
                               const gchar *object_path,
                               const gchar *interface_name,
                               const gchar *method_name,
                               GVariant *parameters,
                               GDBusMethodInvocation *invocation,
                               gpointer user_data)
    {
      (void)connection;
      (void)sender;
      (void)object_path;
      (void)interface_name;
      auto *registration = static_cast<InterfaceRegistration *>(user_data);
      try
        {
          registration->interface->dispatch(method_name, parameters, invocation);
        }
      catch (const Error &error)
        {
          g_dbus_method_invocation_return_dbus_error(invocation, error.name().c_str(), error.what());
        }
      catch (const std::exception &error)
        {
          const std::string name(error_names::failed);
          g_dbus_method_invocation_return_dbus_error(invocation, name.c_str(), error.what());
        }
    }

    GDBusConnection *connection_{nullptr};
    std::map<std::pair<std::string, std::string>, std::unique_ptr<InterfaceRegistration>> registrations_;
    static const GDBusInterfaceVTable interface_vtable_;
  };

  const GDBusInterfaceVTable GioServer::State::interface_vtable_ = {
    &GioServer::State::on_method_call,
    nullptr,
    nullptr,
    {nullptr},
  };

  GioServer::GioServer(std::shared_ptr<State> state)
    : state_(std::move(state))
  {
  }

  GioServer::~GioServer() = default;

  std::unique_ptr<GioServer> GioServer::session()
  {
    GError *error = nullptr;
    GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
    if (connection == nullptr)
      {
        throw Error(std::string(error_names::failed),
                    take_gerror_message(error, "Unable to connect to the session bus"));
      }
    return std::unique_ptr<GioServer>(new GioServer(std::make_shared<State>(connection)));
  }

  bool GioServer::is_available() const noexcept
  {
    return state_->connection_ != nullptr && !g_dbus_connection_is_closed(state_->connection_);
  }

  void GioServer::request_name(std::string_view name)
  {
    const std::string owned_name(name);
    constexpr guint do_not_queue = 4;
    GError *error = nullptr;
    GVariant *reply = g_dbus_connection_call_sync(state_->connection_,
                                                  "org.freedesktop.DBus",
                                                  "/org/freedesktop/DBus",
                                                  "org.freedesktop.DBus",
                                                  "RequestName",
                                                  g_variant_new("(su)", owned_name.c_str(), do_not_queue),
                                                  G_VARIANT_TYPE("(u)"),
                                                  G_DBUS_CALL_FLAGS_NONE,
                                                  -1,
                                                  nullptr,
                                                  &error);
    if (reply == nullptr)
      {
        throw Error(std::string(error_names::failed),
                    take_gerror_message(error, "Unable to acquire DBus service name " + owned_name));
      }

    guint result = 0;
    g_variant_get(reply, "(u)", &result);
    g_variant_unref(reply);
    constexpr guint primary_owner = 1;
    constexpr guint already_owner = 4;
    if (result != primary_owner && result != already_owner)
      {
        throw Error(std::string(error_names::failed), "DBus service name is already owned: " + owned_name);
      }
  }

  Registration GioServer::register_interface(std::string path, std::shared_ptr<GioInterface> interface)
  {
    return state_->register_interface(std::move(path), std::move(interface));
  }

  Registration GioServer::watch_name(std::string name, std::function<void(bool)> callback)
  {
    struct WatchState
    {
      std::function<void(bool)> callback;
    };

    auto state = std::make_shared<WatchState>(WatchState{std::move(callback)});
    auto *holder = new std::shared_ptr<WatchState>(state);
    const guint watch_id = g_bus_watch_name_on_connection(
      state_->connection_,
      name.c_str(),
      G_BUS_NAME_WATCHER_FLAGS_NONE,
      [](GDBusConnection *, const gchar *, const gchar *, gpointer user_data) {
        (*static_cast<std::shared_ptr<WatchState> *>(user_data))->callback(true);
      },
      [](GDBusConnection *, const gchar *, gpointer user_data) {
        (*static_cast<std::shared_ptr<WatchState> *>(user_data))->callback(false);
      },
      holder,
      [](gpointer user_data) { delete static_cast<std::shared_ptr<WatchState> *>(user_data); });

    return Registration([watch_id] { g_bus_unwatch_name(watch_id); });
  }

  void GioServer::emit_signal(std::string_view path,
                              std::string_view interface_name,
                              std::string_view signal_name,
                              GVariant *parameters,
                              GUnixFDList *fd_list)
  {
    GError *error = nullptr;
    const std::string object_path(path);
    const std::string interface(interface_name);
    const std::string signal(signal_name);
    bool sent = false;
    if (fd_list == nullptr)
      {
        sent = g_dbus_connection_emit_signal(state_->connection_,
                                             nullptr,
                                             object_path.c_str(),
                                             interface.c_str(),
                                             signal.c_str(),
                                             parameters,
                                             &error);
      }
#if defined(G_OS_UNIX)
    else
      {
        GDBusMessage *message = g_dbus_message_new_signal(object_path.c_str(),
                                                          interface.c_str(),
                                                          signal.c_str());
        g_dbus_message_set_body(message, parameters);
        g_dbus_message_set_unix_fd_list(message, fd_list);
        sent = g_dbus_connection_send_message(state_->connection_,
                                               message,
                                               G_DBUS_SEND_MESSAGE_FLAGS_NONE,
                                               nullptr,
                                               &error);
        g_object_unref(message);
      }
#else
    else
      {
        throw Error(std::string(error_names::failed),
                    "D-Bus UNIX_FD arguments are not supported on this platform");
      }
#endif
    if (!sent)
      {
        throw Error(std::string(error_names::failed),
                    take_gerror_message(error, "Unable to send DBus signal " + signal));
      }
  }
}
