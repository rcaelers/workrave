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

#include "RpcDBusBreakBindingDBus.hh"


namespace workrave::rpc::dbus
{
template<>
struct QtCodec<workrave::BreakEvent>
{
  static workrave::BreakEvent decode(const QVariant &variant)
  {
    const std::string value = QtCodec<std::string>::decode(variant);

    if (value == "show_prelude") return workrave::BreakEvent::ShowPrelude;

    if (value == "show_break") return workrave::BreakEvent::ShowBreak;

    if (value == "show_break_forced") return workrave::BreakEvent::ShowBreakForced;

    if (value == "break_start") return workrave::BreakEvent::BreakStart;

    if (value == "break_idle") return workrave::BreakEvent::BreakIdle;

    if (value == "break_stop") return workrave::BreakEvent::BreakStop;

    if (value == "break_ignored") return workrave::BreakEvent::BreakIgnored;

    if (value == "break_postponed") return workrave::BreakEvent::BreakPostponed;

    if (value == "break_skipped") return workrave::BreakEvent::BreakSkipped;

    if (value == "break_taken") return workrave::BreakEvent::BreakTaken;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }
  static void append(QDBusArgument &arg, const workrave::BreakEvent &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::BreakEvent::ShowPrelude: str = "show_prelude"; break;

      case workrave::BreakEvent::ShowBreak: str = "show_break"; break;

      case workrave::BreakEvent::ShowBreakForced: str = "show_break_forced"; break;

      case workrave::BreakEvent::BreakStart: str = "break_start"; break;

      case workrave::BreakEvent::BreakIdle: str = "break_idle"; break;

      case workrave::BreakEvent::BreakStop: str = "break_stop"; break;

      case workrave::BreakEvent::BreakIgnored: str = "break_ignored"; break;

      case workrave::BreakEvent::BreakPostponed: str = "break_postponed"; break;

      case workrave::BreakEvent::BreakSkipped: str = "break_skipped"; break;

      case workrave::BreakEvent::BreakTaken: str = "break_taken"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    arg << QString::fromStdString(str);
  }
  static QVariant encode(const workrave::BreakEvent &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::BreakEvent::ShowPrelude: str = "show_prelude"; break;

      case workrave::BreakEvent::ShowBreak: str = "show_break"; break;

      case workrave::BreakEvent::ShowBreakForced: str = "show_break_forced"; break;

      case workrave::BreakEvent::BreakStart: str = "break_start"; break;

      case workrave::BreakEvent::BreakIdle: str = "break_idle"; break;

      case workrave::BreakEvent::BreakStop: str = "break_stop"; break;

      case workrave::BreakEvent::BreakIgnored: str = "break_ignored"; break;

      case workrave::BreakEvent::BreakPostponed: str = "break_postponed"; break;

      case workrave::BreakEvent::BreakSkipped: str = "break_skipped"; break;

      case workrave::BreakEvent::BreakTaken: str = "break_taken"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    return QVariant::fromValue(QString::fromStdString(str));
  }
};
} // namespace workrave::rpc::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::rpc::dbus, where they'd only be found via workrave::BreakEvent's
// own namespace (which may not be workrave::rpc::dbus at all), silently breaks
// qDBusRegisterMetaType<workrave::BreakEvent>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const workrave::BreakEvent &data)
{
  workrave::rpc::dbus::QtCodec<workrave::BreakEvent>::append(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, workrave::BreakEvent &data)
{
  QString value;
  arg >> value;
  data = workrave::rpc::dbus::QtCodec<workrave::BreakEvent>::decode(QVariant::fromValue(value));
  return arg;
}

namespace workrave::rpc::dbus
{
template<>
struct QtCodec<BreakStage>
{
  static BreakStage decode(const QVariant &variant)
  {
    const std::string value = QtCodec<std::string>::decode(variant);

    if (value == "none") return BreakStage::None;

    if (value == "snoozed") return BreakStage::Snoozed;

    if (value == "prelude") return BreakStage::Prelude;

    if (value == "taking") return BreakStage::Taking;

    if (value == "delayed") return BreakStage::Delayed;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }
  static void append(QDBusArgument &arg, const BreakStage &value)
  {
    std::string str;
    switch (value)
      {

      case BreakStage::None: str = "none"; break;

      case BreakStage::Snoozed: str = "snoozed"; break;

      case BreakStage::Prelude: str = "prelude"; break;

      case BreakStage::Taking: str = "taking"; break;

      case BreakStage::Delayed: str = "delayed"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    arg << QString::fromStdString(str);
  }
  static QVariant encode(const BreakStage &value)
  {
    std::string str;
    switch (value)
      {

      case BreakStage::None: str = "none"; break;

      case BreakStage::Snoozed: str = "snoozed"; break;

      case BreakStage::Prelude: str = "prelude"; break;

      case BreakStage::Taking: str = "taking"; break;

      case BreakStage::Delayed: str = "delayed"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    return QVariant::fromValue(QString::fromStdString(str));
  }
};
} // namespace workrave::rpc::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::rpc::dbus, where they'd only be found via BreakStage's
// own namespace (which may not be workrave::rpc::dbus at all), silently breaks
// qDBusRegisterMetaType<BreakStage>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const BreakStage &data)
{
  workrave::rpc::dbus::QtCodec<BreakStage>::append(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, BreakStage &data)
{
  QString value;
  arg >> value;
  data = workrave::rpc::dbus::QtCodec<BreakStage>::decode(QVariant::fromValue(value));
  return arg;
}


namespace workrave::core::rpc
{

org_workrave_BreakInterface::org_workrave_BreakInterface(::workrave::rpc::dbus::QtServer &server,
                         std::string path,
                         Break &implementation)

  : server_(server)
  , path_(std::move(path))

  , implementation_(implementation)
{


  qDBusRegisterMetaType<workrave::BreakEvent>();

  qDBusRegisterMetaType<BreakStage>();

  signal_connections_.emplace_back(implementation_.signal_break_event().connect(
    [this](workrave::BreakEvent value) {
      emit_BreakEvent(value);
    }));

  signal_connections_.emplace_back(implementation_.signal_break_stage_changed().connect(
    [this](BreakStage value) {
      emit_BreakStateChanged(value);
    }));

}

std::string_view
org_workrave_BreakInterface::name() const noexcept
{
  return "org.workrave.BreakInterface";
}

std::string_view
org_workrave_BreakInterface::introspection() const noexcept
{
  static constexpr std::string_view xml =

  "  <interface name=\"org.workrave.BreakInterface\">\n"

  "\n"

  "    <method name=\"GetName\">\n"

  "\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"IsEnabled\">\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"IsTimerRunning\">\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"IsTaking\">\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"IsMaxPreludesReached\">\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"IsActive\">\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetTimerElapsed\">\n"

  "\n"

  "      <arg type=\"i\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetTimerIdle\">\n"

  "\n"

  "      <arg type=\"i\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetAutoReset\">\n"

  "\n"

  "      <arg type=\"x\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"IsAutoResetEnabled\">\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetLimit\">\n"

  "\n"

  "      <arg type=\"x\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"IsLimitEnabled\">\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetTimerRemaining\">\n"

  "\n"

  "      <arg type=\"i\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetTimerOverdue\">\n"

  "\n"

  "      <arg type=\"i\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"PostponeBreak\">\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"SkipBreak\">\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetBreakState\">\n"

  "\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "\n"

  "    <signal name=\"BreakEvent\">\n"

  "\n"

  "      <arg type=\"s\" name=\"value\" />\n"

  "\n"

  "    </signal>\n"

  "\n"

  "    <signal name=\"BreakStateChanged\">\n"

  "\n"

  "      <arg type=\"s\" name=\"value\" />\n"

  "\n"

  "    </signal>\n"

  "\n"

  "  </interface>\n";
  return xml;
}

bool
org_workrave_BreakInterface::dispatch(const QDBusMessage &message, const QDBusConnection &connection)
{
  using Method = void (org_workrave_BreakInterface::*)(const QDBusMessage &, const QDBusConnection &);
  struct Entry
  {
    std::string_view name;
    Method method;
  };
  static constexpr std::array<Entry, 17> methods =
  { {

      {.name = "GetName", .method = &org_workrave_BreakInterface::dispatch_GetName},

      {.name = "IsEnabled", .method = &org_workrave_BreakInterface::dispatch_IsEnabled},

      {.name = "IsTimerRunning", .method = &org_workrave_BreakInterface::dispatch_IsTimerRunning},

      {.name = "IsTaking", .method = &org_workrave_BreakInterface::dispatch_IsTaking},

      {.name = "IsMaxPreludesReached", .method = &org_workrave_BreakInterface::dispatch_IsMaxPreludesReached},

      {.name = "IsActive", .method = &org_workrave_BreakInterface::dispatch_IsActive},

      {.name = "GetTimerElapsed", .method = &org_workrave_BreakInterface::dispatch_GetTimerElapsed},

      {.name = "GetTimerIdle", .method = &org_workrave_BreakInterface::dispatch_GetTimerIdle},

      {.name = "GetAutoReset", .method = &org_workrave_BreakInterface::dispatch_GetAutoReset},

      {.name = "IsAutoResetEnabled", .method = &org_workrave_BreakInterface::dispatch_IsAutoResetEnabled},

      {.name = "GetLimit", .method = &org_workrave_BreakInterface::dispatch_GetLimit},

      {.name = "IsLimitEnabled", .method = &org_workrave_BreakInterface::dispatch_IsLimitEnabled},

      {.name = "GetTimerRemaining", .method = &org_workrave_BreakInterface::dispatch_GetTimerRemaining},

      {.name = "GetTimerOverdue", .method = &org_workrave_BreakInterface::dispatch_GetTimerOverdue},

      {.name = "PostponeBreak", .method = &org_workrave_BreakInterface::dispatch_PostponeBreak},

      {.name = "SkipBreak", .method = &org_workrave_BreakInterface::dispatch_SkipBreak},

      {.name = "GetBreakState", .method = &org_workrave_BreakInterface::dispatch_GetBreakState},

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
org_workrave_BreakInterface::dispatch_GetName(const QDBusMessage &message, const QDBusConnection &connection)
{


  std::string p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetName");
    }




  p_result = implementation_.get_name();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<std::string>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.GetName");
    }
}


void
org_workrave_BreakInterface::dispatch_IsEnabled(const QDBusMessage &message, const QDBusConnection &connection)
{


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsEnabled");
    }




  p_result = implementation_.is_enabled();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.IsEnabled");
    }
}


void
org_workrave_BreakInterface::dispatch_IsTimerRunning(const QDBusMessage &message, const QDBusConnection &connection)
{


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsTimerRunning");
    }




  p_result = implementation_.is_running();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.IsTimerRunning");
    }
}


void
org_workrave_BreakInterface::dispatch_IsTaking(const QDBusMessage &message, const QDBusConnection &connection)
{


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsTaking");
    }




  p_result = implementation_.is_taking();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.IsTaking");
    }
}


