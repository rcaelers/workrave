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

#include "rpc/Duration.hh"

#include "RpcDBusCoreBindingDBus.hh"


namespace workrave::rpc::dbus
{
template<>
struct GioSignature<workrave::BreakId>
{
  static std::string value() { return "s"; }
};

template<>
struct GioCodec<workrave::BreakId>
{
  static workrave::BreakId decode(GVariant *variant)
  {
    const std::string value = GioCodec<std::string>::decode(variant);

    if (value == "none") return workrave::BreakId::BREAK_ID_NONE;

    if (value == "microbreak") return workrave::BreakId::BREAK_ID_MICRO_BREAK;

    if (value == "restbreak") return workrave::BreakId::BREAK_ID_REST_BREAK;

    if (value == "dailylimit") return workrave::BreakId::BREAK_ID_DAILY_LIMIT;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }

  static GVariant *encode(const workrave::BreakId &value)
  {
    switch (value)
      {

      case workrave::BreakId::BREAK_ID_NONE: return g_variant_new_string("none");

      case workrave::BreakId::BREAK_ID_MICRO_BREAK: return g_variant_new_string("microbreak");

      case workrave::BreakId::BREAK_ID_REST_BREAK: return g_variant_new_string("restbreak");

      case workrave::BreakId::BREAK_ID_DAILY_LIMIT: return g_variant_new_string("dailylimit");

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
  }
};
} // namespace workrave::rpc::dbus

namespace workrave::rpc::dbus
{
template<>
struct GioSignature<workrave::BreakHint>
{
  static std::string value() { return "s"; }
};

template<>
struct GioCodec<workrave::BreakHint>
{
  static workrave::BreakHint decode(GVariant *variant)
  {
    const std::string value = GioCodec<std::string>::decode(variant);

    if (value == "normal") return workrave::BreakHint::Normal;

    if (value == "userinitiated") return workrave::BreakHint::UserInitiated;

    if (value == "naturalbreak") return workrave::BreakHint::NaturalBreak;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }

  static GVariant *encode(const workrave::BreakHint &value)
  {
    switch (value)
      {

      case workrave::BreakHint::Normal: return g_variant_new_string("normal");

      case workrave::BreakHint::UserInitiated: return g_variant_new_string("userinitiated");

      case workrave::BreakHint::NaturalBreak: return g_variant_new_string("naturalbreak");

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
  }
};
} // namespace workrave::rpc::dbus

namespace workrave::rpc::dbus
{
template<>
struct GioSignature<workrave::utils::Flags<workrave::BreakHint>>
{
  static std::string value() { return GioSignature<std::list<workrave::BreakHint>>::value(); }
};

template<>
struct GioCodec<workrave::utils::Flags<workrave::BreakHint>>
{
  static workrave::utils::Flags<workrave::BreakHint> decode(GVariant *variant)
  {
    const auto items = GioCodec<std::list<workrave::BreakHint>>::decode(variant);
    workrave::utils::Flags<workrave::BreakHint> result;
    for (const auto &item: items) result |= item;
    return result;
  }

  static GVariant *encode(const workrave::utils::Flags<workrave::BreakHint> &value)
  {
    std::list<workrave::BreakHint> items;
    for (unsigned int bit = 0; bit < workrave::utils::enum_traits<workrave::BreakHint>::bits; ++bit)
      {
        const auto item = static_cast<workrave::BreakHint>(1U << bit);
        if (value.is_set(item)) items.push_back(item);
      }
    return GioCodec<std::list<workrave::BreakHint>>::encode(items);
  }
};
} // namespace workrave::rpc::dbus

namespace workrave::rpc::dbus
{
template<>
struct GioSignature<workrave::OperationMode>
{
  static std::string value() { return "s"; }
};

template<>
struct GioCodec<workrave::OperationMode>
{
  static workrave::OperationMode decode(GVariant *variant)
  {
    const std::string value = GioCodec<std::string>::decode(variant);

    if (value == "normal") return workrave::OperationMode::Normal;

    if (value == "suspended") return workrave::OperationMode::Suspended;

    if (value == "quiet") return workrave::OperationMode::Quiet;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }

