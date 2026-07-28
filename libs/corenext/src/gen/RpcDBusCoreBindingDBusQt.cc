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

#include "rpc/Duration.hh"

#include "RpcDBusCoreBindingDBus.hh"


namespace workrave::rpc::dbus
{
template<>
struct QtCodec<workrave::BreakId>
{
  static workrave::BreakId decode(const QVariant &variant)
  {
    const std::string value = QtCodec<std::string>::decode(variant);

    if (value == "none") return workrave::BreakId::BREAK_ID_NONE;

    if (value == "microbreak") return workrave::BreakId::BREAK_ID_MICRO_BREAK;

    if (value == "restbreak") return workrave::BreakId::BREAK_ID_REST_BREAK;

    if (value == "dailylimit") return workrave::BreakId::BREAK_ID_DAILY_LIMIT;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }
  static void append(QDBusArgument &arg, const workrave::BreakId &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::BreakId::BREAK_ID_NONE: str = "none"; break;

      case workrave::BreakId::BREAK_ID_MICRO_BREAK: str = "microbreak"; break;

      case workrave::BreakId::BREAK_ID_REST_BREAK: str = "restbreak"; break;

      case workrave::BreakId::BREAK_ID_DAILY_LIMIT: str = "dailylimit"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    arg << QString::fromStdString(str);
  }
  static QVariant encode(const workrave::BreakId &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::BreakId::BREAK_ID_NONE: str = "none"; break;

      case workrave::BreakId::BREAK_ID_MICRO_BREAK: str = "microbreak"; break;

      case workrave::BreakId::BREAK_ID_REST_BREAK: str = "restbreak"; break;

      case workrave::BreakId::BREAK_ID_DAILY_LIMIT: str = "dailylimit"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    return QVariant::fromValue(QString::fromStdString(str));
  }
};
} // namespace workrave::rpc::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::rpc::dbus, where they'd only be found via workrave::BreakId's
// own namespace (which may not be workrave::rpc::dbus at all), silently breaks
// qDBusRegisterMetaType<workrave::BreakId>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const workrave::BreakId &data)
{
  workrave::rpc::dbus::QtCodec<workrave::BreakId>::append(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, workrave::BreakId &data)
{
  QString value;
  arg >> value;
  data = workrave::rpc::dbus::QtCodec<workrave::BreakId>::decode(QVariant::fromValue(value));
  return arg;
}

namespace workrave::rpc::dbus
{
template<>
struct QtCodec<workrave::BreakHint>
{
  static workrave::BreakHint decode(const QVariant &variant)
  {
    const std::string value = QtCodec<std::string>::decode(variant);

    if (value == "normal") return workrave::BreakHint::Normal;

    if (value == "userinitiated") return workrave::BreakHint::UserInitiated;

    if (value == "naturalbreak") return workrave::BreakHint::NaturalBreak;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }
  static void append(QDBusArgument &arg, const workrave::BreakHint &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::BreakHint::Normal: str = "normal"; break;

      case workrave::BreakHint::UserInitiated: str = "userinitiated"; break;

      case workrave::BreakHint::NaturalBreak: str = "naturalbreak"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    arg << QString::fromStdString(str);
  }
  static QVariant encode(const workrave::BreakHint &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::BreakHint::Normal: str = "normal"; break;

      case workrave::BreakHint::UserInitiated: str = "userinitiated"; break;

      case workrave::BreakHint::NaturalBreak: str = "naturalbreak"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    return QVariant::fromValue(QString::fromStdString(str));
  }
};
} // namespace workrave::rpc::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::rpc::dbus, where they'd only be found via workrave::BreakHint's
// own namespace (which may not be workrave::rpc::dbus at all), silently breaks
// qDBusRegisterMetaType<workrave::BreakHint>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const workrave::BreakHint &data)
{
  workrave::rpc::dbus::QtCodec<workrave::BreakHint>::append(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, workrave::BreakHint &data)
{
  QString value;
  arg >> value;
  data = workrave::rpc::dbus::QtCodec<workrave::BreakHint>::decode(QVariant::fromValue(value));
  return arg;
}

namespace workrave::rpc::dbus
{
template<>
struct QtCodec<workrave::utils::Flags<workrave::BreakHint>>
{
  static workrave::utils::Flags<workrave::BreakHint> decode(const QVariant &variant)
  {
    std::list<workrave::BreakHint> items = QtCodec<std::list<workrave::BreakHint>>::decode(variant);
    workrave::utils::Flags<workrave::BreakHint> result;
    for (const auto &item : items)
      {
        result |= item;
      }
    return result;
  }
  static void append(QDBusArgument &arg, const workrave::utils::Flags<workrave::BreakHint> &value)
  {
    std::list<workrave::BreakHint> items;
    for (unsigned int b = 0; b < workrave::utils::enum_traits<workrave::BreakHint>::bits; ++b)
      {
        auto e = static_cast<workrave::BreakHint>(1 << b);
        if (value.is_set(e))
          {
            items.push_back(e);
          }
      }
    QtCodec<std::list<workrave::BreakHint>>::append(arg, items);
  }
  static QVariant encode(const workrave::utils::Flags<workrave::BreakHint> &value)
  {
    QDBusArgument arg;
    append(arg, value);
    return QVariant::fromValue(arg);
  }
};
} // namespace workrave::rpc::dbus

namespace workrave::rpc::dbus
{
template<>
struct QtCodec<workrave::OperationMode>
{
  static workrave::OperationMode decode(const QVariant &variant)
  {
    const std::string value = QtCodec<std::string>::decode(variant);

    if (value == "normal") return workrave::OperationMode::Normal;

    if (value == "suspended") return workrave::OperationMode::Suspended;

    if (value == "quiet") return workrave::OperationMode::Quiet;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }
  static void append(QDBusArgument &arg, const workrave::OperationMode &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::OperationMode::Normal: str = "normal"; break;

      case workrave::OperationMode::Suspended: str = "suspended"; break;

      case workrave::OperationMode::Quiet: str = "quiet"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    arg << QString::fromStdString(str);
  }
  static QVariant encode(const workrave::OperationMode &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::OperationMode::Normal: str = "normal"; break;

      case workrave::OperationMode::Suspended: str = "suspended"; break;

      case workrave::OperationMode::Quiet: str = "quiet"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    return QVariant::fromValue(QString::fromStdString(str));
  }
};
} // namespace workrave::rpc::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::rpc::dbus, where they'd only be found via workrave::OperationMode's
// own namespace (which may not be workrave::rpc::dbus at all), silently breaks
// qDBusRegisterMetaType<workrave::OperationMode>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const workrave::OperationMode &data)
{
  workrave::rpc::dbus::QtCodec<workrave::OperationMode>::append(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, workrave::OperationMode &data)
{
  QString value;
  arg >> value;
  data = workrave::rpc::dbus::QtCodec<workrave::OperationMode>::decode(QVariant::fromValue(value));
  return arg;
}

namespace workrave::rpc::dbus
{
template<>
struct QtCodec<std::chrono::minutes>
{
  static std::chrono::minutes decode(const QVariant &variant)
  {
    std::string text = QtCodec<std::string>::decode(variant);
    return std::chrono::duration_cast<std::chrono::minutes>(::rpc::parse_duration(text));
  }
};
} // namespace workrave::rpc::dbus

namespace workrave::rpc::dbus
{
template<>
struct QtCodec<workrave::UsageMode>
{
  static workrave::UsageMode decode(const QVariant &variant)
  {
    const std::string value = QtCodec<std::string>::decode(variant);

    if (value == "normal") return workrave::UsageMode::Normal;

    if (value == "reading") return workrave::UsageMode::Reading;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }
  static void append(QDBusArgument &arg, const workrave::UsageMode &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::UsageMode::Normal: str = "normal"; break;

      case workrave::UsageMode::Reading: str = "reading"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    arg << QString::fromStdString(str);
  }
  static QVariant encode(const workrave::UsageMode &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::UsageMode::Normal: str = "normal"; break;

      case workrave::UsageMode::Reading: str = "reading"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    return QVariant::fromValue(QString::fromStdString(str));
  }
};
} // namespace workrave::rpc::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::rpc::dbus, where they'd only be found via workrave::UsageMode's
// own namespace (which may not be workrave::rpc::dbus at all), silently breaks
// qDBusRegisterMetaType<workrave::UsageMode>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const workrave::UsageMode &data)
{
  workrave::rpc::dbus::QtCodec<workrave::UsageMode>::append(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, workrave::UsageMode &data)
{
  QString value;
  arg >> value;
  data = workrave::rpc::dbus::QtCodec<workrave::UsageMode>::decode(QVariant::fromValue(value));
  return arg;
}


namespace workrave::core::rpc
{

org_workrave_CoreInterface::org_workrave_CoreInterface(::workrave::rpc::dbus::QtServer &server,
                         std::string path,
                         Core &implementation)

  : server_(server)
  , path_(std::move(path))

  , implementation_(implementation)
{


  qDBusRegisterMetaType<workrave::BreakId>();

  qDBusRegisterMetaType<workrave::BreakHint>();

  qDBusRegisterMetaType<workrave::OperationMode>();

  qDBusRegisterMetaType<workrave::UsageMode>();

  signal_connections_.emplace_back(implementation_.signal_operation_mode_changed().connect(
    [this](workrave::OperationMode value) {
      emit_OperationModeChanged(value);
    }));

  signal_connections_.emplace_back(implementation_.signal_usage_mode_changed().connect(
    [this](workrave::UsageMode value) {
      emit_UsageModeChanged(value);
    }));

}

std::string_view
org_workrave_CoreInterface::name() const noexcept
{
  return "org.workrave.CoreInterface";
}

std::string_view
org_workrave_CoreInterface::introspection() const noexcept
{
  static constexpr std::string_view xml =

  "  <interface name=\"org.workrave.CoreInterface\">\n"

  "\n"

  "    <method name=\"ForceBreak\">\n"

  "\n"

  "      <arg type=\"s\" name=\"id\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"as\" name=\"break_hint\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"IsActive\">\n"

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

  "    <method name=\"GetActiveOperationMode\">\n"

  "\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetOperationMode\">\n"

  "\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"IsOperationModeAnOverride\">\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"SetOperationMode\">\n"

  "\n"

  "      <arg type=\"s\" name=\"mode\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"SetOperationModeFor\">\n"

  "\n"

  "      <arg type=\"s\" name=\"mode\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"duration\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetUsageMode\">\n"

  "\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"SetUsageMode\">\n"

  "\n"

  "      <arg type=\"s\" name=\"mode\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"ReportActivity\">\n"

  "\n"

  "      <arg type=\"s\" name=\"who\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"b\" name=\"act\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "\n"

  "    <signal name=\"OperationModeChanged\">\n"

  "\n"

  "      <arg type=\"s\" name=\"value\" />\n"

  "\n"

  "    </signal>\n"

  "\n"

  "    <signal name=\"UsageModeChanged\">\n"

  "\n"

  "      <arg type=\"s\" name=\"value\" />\n"

  "\n"

  "    </signal>\n"

  "\n"

  "  </interface>\n";
  return xml;
}

bool
org_workrave_CoreInterface::dispatch(const QDBusMessage &message, const QDBusConnection &connection)
{
  using Method = void (org_workrave_CoreInterface::*)(const QDBusMessage &, const QDBusConnection &);
  struct Entry
  {
    std::string_view name;
    Method method;
  };
  static constexpr std::array<Entry, 11> methods =
  { {

      {.name = "ForceBreak", .method = &org_workrave_CoreInterface::dispatch_ForceBreak},

      {.name = "IsActive", .method = &org_workrave_CoreInterface::dispatch_IsActive},

      {.name = "IsTaking", .method = &org_workrave_CoreInterface::dispatch_IsTaking},

      {.name = "GetActiveOperationMode", .method = &org_workrave_CoreInterface::dispatch_GetActiveOperationMode},

      {.name = "GetOperationMode", .method = &org_workrave_CoreInterface::dispatch_GetOperationMode},

      {.name = "IsOperationModeAnOverride", .method = &org_workrave_CoreInterface::dispatch_IsOperationModeAnOverride},

      {.name = "SetOperationMode", .method = &org_workrave_CoreInterface::dispatch_SetOperationMode},

      {.name = "SetOperationModeFor", .method = &org_workrave_CoreInterface::dispatch_SetOperationModeFor},

      {.name = "GetUsageMode", .method = &org_workrave_CoreInterface::dispatch_GetUsageMode},

      {.name = "SetUsageMode", .method = &org_workrave_CoreInterface::dispatch_SetUsageMode},

      {.name = "ReportActivity", .method = &org_workrave_CoreInterface::dispatch_ReportActivity},

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
org_workrave_CoreInterface::dispatch_ForceBreak(const QDBusMessage &message, const QDBusConnection &connection)
{

  workrave::BreakId p_id{};

  workrave::utils::Flags<workrave::BreakHint> p_break_hint{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.ForceBreak");
    }



  p_id = ::workrave::rpc::dbus::QtCodec<workrave::BreakId>::decode(message.arguments().at(0));



  p_break_hint = ::workrave::rpc::dbus::QtCodec<workrave::utils::Flags<workrave::BreakHint>>::decode(message.arguments().at(1));




  implementation_.force_break(p_id, p_break_hint);


  QDBusMessage reply = message.createReply();







  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.CoreInterface.ForceBreak");
    }
}


void
org_workrave_CoreInterface::dispatch_IsActive(const QDBusMessage &message, const QDBusConnection &connection)
{


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.IsActive");
    }




  p_result = implementation_.is_user_active();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.CoreInterface.IsActive");
    }
}


void
org_workrave_CoreInterface::dispatch_IsTaking(const QDBusMessage &message, const QDBusConnection &connection)
{


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.IsTaking");
    }




  p_result = implementation_.is_taking();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.CoreInterface.IsTaking");
    }
}


void
org_workrave_CoreInterface::dispatch_GetActiveOperationMode(const QDBusMessage &message, const QDBusConnection &connection)
{


  workrave::OperationMode p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.GetActiveOperationMode");
    }




  p_result = implementation_.get_active_operation_mode();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<workrave::OperationMode>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.CoreInterface.GetActiveOperationMode");
    }
}


void
org_workrave_CoreInterface::dispatch_GetOperationMode(const QDBusMessage &message, const QDBusConnection &connection)
{


  workrave::OperationMode p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.GetOperationMode");
    }




  p_result = implementation_.get_regular_operation_mode();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<workrave::OperationMode>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.CoreInterface.GetOperationMode");
    }
}