void
org_workrave_BreakInterface::dispatch_IsMaxPreludesReached(const QDBusMessage &message, const QDBusConnection &connection)
{


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsMaxPreludesReached");
    }




  p_result = implementation_.is_max_preludes_reached();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.IsMaxPreludesReached");
    }
}


void
org_workrave_BreakInterface::dispatch_IsActive(const QDBusMessage &message, const QDBusConnection &connection)
{


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsActive");
    }




  p_result = implementation_.is_active();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.IsActive");
    }
}


void
org_workrave_BreakInterface::dispatch_GetTimerElapsed(const QDBusMessage &message, const QDBusConnection &connection)
{


  int64_t p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetTimerElapsed");
    }




  p_result = implementation_.get_elapsed_time();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<int32_t>::encode(::workrave::rpc::dbus::checked_dbus_wire_cast<int32_t>(p_result));



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.GetTimerElapsed");
    }
}


void
org_workrave_BreakInterface::dispatch_GetTimerIdle(const QDBusMessage &message, const QDBusConnection &connection)
{


  int64_t p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetTimerIdle");
    }




  p_result = implementation_.get_elapsed_idle_time();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<int32_t>::encode(::workrave::rpc::dbus::checked_dbus_wire_cast<int32_t>(p_result));



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.GetTimerIdle");
    }
}