  static GVariant *encode(const workrave::OperationMode &value)
  {
    switch (value)
      {

      case workrave::OperationMode::Normal: return g_variant_new_string("normal");

      case workrave::OperationMode::Suspended: return g_variant_new_string("suspended");

      case workrave::OperationMode::Quiet: return g_variant_new_string("quiet");

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
  }
};
} // namespace workrave::rpc::dbus

namespace workrave::rpc::dbus
{
template<>
struct GioSignature<std::chrono::minutes>
{
  static std::string value() { return "s"; }
};

template<>
struct GioCodec<std::chrono::minutes>
{
  static std::chrono::minutes decode(GVariant *variant)
  {
    const std::string text = GioCodec<std::string>::decode(variant);
    return std::chrono::duration_cast<std::chrono::minutes>(::rpc::parse_duration(text));
  }

  static GVariant *encode(const std::chrono::minutes &value)
  {
    const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(value).count();
    return GioCodec<std::string>::encode(std::to_string(seconds) + "s");
  }
};
} // namespace workrave::rpc::dbus

namespace workrave::rpc::dbus
{
template<>
struct GioSignature<workrave::UsageMode>
{
  static std::string value() { return "s"; }
};

template<>
struct GioCodec<workrave::UsageMode>
{
  static workrave::UsageMode decode(GVariant *variant)
  {
    const std::string value = GioCodec<std::string>::decode(variant);

    if (value == "normal") return workrave::UsageMode::Normal;

    if (value == "reading") return workrave::UsageMode::Reading;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }

  static GVariant *encode(const workrave::UsageMode &value)
  {
    switch (value)
      {

      case workrave::UsageMode::Normal: return g_variant_new_string("normal");

      case workrave::UsageMode::Reading: return g_variant_new_string("reading");

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
  }
};
} // namespace workrave::rpc::dbus


namespace workrave::core::rpc
{

org_workrave_CoreInterface::org_workrave_CoreInterface(::workrave::rpc::dbus::GioServer &server,
                         std::string path,
                         Core &implementation)

  : server_(server)
  , path_(std::move(path))

  , implementation_(implementation)
{

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

  "    <method name=\"ForceBreak\">\n"

  "      <arg type=\"s\" name=\"id\" direction=\"in\" />\n"

  "      <arg type=\"as\" name=\"break_hint\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"IsActive\">\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"IsTaking\">\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetActiveOperationMode\">\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetOperationMode\">\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"IsOperationModeAnOverride\">\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"SetOperationMode\">\n"

  "      <arg type=\"s\" name=\"mode\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"SetOperationModeFor\">\n"

  "      <arg type=\"s\" name=\"mode\" direction=\"in\" />\n"

  "      <arg type=\"s\" name=\"duration\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"GetUsageMode\">\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"SetUsageMode\">\n"

  "      <arg type=\"s\" name=\"mode\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"ReportActivity\">\n"

  "      <arg type=\"s\" name=\"who\" direction=\"in\" />\n"

  "      <arg type=\"b\" name=\"act\" direction=\"in\" />\n"

  "    </method>\n"


  "    <signal name=\"OperationModeChanged\">\n"

  "      <arg type=\"s\" name=\"value\" />\n"

  "    </signal>\n"

  "    <signal name=\"UsageModeChanged\">\n"

  "      <arg type=\"s\" name=\"value\" />\n"

  "    </signal>\n"

  "  </interface>\n";
  return xml;
}

void
org_workrave_CoreInterface::dispatch(std::string_view method, GVariant *parameters, GDBusMethodInvocation *invocation)
{
  using Method = void (org_workrave_CoreInterface::*)(GVariant *, GDBusMethodInvocation *);
  struct Entry { std::string_view name; Method method; };
  static constexpr std::array<Entry, 11> methods = { {

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
org_workrave_CoreInterface::dispatch_ForceBreak(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.ForceBreak");
    }

  workrave::BreakId p_id{};

  p_id = ::workrave::rpc::dbus::gio_decode_child<workrave::BreakId>(parameters, 0);


  workrave::utils::Flags<workrave::BreakHint> p_break_hint{};

  p_break_hint = ::workrave::rpc::dbus::gio_decode_child<workrave::utils::Flags<workrave::BreakHint>>(parameters, 1);



  implementation_.force_break(p_id, p_break_hint);


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;






  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_CoreInterface::dispatch_IsActive(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.IsActive");
    }


  bool p_result{};
  p_result = implementation_.is_user_active();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<bool>::encode(p_result));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_CoreInterface::dispatch_IsTaking(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.IsTaking");
    }


  bool p_result{};
  p_result = implementation_.is_taking();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<bool>::encode(p_result));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_CoreInterface::dispatch_GetActiveOperationMode(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.GetActiveOperationMode");
    }


  workrave::OperationMode p_result{};
  p_result = implementation_.get_active_operation_mode();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<workrave::OperationMode>::encode(p_result));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_CoreInterface::dispatch_GetOperationMode(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.GetOperationMode");
    }


