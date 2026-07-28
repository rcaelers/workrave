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

#include "RpcDbusScalarDBus.hh"


namespace workrave::rpc::dbus
{
template<>
struct QtCodec<TestMode>
{
  static TestMode decode(const QVariant &variant)
  {
    const std::string value = QtCodec<std::string>::decode(variant);

    if (value == "idle") return TestMode::Idle;

    if (value == "active") return TestMode::Active;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }
  static void append(QDBusArgument &arg, const TestMode &value)
  {
    std::string str;
    switch (value)
      {

      case TestMode::Idle: str = "idle"; break;

      case TestMode::Active: str = "active"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    arg << QString::fromStdString(str);
  }
  static QVariant encode(const TestMode &value)
  {
    std::string str;
    switch (value)
      {

      case TestMode::Idle: str = "idle"; break;

      case TestMode::Active: str = "active"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    return QVariant::fromValue(QString::fromStdString(str));
  }
};
} // namespace workrave::rpc::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::rpc::dbus, where they'd only be found via TestMode's
// own namespace (which may not be workrave::rpc::dbus at all), silently breaks
// qDBusRegisterMetaType<TestMode>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const TestMode &data)
{
  workrave::rpc::dbus::QtCodec<TestMode>::append(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, TestMode &data)
{
  QString value;
  arg >> value;
  data = workrave::rpc::dbus::QtCodec<TestMode>::decode(QVariant::fromValue(value));
  return arg;
}


org_workrave_TestInterface::org_workrave_TestInterface(::workrave::rpc::dbus::QtServer &server,
                         std::string path,
                         RpcDBusFixture &implementation)

  : server_(server)
  , path_(std::move(path))

  , implementation_(implementation)
{


  qDBusRegisterMetaType<TestMode>();

  signal_connections_.emplace_back(implementation_.signal_mode_changed().connect(
    [this](TestMode value) {
      emit_ModeChanged(value);
    }));

}

std::string_view
org_workrave_TestInterface::name() const noexcept
{
  return "org.workrave.TestInterface";
}

std::string_view
org_workrave_TestInterface::introspection() const noexcept
{
  static constexpr std::string_view xml =

  "  <interface name=\"org.workrave.TestInterface\">\n"

  "\n"

  "    <method name=\"Ping\">\n"

  "\n"

  "      <arg type=\"s\" name=\"message\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetMode\">\n"

  "\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"SetMode\">\n"

  "\n"

  "      <arg type=\"s\" name=\"mode\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "\n"

  "    <signal name=\"ModeChanged\">\n"

  "\n"

  "      <arg type=\"s\" name=\"value\" />\n"

  "\n"

  "    </signal>\n"

  "\n"

  "  </interface>\n";
  return xml;
}

bool
org_workrave_TestInterface::dispatch(const QDBusMessage &message, const QDBusConnection &connection)
{
  using Method = void (org_workrave_TestInterface::*)(const QDBusMessage &, const QDBusConnection &);
  struct Entry
  {
    std::string_view name;
    Method method;
  };
  static constexpr std::array<Entry, 3> methods =
  { {

      {.name = "Ping", .method = &org_workrave_TestInterface::dispatch_Ping},

      {.name = "GetMode", .method = &org_workrave_TestInterface::dispatch_GetMode},

      {.name = "SetMode", .method = &org_workrave_TestInterface::dispatch_SetMode},

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
org_workrave_TestInterface::dispatch_Ping(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_message{};


  std::string p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.TestInterface.Ping");
    }



  p_message = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));




  p_result = implementation_.ping(p_message);


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<std::string>::encode(p_result);





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.TestInterface.Ping");
    }
}


void
org_workrave_TestInterface::dispatch_GetMode(const QDBusMessage &message, const QDBusConnection &connection)
{


  TestMode p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.TestInterface.GetMode");
    }




  p_result = implementation_.get_mode();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<TestMode>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.TestInterface.GetMode");
    }
}


void
org_workrave_TestInterface::dispatch_SetMode(const QDBusMessage &message, const QDBusConnection &connection)
{

  TestMode p_mode{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.TestInterface.SetMode");
    }



  p_mode = ::workrave::rpc::dbus::QtCodec<TestMode>::decode(message.arguments().at(0));




  implementation_.set_mode(p_mode);


  QDBusMessage reply = message.createReply();





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.TestInterface.SetMode");
    }
}


void
org_workrave_TestInterface::emit_ModeChanged(TestMode value)
{
  QVariantList arguments;

  arguments << ::workrave::rpc::dbus::QtCodec<TestMode>::encode(value);

  server_.emit_signal(path_, "org.workrave.TestInterface", "ModeChanged", arguments);
}
