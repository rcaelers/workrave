// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's GDBus backend from an annotated C++ header.
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <array>
#include <chrono>
#include <list>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "rpc/dbus/Error.hh"
#include "rpc/dbus/GioCodec.hh"

#include "RpcDBusControlBindingDBus.hh"



namespace workrave::ui::rpc
{

org_workrave_ControlInterface::org_workrave_ControlInterface(::workrave::rpc::dbus::GioServer &server,
                         std::string path,
                         Menus &implementation)

  : path_(std::move(path))

  , implementation_(implementation)
{

  (void)server;

}

std::string_view
org_workrave_ControlInterface::name() const noexcept
{
  return "org.workrave.ControlInterface";
}

std::string_view
org_workrave_ControlInterface::introspection() const noexcept
{
  static constexpr std::string_view xml =
  "  <interface name=\"org.workrave.ControlInterface\">\n"

  "    <method name=\"OpenMain\">\n"

  "    </method>\n"

  "    <method name=\"Preferences\">\n"

  "    </method>\n"

  "    <method name=\"ReadingMode\">\n"

  "      <arg type=\"b\" name=\"on\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"Statistics\">\n"

  "    </method>\n"

  "    <method name=\"Exercises\">\n"

  "    </method>\n"

  "    <method name=\"RestBreak\">\n"

  "    </method>\n"

  "    <method name=\"Quit\">\n"

  "    </method>\n"

  "    <method name=\"About\">\n"

  "    </method>\n"


  "  </interface>\n";
  return xml;
}

void
org_workrave_ControlInterface::dispatch(std::string_view method, GVariant *parameters, GDBusMethodInvocation *invocation)
{
  using Method = void (org_workrave_ControlInterface::*)(GVariant *, GDBusMethodInvocation *);
  struct Entry { std::string_view name; Method method; };
  static constexpr std::array<Entry, 8> methods = { {

    {.name = "OpenMain", .method = &org_workrave_ControlInterface::dispatch_OpenMain},

    {.name = "Preferences", .method = &org_workrave_ControlInterface::dispatch_Preferences},

    {.name = "ReadingMode", .method = &org_workrave_ControlInterface::dispatch_ReadingMode},

    {.name = "Statistics", .method = &org_workrave_ControlInterface::dispatch_Statistics},

    {.name = "Exercises", .method = &org_workrave_ControlInterface::dispatch_Exercises},

    {.name = "RestBreak", .method = &org_workrave_ControlInterface::dispatch_RestBreak},

    {.name = "Quit", .method = &org_workrave_ControlInterface::dispatch_Quit},

    {.name = "About", .method = &org_workrave_ControlInterface::dispatch_About},

  } };
  for (const auto &entry: methods)
    {
      if (entry.name == method)
        {
          (this->*entry.method)(parameters, invocation);
          return;
        }
    }
  throw ::workrave::rpc::dbus::Error(
    std::string(::workrave::rpc::dbus::error_names::unknown_method),
    "Unknown DBus method: " + std::string(method));
}


void
org_workrave_ControlInterface::dispatch_OpenMain(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.OpenMain");
    }


  implementation_.on_menu_open_main_window();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  ::workrave::rpc::dbus::gio_return_method_value(invocation, reply, reply_fd_list.get());
}


void
org_workrave_ControlInterface::dispatch_Preferences(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.Preferences");
    }


  implementation_.on_menu_preferences();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  ::workrave::rpc::dbus::gio_return_method_value(invocation, reply, reply_fd_list.get());
}


void
org_workrave_ControlInterface::dispatch_ReadingMode(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.ReadingMode");
    }

  bool p_on{};

  p_on = ::workrave::rpc::dbus::gio_decode_child<bool>(parameters, 0);



  implementation_.on_menu_reading(p_on);


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;




  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  ::workrave::rpc::dbus::gio_return_method_value(invocation, reply, reply_fd_list.get());
}


void
org_workrave_ControlInterface::dispatch_Statistics(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.Statistics");
    }


  implementation_.on_menu_statistics();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  ::workrave::rpc::dbus::gio_return_method_value(invocation, reply, reply_fd_list.get());
}


void
org_workrave_ControlInterface::dispatch_Exercises(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.Exercises");
    }


  implementation_.on_menu_exercises();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  ::workrave::rpc::dbus::gio_return_method_value(invocation, reply, reply_fd_list.get());
}


void
org_workrave_ControlInterface::dispatch_RestBreak(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.RestBreak");
    }


  implementation_.on_menu_restbreak_now();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  ::workrave::rpc::dbus::gio_return_method_value(invocation, reply, reply_fd_list.get());
}


void
org_workrave_ControlInterface::dispatch_Quit(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.Quit");
    }


  implementation_.on_menu_quit();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  ::workrave::rpc::dbus::gio_return_method_value(invocation, reply, reply_fd_list.get());
}


void
org_workrave_ControlInterface::dispatch_About(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.About");
    }


  implementation_.on_menu_about();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  ::workrave::rpc::dbus::gio_return_method_value(invocation, reply, reply_fd_list.get());
}


} // namespace workrave::ui::rpc