void
org_workrave_CoreInterface::dispatch_IsOperationModeAnOverride(const QDBusMessage &message, const QDBusConnection &connection)
{


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.IsOperationModeAnOverride");
    }




  p_result = implementation_.is_operation_mode_an_override();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.CoreInterface.IsOperationModeAnOverride");
    }
}


void
org_workrave_CoreInterface::dispatch_SetOperationMode(const QDBusMessage &message, const QDBusConnection &connection)
{

  workrave::OperationMode p_mode{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.SetOperationMode");
    }



  p_mode = ::workrave::rpc::dbus::QtCodec<workrave::OperationMode>::decode(message.arguments().at(0));




  implementation_.set_operation_mode(p_mode);


  QDBusMessage reply = message.createReply();





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.CoreInterface.SetOperationMode");
    }
}


void
org_workrave_CoreInterface::dispatch_SetOperationModeFor(const QDBusMessage &message, const QDBusConnection &connection)
{

  workrave::OperationMode p_mode{};

  std::chrono::minutes p_duration{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.SetOperationModeFor");
    }



  p_mode = ::workrave::rpc::dbus::QtCodec<workrave::OperationMode>::decode(message.arguments().at(0));



  p_duration = ::workrave::rpc::dbus::QtCodec<std::chrono::minutes>::decode(message.arguments().at(1));




  implementation_.set_operation_mode_for(p_mode, p_duration);


  QDBusMessage reply = message.createReply();







  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.CoreInterface.SetOperationModeFor");
    }
}


