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

#include "RpcDBusConfigBindingDBus.hh"


namespace workrave::rpc::dbus
{
template<>
struct GioSignature<workrave::config::ConfigFlags>
{
  static std::string value() { return "s"; }
};

template<>
struct GioCodec<workrave::config::ConfigFlags>
{
  static workrave::config::ConfigFlags decode(GVariant *variant)
  {
    const std::string value = GioCodec<std::string>::decode(variant);

    if (value == "none") return workrave::config::ConfigFlags::CONFIG_FLAG_NONE;

    if (value == "initial") return workrave::config::ConfigFlags::CONFIG_FLAG_INITIAL;

    if (value == "immediate") return workrave::config::ConfigFlags::CONFIG_FLAG_IMMEDIATE;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }

  static GVariant *encode(const workrave::config::ConfigFlags &value)
  {
    switch (value)
      {

      case workrave::config::ConfigFlags::CONFIG_FLAG_NONE: return g_variant_new_string("none");

      case workrave::config::ConfigFlags::CONFIG_FLAG_INITIAL: return g_variant_new_string("initial");

      case workrave::config::ConfigFlags::CONFIG_FLAG_IMMEDIATE: return g_variant_new_string("immediate");

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
  }
};
} // namespace workrave::rpc::dbus


namespace workrave::core::rpc
{

org_workrave_ConfigInterface::org_workrave_ConfigInterface(::workrave::rpc::dbus::GioServer &server,
                         std::string path,
                         workrave::config::IConfigurator &implementation)

  : path_(std::move(path))

