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

#include "RpcDBusConfigBindingDBus.hh"


namespace workrave::rpc::dbus
{
template<>
struct QtCodec<workrave::config::ConfigFlags>
{
  static workrave::config::ConfigFlags decode(const QVariant &variant)
  {
    const std::string value = QtCodec<std::string>::decode(variant);

    if (value == "none") return workrave::config::ConfigFlags::CONFIG_FLAG_NONE;

    if (value == "initial") return workrave::config::ConfigFlags::CONFIG_FLAG_INITIAL;

    if (value == "immediate") return workrave::config::ConfigFlags::CONFIG_FLAG_IMMEDIATE;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }
  static void append(QDBusArgument &arg, const workrave::config::ConfigFlags &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::config::ConfigFlags::CONFIG_FLAG_NONE: str = "none"; break;

      case workrave::config::ConfigFlags::CONFIG_FLAG_INITIAL: str = "initial"; break;

      case workrave::config::ConfigFlags::CONFIG_FLAG_IMMEDIATE: str = "immediate"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    arg << QString::fromStdString(str);
  }
  static QVariant encode(const workrave::config::ConfigFlags &value)
  {
    std::string str;
    switch (value)
      {

      case workrave::config::ConfigFlags::CONFIG_FLAG_NONE: str = "none"; break;

      case workrave::config::ConfigFlags::CONFIG_FLAG_INITIAL: str = "initial"; break;

      case workrave::config::ConfigFlags::CONFIG_FLAG_IMMEDIATE: str = "immediate"; break;

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
    return QVariant::fromValue(QString::fromStdString(str));
  }
};
} // namespace workrave::rpc::dbus

// ADL requires these at global scope (or a namespace associated with one of
// the parameter types) to be found from Qt's own template code — nesting
// them inside workrave::rpc::dbus, where they'd only be found via workrave::config::ConfigFlags's
// own namespace (which may not be workrave::rpc::dbus at all), silently breaks
// qDBusRegisterMetaType<workrave::config::ConfigFlags>() and any use as a sequence/map element.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const workrave::config::ConfigFlags &data)
{
  workrave::rpc::dbus::QtCodec<workrave::config::ConfigFlags>::append(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, workrave::config::ConfigFlags &data)
{
  QString value;
  arg >> value;
  data = workrave::rpc::dbus::QtCodec<workrave::config::ConfigFlags>::decode(QVariant::fromValue(value));
  return arg;
}


namespace workrave::core::rpc
{

org_workrave_ConfigInterface::org_workrave_ConfigInterface(::workrave::rpc::dbus::QtServer &server,
                         std::string path,
                         workrave::config::IConfigurator &implementation)

  : path_(std::move(path))

  , implementation_(implementation)
{

  (void)server;


  qDBusRegisterMetaType<workrave::config::ConfigFlags>();

}

std::string_view
org_workrave_ConfigInterface::name() const noexcept
{
  return "org.workrave.ConfigInterface";
}

std::string_view
org_workrave_ConfigInterface::introspection() const noexcept
{
  static constexpr std::string_view xml =

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

  "  </interface>\n";
  return xml;
}

bool
org_workrave_ConfigInterface::dispatch(const QDBusMessage &message, const QDBusConnection &connection)
{
  using Method = void (org_workrave_ConfigInterface::*)(const QDBusMessage &, const QDBusConnection &);
  struct Entry
  {
    std::string_view name;
    Method method;
  };
  static constexpr std::array<Entry, 18> methods =
  { {

      {.name = "RemoveKey", .method = &org_workrave_ConfigInterface::dispatch_RemoveKey},

      {.name = "RenameKey", .method = &org_workrave_ConfigInterface::dispatch_RenameKey},

      {.name = "HasUserValue", .method = &org_workrave_ConfigInterface::dispatch_HasUserValue},

      {.name = "GetString", .method = &org_workrave_ConfigInterface::dispatch_GetString},

      {.name = "GetBool", .method = &org_workrave_ConfigInterface::dispatch_GetBool},

      {.name = "GetInt", .method = &org_workrave_ConfigInterface::dispatch_GetInt},

      {.name = "GetInt64", .method = &org_workrave_ConfigInterface::dispatch_GetInt64},

      {.name = "GetDouble", .method = &org_workrave_ConfigInterface::dispatch_GetDouble},

      {.name = "GetStringWithDefault", .method = &org_workrave_ConfigInterface::dispatch_GetStringWithDefault},

      {.name = "GetBoolWithDefault", .method = &org_workrave_ConfigInterface::dispatch_GetBoolWithDefault},

      {.name = "GetIntWithDefault", .method = &org_workrave_ConfigInterface::dispatch_GetIntWithDefault},

      {.name = "GetInt64WithDefault", .method = &org_workrave_ConfigInterface::dispatch_GetInt64WithDefault},

      {.name = "GetDoubleWithDefault", .method = &org_workrave_ConfigInterface::dispatch_GetDoubleWithDefault},

      {.name = "SetString", .method = &org_workrave_ConfigInterface::dispatch_SetString},

      {.name = "SetInt", .method = &org_workrave_ConfigInterface::dispatch_SetInt},

      {.name = "SetInt64", .method = &org_workrave_ConfigInterface::dispatch_SetInt64},

      {.name = "SetBool", .method = &org_workrave_ConfigInterface::dispatch_SetBool},

      {.name = "SetDouble", .method = &org_workrave_ConfigInterface::dispatch_SetDouble},

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
org_workrave_ConfigInterface::dispatch_RemoveKey(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.RemoveKey");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));




  implementation_.remove_key(p_key);


  QDBusMessage reply = message.createReply();





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.RemoveKey");
    }
}


void
org_workrave_ConfigInterface::dispatch_RenameKey(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  std::string p_new_key{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.RenameKey");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));



  p_new_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(1));




  implementation_.rename_key(p_key, p_new_key);


  QDBusMessage reply = message.createReply();







  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.RenameKey");
    }
}


void
org_workrave_ConfigInterface::dispatch_HasUserValue(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.HasUserValue");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));




  p_result = implementation_.has_user_value(p_key);


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.HasUserValue");
    }
}


void
org_workrave_ConfigInterface::dispatch_GetString(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  std::string p_out{};


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetString");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));






  p_result = implementation_.get_value(p_key, p_out);


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);





  reply << ::workrave::rpc::dbus::QtCodec<std::string>::encode(p_out);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.GetString");
    }
}


void
org_workrave_ConfigInterface::dispatch_GetBool(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  bool p_out{};


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetBool");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));






  p_result = implementation_.get_value(p_key, p_out);


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);





  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_out);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.GetBool");
    }
}