void
org_workrave_CoreInterface::dispatch_GetUsageMode(const QDBusMessage &message, const QDBusConnection &connection)
{


  workrave::UsageMode p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.GetUsageMode");
    }




  p_result = implementation_.get_usage_mode();


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<workrave::UsageMode>::encode(p_result);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.CoreInterface.GetUsageMode");
    }
}


void
org_workrave_CoreInterface::dispatch_SetUsageMode(const QDBusMessage &message, const QDBusConnection &connection)
{

  workrave::UsageMode p_mode{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.SetUsageMode");
    }



  p_mode = ::workrave::rpc::dbus::QtCodec<workrave::UsageMode>::decode(message.arguments().at(0));




  implementation_.set_usage_mode(p_mode);


  QDBusMessage reply = message.createReply();





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.CoreInterface.SetUsageMode");
    }
}


void
org_workrave_CoreInterface::dispatch_ReportActivity(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_who{};

  bool p_act{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.ReportActivity");
    }



  p_who = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));



  p_act = ::workrave::rpc::dbus::QtCodec<bool>::decode(message.arguments().at(1));




  implementation_.report_external_activity(p_who, p_act);


  QDBusMessage reply = message.createReply();







  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.CoreInterface.ReportActivity");
    }
}


void
org_workrave_CoreInterface::emit_OperationModeChanged(workrave::OperationMode value)
{
  QVariantList arguments;

  arguments << ::workrave::rpc::dbus::QtCodec<workrave::OperationMode>::encode(value);

  server_.emit_signal(path_, "org.workrave.CoreInterface", "OperationModeChanged", arguments);
}
void
org_workrave_CoreInterface::emit_UsageModeChanged(workrave::UsageMode value)
{
  QVariantList arguments;

  arguments << ::workrave::rpc::dbus::QtCodec<workrave::UsageMode>::encode(value);

  server_.emit_signal(path_, "org.workrave.CoreInterface", "UsageModeChanged", arguments);
}
} // namespace workrave::core::rpc
