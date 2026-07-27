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

#include "RpcDBusBreakBindingDBus.hh"

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
struct DBusMarshall<workrave::BreakEvent>
{
  static workrave::BreakEvent convert(const QVariant &variant)
  {
    QString arg = variant.value<QString>();
    workrave::BreakEvent value{};

    if ("show_prelude" == arg) { value = workrave::BreakEvent::ShowPrelude; }

    if ("show_break" == arg) { value = workrave::BreakEvent::ShowBreak; }

    if ("show_break_forced" == arg) { value = workrave::BreakEvent::ShowBreakForced; }

    if ("break_start" == arg) { value = workrave::BreakEvent::BreakStart; }

    if ("break_idle" == arg) { value = workrave::BreakEvent::BreakIdle; }

    if ("break_stop" == arg) { value = workrave::BreakEvent::BreakStop; }

    if ("break_ignored" == arg) { value = workrave::BreakEvent::BreakIgnored; }

    if ("break_postponed" == arg) { value = workrave::BreakEvent::BreakPostponed; }

    if ("break_skipped" == arg) { value = workrave::BreakEvent::BreakSkipped; }

    if ("break_taken" == arg) { value = workrave::BreakEvent::BreakTaken; }

    return value;
  }
  static void marshall(QDBusArgument &arg, const workrave::BreakEvent &value)
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
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }
    arg << QString::fromStdString(str);
  }
  static QVariant convert(const workrave::BreakEvent &value)
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
// them inside workrave::dbus, where they'd only be found via workrave::BreakEvent's
// own namespace (which may not be workrave::dbus at all), silently breaks
// qDBusRegisterMetaType<workrave::BreakEvent>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const workrave::BreakEvent &data)
{
  workrave::dbus::DBusMarshall<workrave::BreakEvent>::marshall(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, workrave::BreakEvent &data)
{
  QString value;
  arg >> value;
  data = workrave::dbus::DBusMarshall<workrave::BreakEvent>::convert(QVariant::fromValue(value));
  return arg;
}

namespace workrave::dbus
{
template<>
struct DBusMarshall<BreakStage>
{
  static BreakStage convert(const QVariant &variant)
  {
    QString arg = variant.value<QString>();
    BreakStage value{};

    if ("none" == arg) { value = BreakStage::None; }

    if ("snoozed" == arg) { value = BreakStage::Snoozed; }

    if ("prelude" == arg) { value = BreakStage::Prelude; }

    if ("taking" == arg) { value = BreakStage::Taking; }

    if ("delayed" == arg) { value = BreakStage::Delayed; }

    return value;
  }
  static void marshall(QDBusArgument &arg, const BreakStage &value)
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
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }
    arg << QString::fromStdString(str);
  }
  static QVariant convert(const BreakStage &value)
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
// them inside workrave::dbus, where they'd only be found via BreakStage's
// own namespace (which may not be workrave::dbus at all), silently breaks
// qDBusRegisterMetaType<BreakStage>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const BreakStage &data)
{
  workrave::dbus::DBusMarshall<BreakStage>::marshall(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, BreakStage &data)
{
  QString value;
  arg >> value;
  data = workrave::dbus::DBusMarshall<BreakStage>::convert(QVariant::fromValue(value));
  return arg;
}


namespace workrave::core::rpc
{

class org_workrave_BreakInterface_Stub : public ::workrave::dbus::DBusBindingQt, public org_workrave_BreakInterface
{
private:
  using DBusMethodPointer = void (org_workrave_BreakInterface_Stub::*)(void *object, const QDBusMessage &message, const QDBusConnection &connection);

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
  explicit org_workrave_BreakInterface_Stub(std::shared_ptr<::workrave::dbus::IDBus> dbus) : ::workrave::dbus::DBusBindingQt(std::move(dbus)) {}
  ~org_workrave_BreakInterface_Stub() override = default;


  void BreakEvent(const std::string &path, workrave::BreakEvent value) override;

  void BreakStateChanged(const std::string &path, BreakStage value) override;


private:

  void GetName(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void IsEnabled(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void IsTimerRunning(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void IsTaking(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void IsMaxPreludesReached(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void IsActive(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetTimerElapsed(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetTimerIdle(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetAutoReset(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void IsAutoResetEnabled(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetLimit(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void IsLimitEnabled(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetTimerRemaining(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetTimerOverdue(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void PostponeBreak(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void SkipBreak(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetBreakState(void *object, const QDBusMessage &message, const QDBusConnection &connection);


  static constexpr std::array<DBusMethod, 17> method_table =
  { {

      {.name = "GetName", .fn = &org_workrave_BreakInterface_Stub::GetName},

      {.name = "IsEnabled", .fn = &org_workrave_BreakInterface_Stub::IsEnabled},

      {.name = "IsTimerRunning", .fn = &org_workrave_BreakInterface_Stub::IsTimerRunning},

      {.name = "IsTaking", .fn = &org_workrave_BreakInterface_Stub::IsTaking},

      {.name = "IsMaxPreludesReached", .fn = &org_workrave_BreakInterface_Stub::IsMaxPreludesReached},

      {.name = "IsActive", .fn = &org_workrave_BreakInterface_Stub::IsActive},

      {.name = "GetTimerElapsed", .fn = &org_workrave_BreakInterface_Stub::GetTimerElapsed},

      {.name = "GetTimerIdle", .fn = &org_workrave_BreakInterface_Stub::GetTimerIdle},

      {.name = "GetAutoReset", .fn = &org_workrave_BreakInterface_Stub::GetAutoReset},

      {.name = "IsAutoResetEnabled", .fn = &org_workrave_BreakInterface_Stub::IsAutoResetEnabled},

      {.name = "GetLimit", .fn = &org_workrave_BreakInterface_Stub::GetLimit},

      {.name = "IsLimitEnabled", .fn = &org_workrave_BreakInterface_Stub::IsLimitEnabled},

      {.name = "GetTimerRemaining", .fn = &org_workrave_BreakInterface_Stub::GetTimerRemaining},

      {.name = "GetTimerOverdue", .fn = &org_workrave_BreakInterface_Stub::GetTimerOverdue},

      {.name = "PostponeBreak", .fn = &org_workrave_BreakInterface_Stub::PostponeBreak},

      {.name = "SkipBreak", .fn = &org_workrave_BreakInterface_Stub::SkipBreak},

      {.name = "GetBreakState", .fn = &org_workrave_BreakInterface_Stub::GetBreakState},

  } };

  static constexpr std::string_view interface_introspect =

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

  "  </interface>\n"

;
};

org_workrave_BreakInterface *org_workrave_BreakInterface::instance(std::shared_ptr<::workrave::dbus::IDBus> dbus)
{
  org_workrave_BreakInterface_Stub *iface = nullptr;
  ::workrave::dbus::DBusBinding *binding = dbus->find_binding("org.workrave.BreakInterface");
  if (binding != nullptr)
    {
      iface = dynamic_cast<org_workrave_BreakInterface_Stub *>(binding);
    }
  return iface;
}

bool
org_workrave_BreakInterface_Stub::call(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  std::string method_name = message.member().toStdString();
  constexpr std::array<DBusMethod, 17> table = method_table;
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
    << ::workrave::dbus::interface_info("org.workrave.BreakInterface");
}


void
org_workrave_BreakInterface_Stub::GetName(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      std::string p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetName")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->get_name();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<std::string>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetName")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetName") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::IsEnabled(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("IsEnabled")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->is_enabled();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("IsEnabled")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("IsEnabled") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::IsTimerRunning(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("IsTimerRunning")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->is_running();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("IsTimerRunning")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("IsTimerRunning") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::IsTaking(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("IsTaking")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
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
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("IsTaking") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::IsMaxPreludesReached(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("IsMaxPreludesReached")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->is_max_preludes_reached();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("IsMaxPreludesReached")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("IsMaxPreludesReached") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::IsActive(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("IsActive")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->is_active();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("IsActive")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("IsActive") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::GetTimerElapsed(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      int64_t p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetTimerElapsed")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->get_elapsed_time();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<int32_t>::convert(static_cast<int32_t>(p_result));



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetTimerElapsed")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetTimerElapsed") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::GetTimerIdle(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      int64_t p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetTimerIdle")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->get_elapsed_idle_time();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<int32_t>::convert(static_cast<int32_t>(p_result));



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetTimerIdle")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetTimerIdle") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::GetAutoReset(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      int64_t p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetAutoReset")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->get_auto_reset();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<int64_t>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetAutoReset")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetAutoReset") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::IsAutoResetEnabled(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("IsAutoResetEnabled")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->is_auto_reset_enabled();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("IsAutoResetEnabled")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("IsAutoResetEnabled") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::GetLimit(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      int64_t p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetLimit")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->get_limit();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<int64_t>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetLimit")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetLimit") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::IsLimitEnabled(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("IsLimitEnabled")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->is_limit_enabled();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("IsLimitEnabled")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("IsLimitEnabled") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::GetTimerRemaining(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      int64_t p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetTimerRemaining")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->get_timer_remaining();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<int32_t>::convert(static_cast<int32_t>(p_result));



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetTimerRemaining")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetTimerRemaining") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::GetTimerOverdue(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      int64_t p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetTimerOverdue")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->get_total_overdue_time();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<int32_t>::convert(static_cast<int32_t>(p_result));



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetTimerOverdue")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetTimerOverdue") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::PostponeBreak(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);




      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("PostponeBreak")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      dbus_object->postpone_break();


      QDBusMessage reply = message.createReply();



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("PostponeBreak")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("PostponeBreak") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::SkipBreak(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);




      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("SkipBreak")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      dbus_object->skip_break();


      QDBusMessage reply = message.createReply();



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("SkipBreak")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("SkipBreak") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}


void
org_workrave_BreakInterface_Stub::GetBreakState(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<Break *>(object);



      std::string p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 0)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetBreakState")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }




      p_result = dbus_object->get_break_stage();


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<std::string>::convert(p_result);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetBreakState")
            << workrave::dbus::interface_info("org.workrave.BreakInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetBreakState") << interface_info("org.workrave.BreakInterface");
      throw;
    }
}



void
org_workrave_BreakInterface_Stub::BreakEvent(const std::string &path, workrave::BreakEvent value)
{
  QDBusMessage sig = QDBusMessage::createSignal(QString::fromStdString(path), "org.workrave.BreakInterface", "BreakEvent");

  sig << ::workrave::dbus::DBusMarshall<workrave::BreakEvent>::convert(value);

  ::workrave::dbus::IDBusPrivateQt::Ptr priv = std::dynamic_pointer_cast<::workrave::dbus::IDBusPrivateQt>(dbus);
  priv->get_connection().send(sig);
}


void
org_workrave_BreakInterface_Stub::BreakStateChanged(const std::string &path, BreakStage value)
{
  QDBusMessage sig = QDBusMessage::createSignal(QString::fromStdString(path), "org.workrave.BreakInterface", "BreakStateChanged");

  sig << ::workrave::dbus::DBusMarshall<BreakStage>::convert(value);

  ::workrave::dbus::IDBusPrivateQt::Ptr priv = std::dynamic_pointer_cast<::workrave::dbus::IDBusPrivateQt>(dbus);
  priv->get_connection().send(sig);
}


void init_org_workrave_BreakInterface(std::shared_ptr<::workrave::dbus::IDBus> dbus)
{
  dbus->register_binding("org.workrave.BreakInterface", new org_workrave_BreakInterface_Stub(dbus));


  qDBusRegisterMetaType<workrave::BreakEvent>();

  qDBusRegisterMetaType<BreakStage>();

}

} // namespace workrave::core::rpc