  , implementation_(implementation)
{

  (void)server;

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

  "    <method name=\"RemoveKey\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"RenameKey\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"s\" name=\"new_key\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"HasUserValue\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetString\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"s\" name=\"out\" direction=\"out\" />\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetBool\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"b\" name=\"out\" direction=\"out\" />\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetInt\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"i\" name=\"out\" direction=\"out\" />\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetInt64\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"x\" name=\"out\" direction=\"out\" />\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetDouble\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"d\" name=\"out\" direction=\"out\" />\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetStringWithDefault\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"s\" name=\"out\" direction=\"out\" />\n"

  "      <arg type=\"s\" name=\"s\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"GetBoolWithDefault\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"b\" name=\"out\" direction=\"out\" />\n"

  "      <arg type=\"b\" name=\"def\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"GetIntWithDefault\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"i\" name=\"out\" direction=\"out\" />\n"

  "      <arg type=\"i\" name=\"def\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"GetInt64WithDefault\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"x\" name=\"out\" direction=\"out\" />\n"

  "      <arg type=\"x\" name=\"def\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"GetDoubleWithDefault\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"d\" name=\"out\" direction=\"out\" />\n"

  "      <arg type=\"d\" name=\"def\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"SetString\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"s\" name=\"v\" direction=\"in\" />\n"

  "      <arg type=\"s\" name=\"flags\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"SetInt\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"i\" name=\"v\" direction=\"in\" />\n"

  "      <arg type=\"s\" name=\"flags\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"SetInt64\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"x\" name=\"v\" direction=\"in\" />\n"

  "      <arg type=\"s\" name=\"flags\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"SetBool\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"b\" name=\"v\" direction=\"in\" />\n"

  "      <arg type=\"s\" name=\"flags\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"SetDouble\">\n"

  "      <arg type=\"s\" name=\"key\" direction=\"in\" />\n"

  "      <arg type=\"d\" name=\"v\" direction=\"in\" />\n"

  "      <arg type=\"s\" name=\"flags\" direction=\"in\" />\n"

  "    </method>\n"


  "  </interface>\n";
  return xml;
}

void
org_workrave_ConfigInterface::dispatch(std::string_view method, GVariant *parameters, GDBusMethodInvocation *invocation)
{
  using Method = void (org_workrave_ConfigInterface::*)(GVariant *, GDBusMethodInvocation *);
  struct Entry { std::string_view name; Method method; };
  static constexpr std::array<Entry, 18> methods = { {

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
org_workrave_ConfigInterface::dispatch_RemoveKey(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.RemoveKey");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);



  implementation_.remove_key(p_key);


  std::vector<GVariant *> reply_values;




  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_RenameKey(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.RenameKey");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  std::string p_new_key{};

  p_new_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 1);



  implementation_.rename_key(p_key, p_new_key);


  std::vector<GVariant *> reply_values;






  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_HasUserValue(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.HasUserValue");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);



  bool p_result{};
  p_result = implementation_.has_user_value(p_key);


  std::vector<GVariant *> reply_values;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<bool>::encode(p_result));




  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_GetString(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetString");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  std::string p_out{};



  bool p_result{};
  p_result = implementation_.get_value(p_key, p_out);


  std::vector<GVariant *> reply_values;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<bool>::encode(p_result));





  reply_values.push_back(::workrave::rpc::dbus::GioCodec<std::string>::encode(p_out));


  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_GetBool(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetBool");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  bool p_out{};



  bool p_result{};
  p_result = implementation_.get_value(p_key, p_out);


  std::vector<GVariant *> reply_values;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<bool>::encode(p_result));





  reply_values.push_back(::workrave::rpc::dbus::GioCodec<bool>::encode(p_out));


  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_GetInt(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetInt");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  int32_t p_out{};



  bool p_result{};
  p_result = implementation_.get_value(p_key, p_out);


  std::vector<GVariant *> reply_values;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<bool>::encode(p_result));





  reply_values.push_back(::workrave::rpc::dbus::GioCodec<int32_t>::encode(p_out));


  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_GetInt64(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetInt64");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  int64_t p_out{};



  bool p_result{};
  p_result = implementation_.get_value(p_key, p_out);


  std::vector<GVariant *> reply_values;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<bool>::encode(p_result));





  reply_values.push_back(::workrave::rpc::dbus::GioCodec<int64_t>::encode(p_out));


  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_GetDouble(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetDouble");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  double p_out{};



  bool p_result{};
  p_result = implementation_.get_value(p_key, p_out);


  std::vector<GVariant *> reply_values;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<bool>::encode(p_result));





  reply_values.push_back(::workrave::rpc::dbus::GioCodec<double>::encode(p_out));


  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_GetStringWithDefault(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetStringWithDefault");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  std::string p_out{};


  std::string p_s{};

  p_s = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 1);



  implementation_.get_value_with_default(p_key, p_out, p_s);


  std::vector<GVariant *> reply_values;





  reply_values.push_back(::workrave::rpc::dbus::GioCodec<std::string>::encode(p_out));




  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_GetBoolWithDefault(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetBoolWithDefault");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  bool p_out{};


  bool p_def{};

  p_def = ::workrave::rpc::dbus::gio_decode_child<bool>(parameters, 1);



  implementation_.get_value_with_default(p_key, p_out, p_def);


  std::vector<GVariant *> reply_values;





  reply_values.push_back(::workrave::rpc::dbus::GioCodec<bool>::encode(p_out));




  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_GetIntWithDefault(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetIntWithDefault");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  int32_t p_out{};


  int32_t p_def{};

  p_def = ::workrave::rpc::dbus::gio_decode_child<int32_t>(parameters, 1);



  implementation_.get_value_with_default(p_key, p_out, p_def);


  std::vector<GVariant *> reply_values;





  reply_values.push_back(::workrave::rpc::dbus::GioCodec<int32_t>::encode(p_out));




  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_GetInt64WithDefault(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetInt64WithDefault");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  int64_t p_out{};


  int64_t p_def{};

  p_def = ::workrave::rpc::dbus::gio_decode_child<int64_t>(parameters, 1);



  implementation_.get_value_with_default(p_key, p_out, p_def);


  std::vector<GVariant *> reply_values;





  reply_values.push_back(::workrave::rpc::dbus::GioCodec<int64_t>::encode(p_out));




  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_GetDoubleWithDefault(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.GetDoubleWithDefault");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  double p_out{};


  double p_def{};

  p_def = ::workrave::rpc::dbus::gio_decode_child<double>(parameters, 1);



  implementation_.get_value_with_default(p_key, p_out, p_def);


  std::vector<GVariant *> reply_values;





  reply_values.push_back(::workrave::rpc::dbus::GioCodec<double>::encode(p_out));




  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_SetString(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 3)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.SetString");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  std::string p_v{};

  p_v = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 1);


  workrave::config::ConfigFlags p_flags{};

  p_flags = ::workrave::rpc::dbus::gio_decode_child<workrave::config::ConfigFlags>(parameters, 2);



  implementation_.set_value(p_key, p_v, p_flags);


  std::vector<GVariant *> reply_values;








  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_SetInt(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 3)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.SetInt");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  int32_t p_v{};

  p_v = ::workrave::rpc::dbus::gio_decode_child<int32_t>(parameters, 1);


  workrave::config::ConfigFlags p_flags{};

  p_flags = ::workrave::rpc::dbus::gio_decode_child<workrave::config::ConfigFlags>(parameters, 2);



  implementation_.set_value(p_key, p_v, p_flags);


  std::vector<GVariant *> reply_values;








  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_SetInt64(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 3)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.SetInt64");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  int64_t p_v{};

  p_v = ::workrave::rpc::dbus::gio_decode_child<int64_t>(parameters, 1);


  workrave::config::ConfigFlags p_flags{};

  p_flags = ::workrave::rpc::dbus::gio_decode_child<workrave::config::ConfigFlags>(parameters, 2);



  implementation_.set_value(p_key, p_v, p_flags);


  std::vector<GVariant *> reply_values;








  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_SetBool(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 3)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.SetBool");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  bool p_v{};

  p_v = ::workrave::rpc::dbus::gio_decode_child<bool>(parameters, 1);


  workrave::config::ConfigFlags p_flags{};

  p_flags = ::workrave::rpc::dbus::gio_decode_child<workrave::config::ConfigFlags>(parameters, 2);



  implementation_.set_value(p_key, p_v, p_flags);


  std::vector<GVariant *> reply_values;








  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_ConfigInterface::dispatch_SetDouble(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 3)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.ConfigInterface.SetDouble");
    }

  std::string p_key{};

  p_key = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  double p_v{};

  p_v = ::workrave::rpc::dbus::gio_decode_child<double>(parameters, 1);


  workrave::config::ConfigFlags p_flags{};

  p_flags = ::workrave::rpc::dbus::gio_decode_child<workrave::config::ConfigFlags>(parameters, 2);



  implementation_.set_value(p_key, p_v, p_flags);


  std::vector<GVariant *> reply_values;








  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


} // namespace workrave::core::rpc