void
org_workrave_ConfigInterface::dispatch_GetInt(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  int32_t p_out{};


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetInt");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));






  p_result = implementation_.get_value(p_key, p_out);


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);





  reply << ::workrave::rpc::dbus::QtCodec<int32_t>::encode(p_out);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.GetInt");
    }
}


void
org_workrave_ConfigInterface::dispatch_GetInt64(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  int64_t p_out{};


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetInt64");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));






  p_result = implementation_.get_value(p_key, p_out);


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);





  reply << ::workrave::rpc::dbus::QtCodec<int64_t>::encode(p_out);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.GetInt64");
    }
}


void
org_workrave_ConfigInterface::dispatch_GetDouble(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  double p_out{};


  bool p_result{};


  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetDouble");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));






  p_result = implementation_.get_value(p_key, p_out);


  QDBusMessage reply = message.createReply();

  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_result);





  reply << ::workrave::rpc::dbus::QtCodec<double>::encode(p_out);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.GetDouble");
    }
}


void
org_workrave_ConfigInterface::dispatch_GetStringWithDefault(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  std::string p_out{};

  std::string p_s{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetStringWithDefault");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));





  p_s = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(1));




  implementation_.get_value_with_default(p_key, p_out, p_s);


  QDBusMessage reply = message.createReply();





  reply << ::workrave::rpc::dbus::QtCodec<std::string>::encode(p_out);





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.GetStringWithDefault");
    }
}


void
org_workrave_ConfigInterface::dispatch_GetBoolWithDefault(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  bool p_out{};

  bool p_def{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetBoolWithDefault");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));





  p_def = ::workrave::rpc::dbus::QtCodec<bool>::decode(message.arguments().at(1));




  implementation_.get_value_with_default(p_key, p_out, p_def);


  QDBusMessage reply = message.createReply();





  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_out);





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.GetBoolWithDefault");
    }
}


