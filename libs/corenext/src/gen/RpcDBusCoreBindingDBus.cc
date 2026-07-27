// GENERATED FILE - DO NOT EDIT.
// Produced by clang-rpc-gen's DBus backend from an annotated C++ header.
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

#include "dbus/DBusBindingQt.hh"
#include "dbus/DBusException.hh"

#include "rpc/Duration.hh"

#include "RpcDBusCoreBindingDBus.hh"

using namespace workrave::dbus;

template<typename To, typename From>
To dbus_checked_integer_cast(From value)
{
  static_assert(std::is_integral_v<To> && std::is_integral_v<From>);
  if (!std::in_range<To>(value))
    {
      throw ::workrave::dbus::DBusRemoteException()
        << ::workrave::dbus::message_info("DBus integer conversion is out of range")
        << ::workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
    }
  return static_cast<To>(value);
}


namespace workrave::dbus
{
template<>
struct DBusMarshall<workrave::BreakId>
{
  static workrave::BreakId convert(const QVariant &variant)
  {
    QString arg = variant.value<QString>();
    workrave::BreakId value{};

    if ("none" == arg) { value = workrave::BreakId::BREAK_ID_NONE; }

    if ("microbreak" == arg) { value = workrave::BreakId::BREAK_ID_MICRO_BREAK; }

    if ("restbreak" == arg) { value = workrave::BreakId::BREAK_ID_REST_BREAK; }

    if ("dailylimit" == arg) { value = workrave::BreakId::BREAK_ID_DAILY_LIMIT; }

    return value;
  }
  static void marshall(QDBusArgument &arg, const workrave::BreakId &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::BreakId::BREAK_ID_NONE: str = "none"; break;

      case workrave::BreakId::BREAK_ID_MICRO_BREAK: str = "microbreak"; break;

      case workrave::BreakId::BREAK_ID_REST_BREAK: str = "restbreak"; break;

      case workrave::BreakId::BREAK_ID_DAILY_LIMIT: str = "dailylimit"; break;

      default:
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }
    arg << QString::fromStdString(str);
  }
  static QVariant convert(const workrave::BreakId &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::BreakId::BREAK_ID_NONE: str = "none"; break;

      case workrave::BreakId::BREAK_ID_MICRO_BREAK: str = "microbreak"; break;

      case workrave::BreakId::BREAK_ID_REST_BREAK: str = "restbreak"; break;

      case workrave::BreakId::BREAK_ID_DAILY_LIMIT: str = "dailylimit"; break;

      default:
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }
    return QVariant::fromValue(QString::fromStdString(str));
  }
};
} // namespace workrave::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::dbus, where they'd only be found via workrave::BreakId's
// own namespace (which may not be workrave::dbus at all), silently breaks
// qDBusRegisterMetaType<workrave::BreakId>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const workrave::BreakId &data)
{
  workrave::dbus::DBusMarshall<workrave::BreakId>::marshall(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, workrave::BreakId &data)
{
  QString value;
  arg >> value;
  data = workrave::dbus::DBusMarshall<workrave::BreakId>::convert(QVariant::fromValue(value));
  return arg;
}

namespace workrave::dbus
{
template<>
struct DBusMarshall<workrave::BreakHint>
{
  static workrave::BreakHint convert(const QVariant &variant)
  {
    QString arg = variant.value<QString>();
    workrave::BreakHint value{};

    if ("normal" == arg) { value = workrave::BreakHint::Normal; }

    if ("userinitiated" == arg) { value = workrave::BreakHint::UserInitiated; }

    if ("naturalbreak" == arg) { value = workrave::BreakHint::NaturalBreak; }

    return value;
  }
  static void marshall(QDBusArgument &arg, const workrave::BreakHint &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::BreakHint::Normal: str = "normal"; break;

      case workrave::BreakHint::UserInitiated: str = "userinitiated"; break;

      case workrave::BreakHint::NaturalBreak: str = "naturalbreak"; break;

      default:
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }
    arg << QString::fromStdString(str);
  }
  static QVariant convert(const workrave::BreakHint &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::BreakHint::Normal: str = "normal"; break;

      case workrave::BreakHint::UserInitiated: str = "userinitiated"; break;

      case workrave::BreakHint::NaturalBreak: str = "naturalbreak"; break;

      default:
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }
    return QVariant::fromValue(QString::fromStdString(str));
  }
};
} // namespace workrave::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::dbus, where they'd only be found via workrave::BreakHint's
// own namespace (which may not be workrave::dbus at all), silently breaks
// qDBusRegisterMetaType<workrave::BreakHint>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const workrave::BreakHint &data)
{
  workrave::dbus::DBusMarshall<workrave::BreakHint>::marshall(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, workrave::BreakHint &data)
{
  QString value;
  arg >> value;
  data = workrave::dbus::DBusMarshall<workrave::BreakHint>::convert(QVariant::fromValue(value));
  return arg;
}

namespace workrave::dbus
{
template<>
struct DBusMarshall<workrave::utils::Flags<workrave::BreakHint>>
{
  static workrave::utils::Flags<workrave::BreakHint> convert(const QVariant &variant)
  {
    std::list<workrave::BreakHint> items = DBusMarshall<std::list<workrave::BreakHint>>::convert(variant);
    workrave::utils::Flags<workrave::BreakHint> result;
    for (const auto &item : items)
      {
        result |= item;
      }
    return result;
  }
  static void marshall(QDBusArgument &arg, const workrave::utils::Flags<workrave::BreakHint> &value)
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
    DBusMarshall<std::list<workrave::BreakHint>>::marshall(arg, items);
  }
  static QVariant convert(const workrave::utils::Flags<workrave::BreakHint> &value)
  {
    QDBusArgument arg;
    marshall(arg, value);
    return QVariant::fromValue(arg);
  }
};
} // namespace workrave::dbus