void
org_workrave_BreakInterface::dispatch_GetAutoReset(const QDBusMessage &message, const QDBusConnection &connection)
{


  int64_t p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetAutoReset");
    }




  p_result = implementation_.get_auto_reset();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<int64_t>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.GetAutoReset");
    }
}


void
org_workrave_BreakInterface::dispatch_IsAutoResetEnabled(const QDBusMessage &message, const QDBusConnection &connection)
{


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsAutoResetEnabled");
    }




  p_result = implementation_.is_auto_reset_enabled();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.IsAutoResetEnabled");
    }
}


void
org_workrave_BreakInterface::dispatch_GetLimit(const QDBusMessage &message, const QDBusConnection &connection)
{


  int64_t p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetLimit");
    }




  p_result = implementation_.get_limit();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<int64_t>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.GetLimit");
    }
}


void
org_workrave_BreakInterface::dispatch_IsLimitEnabled(const QDBusMessage &message, const QDBusConnection &connection)
{


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsLimitEnabled");
    }




  p_result = implementation_.is_limit_enabled();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.IsLimitEnabled");
    }
}


void
org_workrave_BreakInterface::dispatch_GetTimerRemaining(const QDBusMessage &message, const QDBusConnection &connection)
{


  int64_t p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetTimerRemaining");
    }




  p_result = implementation_.get_timer_remaining();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<int32_t>::encode(::workrave::rpc::dbus::checked_dbus_wire_cast<int32_t>(p_result));



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.GetTimerRemaining");
    }
}


