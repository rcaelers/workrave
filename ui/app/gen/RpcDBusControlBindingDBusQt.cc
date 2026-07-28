// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's QtDBus backend from an annotated C++ header.
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
#include "rpc/dbus/QtCodec.hh"

#include "RpcDBusControlBindingDBus.hh"



namespace workrave::ui::rpc
{

org_workrave_ControlInterface::org_workrave_ControlInterface(::workrave::rpc::dbus::QtServer &server,
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

  "\n"

  "    <method name=\"OpenMain\">\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"Preferences\">\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"ReadingMode\">\n"

  "\n"

  "      <arg type=\"b\" name=\"on\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"Statistics\">\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"Exercises\">\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"RestBreak\">\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"Quit\">\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"About\">\n"

  "\n"

  "    </method>\n"

  "\n"

  "\n"

  "  </interface>\n";
  return xml;
}

bool
org_workrave_ControlInterface::dispatch(const QDBusMessage &message, const QDBusConnection &connection)
{
  using Method = void (org_workrave_ControlInterface::*)(const QDBusMessage &, const QDBusConnection &);
  struct Entry
  {
    std::string_view name;
    Method method;
  };
  static constexpr std::array<Entry, 8> methods =
  { {

      {.name = "OpenMain", .method = &org_workrave_ControlInterface::dispatch_OpenMain},

      {.name = "Preferences", .method = &org_workrave_ControlInterface::dispatch_Preferences},

      {.name = "ReadingMode", .method = &org_workrave_ControlInterface::dispatch_ReadingMode},

      {.name = "Statistics", .method = &org_workrave_ControlInterface::dispatch_Statistics},

      {.name = "Exercises", .method = &org_workrave_ControlInterface::dispatch_Exercises},

      {.name = "RestBreak", .method = &org_workrave_ControlInterface::dispatch_RestBreak},

      {.name = "Quit", .method = &org_workrave_ControlInterface::dispatch_Quit},

      {.name = "About", .method = &org_workrave_ControlInterface::dispatch_About},

  } };

  const std::string method_name = message.member().toStdString();
  for (const auto &entry: methods)
    {
      if (entry.name == method_name)
        {
          (this->*entry.method)(message, connection);
          return true;
        }
    }
  return false;
}


void
org_workrave_ControlInterface::dispatch_OpenMain(const QDBusMessage &message, const QDBusConnection &connection)
{



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.OpenMain");
    }




  implementation_.on_menu_open_main_window();


  QDBusMessage reply = message.createReply();



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ControlInterface.OpenMain");
    }
}


void
org_workrave_ControlInterface::dispatch_Preferences(const QDBusMessage &message, const QDBusConnection &connection)
{



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.Preferences");
    }




  implementation_.on_menu_preferences();


  QDBusMessage reply = message.createReply();



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ControlInterface.Preferences");
    }
}


void
org_workrave_ControlInterface::dispatch_ReadingMode(const QDBusMessage &message, const QDBusConnection &connection)
{

  bool p_on{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.ReadingMode");
    }



  p_on = ::workrave::rpc::dbus::QtCodec<bool>::decode(message.arguments().at(0));




  implementation_.on_menu_reading(p_on);


  QDBusMessage reply = message.createReply();





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ControlInterface.ReadingMode");
    }
}


void
org_workrave_ControlInterface::dispatch_Statistics(const QDBusMessage &message, const QDBusConnection &connection)
{



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.Statistics");
    }




  implementation_.on_menu_statistics();


  QDBusMessage reply = message.createReply();



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ControlInterface.Statistics");
    }
}


void
org_workrave_ControlInterface::dispatch_Exercises(const QDBusMessage &message, const QDBusConnection &connection)
{



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.Exercises");
    }




  implementation_.on_menu_exercises();


  QDBusMessage reply = message.createReply();



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ControlInterface.Exercises");
    }
}


void
org_workrave_ControlInterface::dispatch_RestBreak(const QDBusMessage &message, const QDBusConnection &connection)
{



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.RestBreak");
    }




  implementation_.on_menu_restbreak_now();


  QDBusMessage reply = message.createReply();



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ControlInterface.RestBreak");
    }
}


void
org_workrave_ControlInterface::dispatch_Quit(const QDBusMessage &message, const QDBusConnection &connection)
{



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.Quit");
    }




  implementation_.on_menu_quit();


  QDBusMessage reply = message.createReply();



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ControlInterface.Quit");
    }
}


void
org_workrave_ControlInterface::dispatch_About(const QDBusMessage &message, const QDBusConnection &connection)
{



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ControlInterface.About");
    }




  implementation_.on_menu_about();


  QDBusMessage reply = message.createReply();



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ControlInterface.About");
    }
}


} // namespace workrave::ui::rpc