namespace workrave::dbus
{
template<>
struct DBusMarshall<workrave::OperationMode>
{
  static workrave::OperationMode convert(const QVariant &variant)
  {
    QString arg = variant.value<QString>();
    workrave::OperationMode value{};

    if ("normal" == arg) { value = workrave::OperationMode::Normal; }

    if ("suspended" == arg) { value = workrave::OperationMode::Suspended; }

    if ("quiet" == arg) { value = workrave::OperationMode::Quiet; }

    return value;
  }
  static void marshall(QDBusArgument &arg, const workrave::OperationMode &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::OperationMode::Normal: str = "normal"; break;

      case workrave::OperationMode::Suspended: str = "suspended"; break;

      case workrave::OperationMode::Quiet: str = "quiet"; break;

      default:
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }
    arg << QString::fromStdString(str);
  }
  static QVariant convert(const workrave::OperationMode &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::OperationMode::Normal: str = "normal"; break;

      case workrave::OperationMode::Suspended: str = "suspended"; break;

      case workrave::OperationMode::Quiet: str = "quiet"; break;

      default:
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }
    return QVariant::fromValue(QString::fromStdString(str));
  }
};
} // namespace workrave::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::dbus, where they'd only be found via workrave::OperationMode's
// own namespace (which may not be workrave::dbus at all), silently breaks
// qDBusRegisterMetaType<workrave::OperationMode>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const workrave::OperationMode &data)
{
  workrave::dbus::DBusMarshall<workrave::OperationMode>::marshall(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, workrave::OperationMode &data)
{
  QString value;
  arg >> value;
  data = workrave::dbus::DBusMarshall<workrave::OperationMode>::convert(QVariant::fromValue(value));
  return arg;
}

namespace workrave::dbus
{
template<>
struct DBusMarshall<std::chrono::minutes>
{
  static std::chrono::minutes convert(const QVariant &variant)
  {
    std::string text = DBusMarshall<std::string>::convert(variant);
    return std::chrono::duration_cast<std::chrono::minutes>(rpc::parse_duration(text));
  }
};
} // namespace workrave::dbus

namespace workrave::dbus
{
template<>
struct DBusMarshall<workrave::UsageMode>
{
  static workrave::UsageMode convert(const QVariant &variant)
  {
    QString arg = variant.value<QString>();
    workrave::UsageMode value{};

    if ("normal" == arg) { value = workrave::UsageMode::Normal; }

    if ("reading" == arg) { value = workrave::UsageMode::Reading; }

    return value;
  }
  static void marshall(QDBusArgument &arg, const workrave::UsageMode &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::UsageMode::Normal: str = "normal"; break;

      case workrave::UsageMode::Reading: str = "reading"; break;

      default:
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }
    arg << QString::fromStdString(str);
  }
  static QVariant convert(const workrave::UsageMode &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::UsageMode::Normal: str = "normal"; break;

      case workrave::UsageMode::Reading: str = "reading"; break;

      default:
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }
    return QVariant::fromValue(QString::fromStdString(str));
  }
};
} // namespace workrave::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::dbus, where they'd only be found via workrave::UsageMode's
// own namespace (which may not be workrave::dbus at all), silently breaks
// qDBusRegisterMetaType<workrave::UsageMode>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const workrave::UsageMode &data)
{
  workrave::dbus::DBusMarshall<workrave::UsageMode>::marshall(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, workrave::UsageMode &data)
{
  QString value;
  arg >> value;
  data = workrave::dbus::DBusMarshall<workrave::UsageMode>::convert(QVariant::fromValue(value));
  return arg;
}


namespace workrave::core::rpc
{

class org_workrave_CoreInterface_Stub : public ::workrave::dbus::DBusBindingQt, public org_workrave_CoreInterface
{
private:
  using DBusMethodPointer = void (org_workrave_CoreInterface_Stub::*)(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  struct DBusMethod
  {
    std::string_view name;
    DBusMethodPointer fn;
  };

  bool call(void *object, const QDBusMessage &message, const QDBusConnection &connection) override;

  std::string_view get_interface_introspect() override
  {
    return interface_introspect;
  }

public:
  explicit org_workrave_CoreInterface_Stub(std::shared_ptr<::workrave::dbus::IDBus> dbus) : ::workrave::dbus::DBusBindingQt(std::move(dbus)) {}
  ~org_workrave_CoreInterface_Stub() override = default;


  void OperationModeChanged(const std::string &path, workrave::OperationMode value) override;

  void UsageModeChanged(const std::string &path, workrave::UsageMode value) override;


private:

  void ForceBreak(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void IsActive(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void IsTaking(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetActiveOperationMode(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetOperationMode(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void IsOperationModeAnOverride(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void SetOperationMode(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void SetOperationModeFor(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetUsageMode(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void SetUsageMode(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void ReportActivity(void *object, const QDBusMessage &message, const QDBusConnection &connection);


  static constexpr std::array<DBusMethod, 11> method_table =
  { {

      {.name = "ForceBreak", .fn = &org_workrave_CoreInterface_Stub::ForceBreak},

      {.name = "IsActive", .fn = &org_workrave_CoreInterface_Stub::IsActive},

      {.name = "IsTaking", .fn = &org_workrave_CoreInterface_Stub::IsTaking},

      {.name = "GetActiveOperationMode", .fn = &org_workrave_CoreInterface_Stub::GetActiveOperationMode},

      {.name = "GetOperationMode", .fn = &org_workrave_CoreInterface_Stub::GetOperationMode},

      {.name = "IsOperationModeAnOverride", .fn = &org_workrave_CoreInterface_Stub::IsOperationModeAnOverride},

      {.name = "SetOperationMode", .fn = &org_workrave_CoreInterface_Stub::SetOperationMode},

      {.name = "SetOperationModeFor", .fn = &org_workrave_CoreInterface_Stub::SetOperationModeFor},

      {.name = "GetUsageMode", .fn = &org_workrave_CoreInterface_Stub::GetUsageMode},

      {.name = "SetUsageMode", .fn = &org_workrave_CoreInterface_Stub::SetUsageMode},

      {.name = "ReportActivity", .fn = &org_workrave_CoreInterface_Stub::ReportActivity},

  } };

  static constexpr std::string_view interface_introspect =

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

  "  </interface>\n"

;
};

org_workrave_CoreInterface *org_workrave_CoreInterface::instance(std::shared_ptr<::workrave::dbus::IDBus> dbus)
{
  org_workrave_CoreInterface_Stub *iface = nullptr;
  ::workrave::dbus::DBusBinding *binding = dbus->find_binding("org.workrave.CoreInterface");
  if (binding != nullptr)
    {
      iface = dynamic_cast<org_workrave_CoreInterface_Stub *>(binding);
    }
  return iface;
}

bool
org_workrave_CoreInterface_Stub::call(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  std::string method_name = message.member().toStdString();
  constexpr std::array<DBusMethod, 11> table = method_table;
  for (const auto &method : table)
    {
      if (method_name == method.name)
        {
          DBusMethodPointer ptr = method.fn;
          if (ptr != nullptr)
            {
              (this->*ptr)(object, message, connection);
            }
          return true;
        }
    }
  throw ::workrave::dbus::DBusRemoteException()
    << ::workrave::dbus::message_info("Unknown method")
    << ::workrave::dbus::error_code_info(DBUS_ERROR_UNKNOWN_METHOD)
    << ::workrave::dbus::method_info(method_name)
    << ::workrave::dbus::interface_info("org.workrave.CoreInterface");
}


void
org_workrave_CoreInterface_Stub::ForceBreak(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Core *>(object);


      workrave::BreakId p_id{};

      workrave::utils::Flags<workrave::BreakHint> p_break_hint{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 2)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("ForceBreak")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }



      p_id = ::workrave::dbus::DBusMarshall<workrave::BreakId>::convert(message.arguments().at(0));



      p_break_hint = ::workrave::dbus::DBusMarshall<workrave::utils::Flags<workrave::BreakHint>>::convert(message.arguments().at(1));




      dbus_object->force_break(p_id, p_break_hint);


      QDBusMessage reply = message.createReply();







      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("ForceBreak")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("ForceBreak") << interface_info("org.workrave.CoreInterface");
      throw;
    }
}


void
org_workrave_CoreInterface_Stub::IsActive(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Core *>(object);



      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("IsActive")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }




      p_result = dbus_object->is_user_active();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("IsActive")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("IsActive") << interface_info("org.workrave.CoreInterface");
      throw;
    }
}


void
org_workrave_CoreInterface_Stub::IsTaking(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Core *>(object);



      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("IsTaking")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }




      p_result = dbus_object->is_taking();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("IsTaking")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("IsTaking") << interface_info("org.workrave.CoreInterface");
      throw;
    }
}


void
org_workrave_CoreInterface_Stub::GetActiveOperationMode(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Core *>(object);



      workrave::OperationMode p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetActiveOperationMode")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }




      p_result = dbus_object->get_active_operation_mode();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<workrave::OperationMode>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetActiveOperationMode")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetActiveOperationMode") << interface_info("org.workrave.CoreInterface");
      throw;
    }
}


void
org_workrave_CoreInterface_Stub::GetOperationMode(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Core *>(object);



      workrave::OperationMode p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetOperationMode")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }




      p_result = dbus_object->get_regular_operation_mode();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<workrave::OperationMode>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetOperationMode")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetOperationMode") << interface_info("org.workrave.CoreInterface");
      throw;
    }
}


void
org_workrave_CoreInterface_Stub::IsOperationModeAnOverride(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Core *>(object);



      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("IsOperationModeAnOverride")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }




      p_result = dbus_object->is_operation_mode_an_override();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("IsOperationModeAnOverride")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("IsOperationModeAnOverride") << interface_info("org.workrave.CoreInterface");
      throw;
    }
}


void
org_workrave_CoreInterface_Stub::SetOperationMode(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Core *>(object);


      workrave::OperationMode p_mode{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 1)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("SetOperationMode")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }



      p_mode = ::workrave::dbus::DBusMarshall<workrave::OperationMode>::convert(message.arguments().at(0));




      dbus_object->set_operation_mode(p_mode);


      QDBusMessage reply = message.createReply();





      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("SetOperationMode")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("SetOperationMode") << interface_info("org.workrave.CoreInterface");
      throw;
    }
}