void
org_workrave_BreakInterface::dispatch_GetTimerOverdue(const QDBusMessage &message, const QDBusConnection &connection)
{


  int64_t p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetTimerOverdue");
    }




  p_result = implementation_.get_total_overdue_time();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<int32_t>::encode(::workrave::rpc::dbus::checked_dbus_wire_cast<int32_t>(p_result));



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.GetTimerOverdue");
    }
}


void
org_workrave_BreakInterface::dispatch_PostponeBreak(const QDBusMessage &message, const QDBusConnection &connection)
{



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.PostponeBreak");
    }




  implementation_.postpone_break();


  QDBusMessage reply = message.createReply();



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.PostponeBreak");
    }
}


void
org_workrave_BreakInterface::dispatch_SkipBreak(const QDBusMessage &message, const QDBusConnection &connection)
{



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.SkipBreak");
    }




  implementation_.skip_break();


  QDBusMessage reply = message.createReply();



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.SkipBreak");
    }
}


void
org_workrave_BreakInterface::dispatch_GetBreakState(const QDBusMessage &message, const QDBusConnection &connection)
{


  std::string p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetBreakState");
    }




  p_result = implementation_.get_break_stage();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<std::string>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.BreakInterface.GetBreakState");
    }
}


void
org_workrave_BreakInterface::emit_BreakEvent(workrave::BreakEvent value)
{
  QVariantList arguments;

  arguments << ::workrave::rpc::dbus::QtCodec<workrave::BreakEvent>::encode(value);

  server_.emit_signal(path_, "org.workrave.BreakInterface", "BreakEvent", arguments);
}
void
org_workrave_BreakInterface::emit_BreakStateChanged(BreakStage value)
{
  QVariantList arguments;

  arguments << ::workrave::rpc::dbus::QtCodec<BreakStage>::encode(value);

  server_.emit_signal(path_, "org.workrave.BreakInterface", "BreakStateChanged", arguments);
}
} // namespace workrave::core::rpc
