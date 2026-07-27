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

#include "RpcDBusConfigBindingDBus.hh"

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
struct DBusMarshall<workrave::config::ConfigFlags>
{
  static workrave::config::ConfigFlags convert(const QVariant &variant)
  {
    QString arg = variant.value<QString>();
    workrave::config::ConfigFlags value{};

    if ("none" == arg) { value = workrave::config::ConfigFlags::CONFIG_FLAG_NONE; }

    if ("initial" == arg) { value = workrave::config::ConfigFlags::CONFIG_FLAG_INITIAL; }

    if ("immediate" == arg) { value = workrave::config::ConfigFlags::CONFIG_FLAG_IMMEDIATE; }

    return value;
  }
  static void marshall(QDBusArgument &arg, const workrave::config::ConfigFlags &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::config::ConfigFlags::CONFIG_FLAG_NONE: str = "none"; break;

      case workrave::config::ConfigFlags::CONFIG_FLAG_INITIAL: str = "initial"; break;

      case workrave::config::ConfigFlags::CONFIG_FLAG_IMMEDIATE: str = "immediate"; break;

      default:
        throw workrave::dbus::DBusRemoteException()
          << workrave::dbus::message_info("Type error in enum")
          << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS);
      }
    arg << QString::fromStdString(str);
  }
  static QVariant convert(const workrave::config::ConfigFlags &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::config::ConfigFlags::CONFIG_FLAG_NONE: str = "none"; break;

      case workrave::config::ConfigFlags::CONFIG_FLAG_INITIAL: str = "initial"; break;

      case workrave::config::ConfigFlags::CONFIG_FLAG_IMMEDIATE: str = "immediate"; break;

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
// them inside workrave::dbus, where they'd only be found via workrave::config::ConfigFlags's
// own namespace (which may not be workrave::dbus at all), silently breaks
// qDBusRegisterMetaType<workrave::config::ConfigFlags>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const workrave::config::ConfigFlags &data)
{
  workrave::dbus::DBusMarshall<workrave::config::ConfigFlags>::marshall(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, workrave::config::ConfigFlags &data)
{
  QString value;
  arg >> value;
  data = workrave::dbus::DBusMarshall<workrave::config::ConfigFlags>::convert(QVariant::fromValue(value));
  return arg;
}


namespace workrave::core::rpc
{

class org_workrave_ConfigInterface_Stub : public ::workrave::dbus::DBusBindingQt, public org_workrave_ConfigInterface
{
private:
  using DBusMethodPointer = void (org_workrave_ConfigInterface_Stub::*)(void *object, const QDBusMessage &message, const QDBusConnection &connection);

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
  explicit org_workrave_ConfigInterface_Stub(std::shared_ptr<::workrave::dbus::IDBus> dbus) : ::workrave::dbus::DBusBindingQt(std::move(dbus)) {}
  ~org_workrave_ConfigInterface_Stub() override = default;



private:

  void RemoveKey(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void RenameKey(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void HasUserValue(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetString(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetBool(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetInt(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetInt64(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetDouble(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetStringWithDefault(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetBoolWithDefault(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetIntWithDefault(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetInt64WithDefault(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void GetDoubleWithDefault(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void SetString(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void SetInt(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void SetInt64(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void SetBool(void *object, const QDBusMessage &message, const QDBusConnection &connection);

  void SetDouble(void *object, const QDBusMessage &message, const QDBusConnection &connection);


  static constexpr std::array<DBusMethod, 18> method_table =
  { {

      {.name = "RemoveKey", .fn = &org_workrave_ConfigInterface_Stub::RemoveKey},

      {.name = "RenameKey", .fn = &org_workrave_ConfigInterface_Stub::RenameKey},

      {.name = "HasUserValue", .fn = &org_workrave_ConfigInterface_Stub::HasUserValue},

      {.name = "GetString", .fn = &org_workrave_ConfigInterface_Stub::GetString},

      {.name = "GetBool", .fn = &org_workrave_ConfigInterface_Stub::GetBool},

      {.name = "GetInt", .fn = &org_workrave_ConfigInterface_Stub::GetInt},

      {.name = "GetInt64", .fn = &org_workrave_ConfigInterface_Stub::GetInt64},

      {.name = "GetDouble", .fn = &org_workrave_ConfigInterface_Stub::GetDouble},

      {.name = "GetStringWithDefault", .fn = &org_workrave_ConfigInterface_Stub::GetStringWithDefault},

      {.name = "GetBoolWithDefault", .fn = &org_workrave_ConfigInterface_Stub::GetBoolWithDefault},

      {.name = "GetIntWithDefault", .fn = &org_workrave_ConfigInterface_Stub::GetIntWithDefault},

      {.name = "GetInt64WithDefault", .fn = &org_workrave_ConfigInterface_Stub::GetInt64WithDefault},

      {.name = "GetDoubleWithDefault", .fn = &org_workrave_ConfigInterface_Stub::GetDoubleWithDefault},

      {.name = "SetString", .fn = &org_workrave_ConfigInterface_Stub::SetString},

      {.name = "SetInt", .fn = &org_workrave_ConfigInterface_Stub::SetInt},

      {.name = "SetInt64", .fn = &org_workrave_ConfigInterface_Stub::SetInt64},

      {.name = "SetBool", .fn = &org_workrave_ConfigInterface_Stub::SetBool},

      {.name = "SetDouble", .fn = &org_workrave_ConfigInterface_Stub::SetDouble},

  } };

  static constexpr std::string_view interface_introspect =

  "  <interface name=\"org.workrave.ConfigInterface\">\n"

  "\n"

  "    <method name=\"RemoveKey\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"RenameKey\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"new_key\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"HasUserValue\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetString\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"out\" direction=\"out\" />\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetBool\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"b\" name=\"out\" direction=\"out\" />\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetInt\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"i\" name=\"out\" direction=\"out\" />\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetInt64\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"x\" name=\"out\" direction=\"out\" />\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetDouble\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"d\" name=\"out\" direction=\"out\" />\n"

  "\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetStringWithDefault\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"out\" direction=\"out\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"s\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetBoolWithDefault\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"b\" name=\"out\" direction=\"out\" />\n"

  "\n"

  "      <arg type=\"b\" name=\"def\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetIntWithDefault\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"i\" name=\"out\" direction=\"out\" />\n"

  "\n"

  "      <arg type=\"i\" name=\"def\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetInt64WithDefault\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"x\" name=\"out\" direction=\"out\" />\n"

  "\n"

  "      <arg type=\"x\" name=\"def\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetDoubleWithDefault\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"d\" name=\"out\" direction=\"out\" />\n"

  "\n"

  "      <arg type=\"d\" name=\"def\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"SetString\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"v\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"flags\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"SetInt\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"i\" name=\"v\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"flags\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"SetInt64\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"x\" name=\"v\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"flags\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"SetBool\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"b\" name=\"v\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"flags\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"SetDouble\">\n"

  "\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"d\" name=\"v\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"flags\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "\n"

  "  </interface>\n"

;
};

org_workrave_ConfigInterface *org_workrave_ConfigInterface::instance(std::shared_ptr<::workrave::dbus::IDBus> dbus)
{
  org_workrave_ConfigInterface_Stub *iface = nullptr;
  ::workrave::dbus::DBusBinding *binding = dbus->find_binding("org.workrave.ConfigInterface");
  if (binding != nullptr)
    {
      iface = dynamic_cast<org_workrave_ConfigInterface_Stub *>(binding);
    }
  return iface;
}

bool
org_workrave_ConfigInterface_Stub::call(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  std::string method_name = message.member().toStdString();
  constexpr std::array<DBusMethod, 18> table = method_table;
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
    << ::workrave::dbus::interface_info("org.workrave.ConfigInterface");
}


void
org_workrave_ConfigInterface_Stub::RemoveKey(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 1)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("RemoveKey")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));




      dbus_object->remove_key(p_key);


      QDBusMessage reply = message.createReply();





      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("RemoveKey")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("RemoveKey") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::RenameKey(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      std::string p_new_key{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 2)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("RenameKey")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));



      p_new_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(1));




      dbus_object->rename_key(p_key, p_new_key);


      QDBusMessage reply = message.createReply();







      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("RenameKey")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("RenameKey") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::HasUserValue(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};


      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 1)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("HasUserValue")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));




      p_result = dbus_object->has_user_value(p_key);


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);





      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("HasUserValue")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("HasUserValue") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::GetString(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      std::string p_out{};


      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 1)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetString")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));






      p_result = dbus_object->get_value(p_key, p_out);


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);





      reply << ::workrave::dbus::DBusMarshall<std::string>::convert(p_out);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetString")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetString") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::GetBool(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      bool p_out{};


      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 1)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetBool")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));






      p_result = dbus_object->get_value(p_key, p_out);


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);





      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_out);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetBool")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetBool") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::GetInt(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      int32_t p_out{};


      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 1)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetInt")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));






      p_result = dbus_object->get_value(p_key, p_out);


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);





      reply << ::workrave::dbus::DBusMarshall<int32_t>::convert(p_out);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetInt")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetInt") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::GetInt64(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      int64_t p_out{};


      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 1)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetInt64")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));






      p_result = dbus_object->get_value(p_key, p_out);


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);





      reply << ::workrave::dbus::DBusMarshall<int64_t>::convert(p_out);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetInt64")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetInt64") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::GetDouble(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      double p_out{};


      bool p_result{};


      auto num_in_args = message.arguments().size();
      if (num_in_args != 1)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetDouble")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));






      p_result = dbus_object->get_value(p_key, p_out);


      QDBusMessage reply = message.createReply();

      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_result);





      reply << ::workrave::dbus::DBusMarshall<double>::convert(p_out);



      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetDouble")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetDouble") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::GetStringWithDefault(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      std::string p_out{};

      std::string p_s{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 2)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetStringWithDefault")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));





      p_s = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(1));




      dbus_object->get_value_with_default(p_key, p_out, p_s);


      QDBusMessage reply = message.createReply();





      reply << ::workrave::dbus::DBusMarshall<std::string>::convert(p_out);





      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetStringWithDefault")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetStringWithDefault") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::GetBoolWithDefault(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      bool p_out{};

      bool p_def{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 2)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetBoolWithDefault")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));





      p_def = ::workrave::dbus::DBusMarshall<bool>::convert(message.arguments().at(1));




      dbus_object->get_value_with_default(p_key, p_out, p_def);


      QDBusMessage reply = message.createReply();





      reply << ::workrave::dbus::DBusMarshall<bool>::convert(p_out);





      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetBoolWithDefault")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetBoolWithDefault") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::GetIntWithDefault(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      int32_t p_out{};

      int32_t p_def{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 2)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetIntWithDefault")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));





      p_def = ::workrave::dbus::DBusMarshall<int32_t>::convert(message.arguments().at(1));




      dbus_object->get_value_with_default(p_key, p_out, p_def);


      QDBusMessage reply = message.createReply();





      reply << ::workrave::dbus::DBusMarshall<int32_t>::convert(p_out);





      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetIntWithDefault")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetIntWithDefault") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::GetInt64WithDefault(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      int64_t p_out{};

      int64_t p_def{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 2)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetInt64WithDefault")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));





      p_def = ::workrave::dbus::DBusMarshall<int64_t>::convert(message.arguments().at(1));




      dbus_object->get_value_with_default(p_key, p_out, p_def);


      QDBusMessage reply = message.createReply();





      reply << ::workrave::dbus::DBusMarshall<int64_t>::convert(p_out);





      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetInt64WithDefault")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetInt64WithDefault") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::GetDoubleWithDefault(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      double p_out{};

      double p_def{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 2)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("GetDoubleWithDefault")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));





      p_def = ::workrave::dbus::DBusMarshall<double>::convert(message.arguments().at(1));




      dbus_object->get_value_with_default(p_key, p_out, p_def);


      QDBusMessage reply = message.createReply();





      reply << ::workrave::dbus::DBusMarshall<double>::convert(p_out);





      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("GetDoubleWithDefault")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("GetDoubleWithDefault") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::SetString(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      std::string p_v{};

      workrave::config::ConfigFlags p_flags{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 3)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("SetString")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));



      p_v = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(1));



      p_flags = ::workrave::dbus::DBusMarshall<workrave::config::ConfigFlags>::convert(message.arguments().at(2));




      dbus_object->set_value(p_key, p_v, p_flags);


      QDBusMessage reply = message.createReply();









      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("SetString")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("SetString") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::SetInt(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      int32_t p_v{};

      workrave::config::ConfigFlags p_flags{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 3)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("SetInt")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));



      p_v = ::workrave::dbus::DBusMarshall<int32_t>::convert(message.arguments().at(1));



      p_flags = ::workrave::dbus::DBusMarshall<workrave::config::ConfigFlags>::convert(message.arguments().at(2));




      dbus_object->set_value(p_key, p_v, p_flags);


      QDBusMessage reply = message.createReply();









      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("SetInt")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("SetInt") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::SetInt64(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      int64_t p_v{};

      workrave::config::ConfigFlags p_flags{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 3)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("SetInt64")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));



      p_v = ::workrave::dbus::DBusMarshall<int64_t>::convert(message.arguments().at(1));



      p_flags = ::workrave::dbus::DBusMarshall<workrave::config::ConfigFlags>::convert(message.arguments().at(2));




      dbus_object->set_value(p_key, p_v, p_flags);


      QDBusMessage reply = message.createReply();









      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("SetInt64")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("SetInt64") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::SetBool(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      bool p_v{};

      workrave::config::ConfigFlags p_flags{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 3)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("SetBool")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));



      p_v = ::workrave::dbus::DBusMarshall<bool>::convert(message.arguments().at(1));



      p_flags = ::workrave::dbus::DBusMarshall<workrave::config::ConfigFlags>::convert(message.arguments().at(2));




      dbus_object->set_value(p_key, p_v, p_flags);


      QDBusMessage reply = message.createReply();









      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("SetBool")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("SetBool") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}