  workrave::OperationMode p_result{};
  p_result = implementation_.get_regular_operation_mode();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<workrave::OperationMode>::encode(p_result));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_CoreInterface::dispatch_IsOperationModeAnOverride(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.IsOperationModeAnOverride");
    }


  bool p_result{};
  p_result = implementation_.is_operation_mode_an_override();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<bool>::encode(p_result));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_CoreInterface::dispatch_SetOperationMode(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.SetOperationMode");
    }

  workrave::OperationMode p_mode{};

  p_mode = ::workrave::rpc::dbus::gio_decode_child<workrave::OperationMode>(parameters, 0);



  implementation_.set_operation_mode(p_mode);


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;




  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_CoreInterface::dispatch_SetOperationModeFor(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.SetOperationModeFor");
    }

  workrave::OperationMode p_mode{};

  p_mode = ::workrave::rpc::dbus::gio_decode_child<workrave::OperationMode>(parameters, 0);


  std::chrono::minutes p_duration{};

  p_duration = ::workrave::rpc::dbus::gio_decode_child<std::chrono::minutes>(parameters, 1);



  implementation_.set_operation_mode_for(p_mode, p_duration);


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;






  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_CoreInterface::dispatch_GetUsageMode(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.GetUsageMode");
    }


  workrave::UsageMode p_result{};
  p_result = implementation_.get_usage_mode();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<workrave::UsageMode>::encode(p_result));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_CoreInterface::dispatch_SetUsageMode(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.SetUsageMode");
    }

  workrave::UsageMode p_mode{};

  p_mode = ::workrave::rpc::dbus::gio_decode_child<workrave::UsageMode>(parameters, 0);



  implementation_.set_usage_mode(p_mode);


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;




  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_CoreInterface::dispatch_ReportActivity(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.CoreInterface.ReportActivity");
    }

  std::string p_who{};

  p_who = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);


  bool p_act{};

  p_act = ::workrave::rpc::dbus::gio_decode_child<bool>(parameters, 1);



  implementation_.report_external_activity(p_who, p_act);


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;






  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_CoreInterface::emit_OperationModeChanged(workrave::OperationMode value)
{
  std::vector<GVariant *> values;
  ::workrave::rpc::dbus::GioUnixFdList fd_list;

  values.push_back(::workrave::rpc::dbus::GioCodec<workrave::OperationMode>::encode(value));

  server_.emit_signal(path_, "org.workrave.CoreInterface", "OperationModeChanged",
                      g_variant_new_tuple(values.empty() ? nullptr : values.data(), values.size()),
                      fd_list.get());
}
void
org_workrave_CoreInterface::emit_UsageModeChanged(workrave::UsageMode value)
{
  std::vector<GVariant *> values;
  ::workrave::rpc::dbus::GioUnixFdList fd_list;

  values.push_back(::workrave::rpc::dbus::GioCodec<workrave::UsageMode>::encode(value));

  server_.emit_signal(path_, "org.workrave.CoreInterface", "UsageModeChanged",
                      g_variant_new_tuple(values.empty() ? nullptr : values.data(), values.size()),
                      fd_list.get());
}
} // namespace workrave::core::rpc