void
org_workrave_CoreInterface_Stub::SetOperationModeFor(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Core *>(object);


      workrave::OperationMode p_mode{};

      std::chrono::minutes p_duration{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 2)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("SetOperationModeFor")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }



      p_mode = ::workrave::dbus::DBusMarshall<workrave::OperationMode>::convert(message.arguments().at(0));



      p_duration = ::workrave::dbus::DBusMarshall<std::chrono::minutes>::convert(message.arguments().at(1));




      dbus_object->set_operation_mode_for(p_mode, p_duration);


      QDBusMessage reply = message.createReply();







      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("SetOperationModeFor")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("SetOperationModeFor") << interface_info("org.workrave.CoreInterface");
      throw;
    }
}


void
org_workrave_CoreInterface_Stub::GetUsageMode(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Core *>(object);



      workrave::UsageMode p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetUsageMode")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }




      p_result = dbus_object->get_usage_mode();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<workrave::UsageMode>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetUsageMode")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetUsageMode") << interface_info("org.workrave.CoreInterface");
      throw;
    }
}


void
org_workrave_CoreInterface_Stub::SetUsageMode(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Core *>(object);


      workrave::UsageMode p_mode{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 1)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("SetUsageMode")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }



      p_mode = ::workrave::dbus::DBusMarshall<workrave::UsageMode>::convert(message.arguments().at(0));




      dbus_object->set_usage_mode(p_mode);


      QDBusMessage reply = message.createReply();





      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("SetUsageMode")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("SetUsageMode") << interface_info("org.workrave.CoreInterface");
      throw;
    }
}