void
org_workrave_ConfigInterface_Stub::SetDouble(void *object, const QDBusMessage &message, const QDBusConnection &connection)
{
  try
    {
      auto *dbus_object = static_cast<workrave::config::IConfigurator *>(object);


      std::string p_key{};

      double p_v{};

      workrave::config::ConfigFlags p_flags{};



      auto num_in_args = message.arguments().size();
      if (num_in_args != 3)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Incorrect number of in-parameters")
            << workrave::dbus::error_code_info(DBUS_ERROR_INVALID_ARGS)
            << workrave::dbus::method_info("SetDouble")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }



      p_key = ::workrave::dbus::DBusMarshall<std::string>::convert(message.arguments().at(0));



      p_v = ::workrave::dbus::DBusMarshall<double>::convert(message.arguments().at(1));



      p_flags = ::workrave::dbus::DBusMarshall<workrave::config::ConfigFlags>::convert(message.arguments().at(2));




      dbus_object->set_value(p_key, p_v, p_flags);


      QDBusMessage reply = message.createReply();









      bool rc = connection.send(reply);
      if (!rc)
        {
          throw workrave::dbus::DBusRemoteException()
            << workrave::dbus::message_info("Failed to send reply")
            << workrave::dbus::error_code_info(DBUS_ERROR_FAILED)
            << workrave::dbus::method_info("SetDouble")
            << workrave::dbus::interface_info("org.workrave.ConfigInterface");
        }
    }
  catch (const DBusRemoteException &e)
    {
      e << method_info("SetDouble") << interface_info("org.workrave.ConfigInterface");
      throw;
    }
}



void init_org_workrave_ConfigInterface(std::shared_ptr<::workrave::dbus::IDBus> dbus)
{
  dbus->register_binding("org.workrave.ConfigInterface", new org_workrave_ConfigInterface_Stub(dbus));


  qDBusRegisterMetaType<workrave::config::ConfigFlags>();

}

} // namespace workrave::core::rpc