void
org_workrave_ConfigInterface::dispatch_GetIntWithDefault(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  int32_t p_out{};

  int32_t p_def{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetIntWithDefault");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));





  p_def = ::workrave::rpc::dbus::QtCodec<int32_t>::decode(message.arguments().at(1));




  implementation_.get_value_with_default(p_key, p_out, p_def);


  QDBusMessage reply = message.createReply();





  reply << ::workrave::rpc::dbus::QtCodec<int32_t>::encode(p_out);





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.GetIntWithDefault");
    }
}


void
org_workrave_ConfigInterface::dispatch_GetInt64WithDefault(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  int64_t p_out{};

  int64_t p_def{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetInt64WithDefault");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));





  p_def = ::workrave::rpc::dbus::QtCodec<int64_t>::decode(message.arguments().at(1));




  implementation_.get_value_with_default(p_key, p_out, p_def);


  QDBusMessage reply = message.createReply();





  reply << ::workrave::rpc::dbus::QtCodec<int64_t>::encode(p_out);





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.GetInt64WithDefault");
    }
}


void
org_workrave_ConfigInterface::dispatch_GetDoubleWithDefault(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  double p_out{};

  double p_def{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetDoubleWithDefault");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));





  p_def = ::workrave::rpc::dbus::QtCodec<double>::decode(message.arguments().at(1));




  implementation_.get_value_with_default(p_key, p_out, p_def);


  QDBusMessage reply = message.createReply();





  reply << ::workrave::rpc::dbus::QtCodec<double>::encode(p_out);





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.GetDoubleWithDefault");
    }
}


void
org_workrave_ConfigInterface::dispatch_SetString(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  std::string p_v{};

  workrave::config::ConfigFlags p_flags{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 3)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.SetString");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));



  p_v = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(1));



  p_flags = ::workrave::rpc::dbus::QtCodec<workrave::config::ConfigFlags>::decode(message.arguments().at(2));




  implementation_.set_value(p_key, p_v, p_flags);


  QDBusMessage reply = message.createReply();









  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.SetString");
    }
}


void
org_workrave_ConfigInterface::dispatch_SetInt(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  int32_t p_v{};

  workrave::config::ConfigFlags p_flags{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 3)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.SetInt");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));



  p_v = ::workrave::rpc::dbus::QtCodec<int32_t>::decode(message.arguments().at(1));



  p_flags = ::workrave::rpc::dbus::QtCodec<workrave::config::ConfigFlags>::decode(message.arguments().at(2));




  implementation_.set_value(p_key, p_v, p_flags);


  QDBusMessage reply = message.createReply();









  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.SetInt");
    }
}


void
org_workrave_ConfigInterface::dispatch_SetInt64(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  int64_t p_v{};

  workrave::config::ConfigFlags p_flags{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 3)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.SetInt64");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));



  p_v = ::workrave::rpc::dbus::QtCodec<int64_t>::decode(message.arguments().at(1));



  p_flags = ::workrave::rpc::dbus::QtCodec<workrave::config::ConfigFlags>::decode(message.arguments().at(2));




  implementation_.set_value(p_key, p_v, p_flags);


  QDBusMessage reply = message.createReply();









  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.SetInt64");
    }
}


void
org_workrave_ConfigInterface::dispatch_SetBool(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  bool p_v{};

  workrave::config::ConfigFlags p_flags{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 3)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.SetBool");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));



  p_v = ::workrave::rpc::dbus::QtCodec<bool>::decode(message.arguments().at(1));



  p_flags = ::workrave::rpc::dbus::QtCodec<workrave::config::ConfigFlags>::decode(message.arguments().at(2));




  implementation_.set_value(p_key, p_v, p_flags);


  QDBusMessage reply = message.createReply();









  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.SetBool");
    }
}


void
org_workrave_ConfigInterface::dispatch_SetDouble(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_key{};

  double p_v{};

  workrave::config::ConfigFlags p_flags{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 3)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.SetDouble");
    }



  p_key = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));



  p_v = ::workrave::rpc::dbus::QtCodec<double>::decode(message.arguments().at(1));



  p_flags = ::workrave::rpc::dbus::QtCodec<workrave::config::ConfigFlags>::decode(message.arguments().at(2));




  implementation_.set_value(p_key, p_v, p_flags);


  QDBusMessage reply = message.createReply();









  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.ConfigInterface.SetDouble");
    }
}


} // namespace workrave::core::rpc