void
org_workrave_CoreInterface_Stub::ReportActivity(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Core *>(object);


      std::string p_who{};

      bool p_act{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 2)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("ReportActivity")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }



      p_who = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));



      p_act = ::workrave::dbus::DBusMarshall<bool>::convert(message.arguments().at(1));




      dbus_object->report_external_activity(p_who, p_act);


      QDBusMessage reply = message.createReply();







      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("ReportActivity")
            << workrave::dbus::interface_info("org.workrave.CoreInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("ReportActivity") << interface_info("org.workrave.CoreInterface");
      throw;
    }
}



void
org_workrave_CoreInterface_Stub::OperationModeChanged(const std::string &path, workrave::OperationMode value)
{
  QDBusMessage sig = QDBusMessage::createSignal(QString::fromStdString(path), "org.workrave.CoreInterface", "OperationModeChanged");

  sig << ::workrave::dbus::DBusMarshall<workrave::OperationMode>::convert(value);

  ::workrave::dbus::IDBusPrivateQt::Ptr priv = std::dynamic_pointer_cast<::workrave::dbus::IDBusPrivateQt>(dbus);
  priv->get_connection().send(sig);
}


void
org_workrave_CoreInterface_Stub::UsageModeChanged(const std::string &path, workrave::UsageMode value)
{
  QDBusMessage sig = QDBusMessage::createSignal(QString::fromStdString(path), "org.workrave.CoreInterface", "UsageModeChanged");

  sig << ::workrave::dbus::DBusMarshall<workrave::UsageMode>::convert(value);

  ::workrave::dbus::IDBusPrivateQt::Ptr priv = std::dynamic_pointer_cast<::workrave::dbus::IDBusPrivateQt>(dbus);
  priv->get_connection().send(sig);
}


void init_org_workrave_CoreInterface(std::shared_ptr<::workrave::dbus::IDBus> dbus)
{
  dbus->register_binding("org.workrave.CoreInterface", new org_workrave_CoreInterface_Stub(dbus));


  qDBusRegisterMetaType<workrave::BreakId>();

  qDBusRegisterMetaType<workrave::BreakHint>();

  qDBusRegisterMetaType<workrave::OperationMode>();

  qDBusRegisterMetaType<workrave::UsageMode>();

}

} // namespace workrave::core::rpc
