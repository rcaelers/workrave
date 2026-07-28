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

#include "RpcDBusBreakBindingDBus.hh"


namespace workrave::rpc::dbus
{
template<>
struct GioSignature<workrave::BreakEvent>
{
  static std::string value() { return "s"; }
};

template<>
struct GioCodec<workrave::BreakEvent>
{
  static workrave::BreakEvent decode(GVariant *variant)
  {
    const std::string value = GioCodec<std::string>::decode(variant);

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

  static GVariant *encode(const workrave::BreakEvent &value)
  {
    switch (value)
      {

      case workrave::BreakEvent::ShowPrelude: return g_variant_new_string("show_prelude");

      case workrave::BreakEvent::ShowBreak: return g_variant_new_string("show_break");

      case workrave::BreakEvent::ShowBreakForced: return g_variant_new_string("show_break_forced");

      case workrave::BreakEvent::BreakStart: return g_variant_new_string("break_start");

      case workrave::BreakEvent::BreakIdle: return g_variant_new_string("break_idle");

      case workrave::BreakEvent::BreakStop: return g_variant_new_string("break_stop");

      case workrave::BreakEvent::BreakIgnored: return g_variant_new_string("break_ignored");

      case workrave::BreakEvent::BreakPostponed: return g_variant_new_string("break_postponed");

      case workrave::BreakEvent::BreakSkipped: return g_variant_new_string("break_skipped");

      case workrave::BreakEvent::BreakTaken: return g_variant_new_string("break_taken");

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
  }
};
} // namespace workrave::rpc::dbus

namespace workrave::rpc::dbus
{
template<>
struct GioSignature<BreakStage>
{
  static std::string value() { return "s"; }
};

template<>
struct GioCodec<BreakStage>
{
  static BreakStage decode(GVariant *variant)
  {
    const std::string value = GioCodec<std::string>::decode(variant);

    if (value == "none") return BreakStage::None;

    if (value == "snoozed") return BreakStage::Snoozed;

    if (value == "prelude") return BreakStage::Prelude;

    if (value == "taking") return BreakStage::Taking;

    if (value == "delayed") return BreakStage::Delayed;

    throw Error(std::string(error_names::invalid_args), "Unknown DBus enum value: " + value);
  }

  static GVariant *encode(const BreakStage &value)
  {
    switch (value)
      {

      case BreakStage::None: return g_variant_new_string("none");

      case BreakStage::Snoozed: return g_variant_new_string("snoozed");

      case BreakStage::Prelude: return g_variant_new_string("prelude");

      case BreakStage::Taking: return g_variant_new_string("taking");

      case BreakStage::Delayed: return g_variant_new_string("delayed");

      default:
        throw Error(std::string(error_names::invalid_args), "Type error in enum");
      }
  }
};
} // namespace workrave::rpc::dbus


namespace workrave::core::rpc
{

org_workrave_BreakInterface::org_workrave_BreakInterface(::workrave::rpc::dbus::GioServer &server,
                         std::string path,
                         Break &implementation)

  : server_(server)
  , path_(std::move(path))

  , implementation_(implementation)
{

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

  "    <method name=\"GetName\">\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"IsEnabled\">\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"IsTimerRunning\">\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"IsTaking\">\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"IsMaxPreludesReached\">\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"IsActive\">\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetTimerElapsed\">\n"

  "      <arg type=\"i\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetTimerIdle\">\n"

  "      <arg type=\"i\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetAutoReset\">\n"

  "      <arg type=\"x\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"IsAutoResetEnabled\">\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetLimit\">\n"

  "      <arg type=\"x\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"IsLimitEnabled\">\n"

  "      <arg type=\"b\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetTimerRemaining\">\n"

  "      <arg type=\"i\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetTimerOverdue\">\n"

  "      <arg type=\"i\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"PostponeBreak\">\n"

  "    </method>\n"

  "    <method name=\"SkipBreak\">\n"

  "    </method>\n"

  "    <method name=\"GetBreakState\">\n"

  "      <arg type=\"s\" name=\"result\" direction=\"out\" />\n"

  "    </method>\n"


  "    <signal name=\"BreakEvent\">\n"

  "      <arg type=\"s\" name=\"value\" />\n"

  "    </signal>\n"

  "    <signal name=\"BreakStateChanged\">\n"

  "      <arg type=\"s\" name=\"value\" />\n"

  "    </signal>\n"

  "  </interface>\n";
  return xml;
}

void
org_workrave_BreakInterface::dispatch(std::string_view method, GVariant *parameters, GDBusMethodInvocation *invocation)
{
  using Method = void (org_workrave_BreakInterface::*)(GVariant *, GDBusMethodInvocation *);
  struct Entry { std::string_view name; Method method; };
  static constexpr std::array<Entry, 17> methods = { {

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
org_workrave_BreakInterface::dispatch_GetName(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetName");
    }


  std::string p_result{};
  p_result = implementation_.get_name();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<std::string>::encode(p_result));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_BreakInterface::dispatch_IsEnabled(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsEnabled");
    }


  bool p_result{};
  p_result = implementation_.is_enabled();


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
org_workrave_BreakInterface::dispatch_IsTimerRunning(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsTimerRunning");
    }


  bool p_result{};
  p_result = implementation_.is_running();


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
org_workrave_BreakInterface::dispatch_IsTaking(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsTaking");
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
org_workrave_BreakInterface::dispatch_IsMaxPreludesReached(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsMaxPreludesReached");
    }


  bool p_result{};
  p_result = implementation_.is_max_preludes_reached();


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
org_workrave_BreakInterface::dispatch_IsActive(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsActive");
    }


  bool p_result{};
  p_result = implementation_.is_active();


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
org_workrave_BreakInterface::dispatch_GetTimerElapsed(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetTimerElapsed");
    }


  int64_t p_result{};
  p_result = implementation_.get_elapsed_time();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<int32_t>::encode(::workrave::rpc::dbus::checked_dbus_wire_cast<int32_t>(p_result)));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_BreakInterface::dispatch_GetTimerIdle(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetTimerIdle");
    }


  int64_t p_result{};
  p_result = implementation_.get_elapsed_idle_time();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<int32_t>::encode(::workrave::rpc::dbus::checked_dbus_wire_cast<int32_t>(p_result)));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_BreakInterface::dispatch_GetAutoReset(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetAutoReset");
    }


  int64_t p_result{};
  p_result = implementation_.get_auto_reset();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<int64_t>::encode(p_result));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_BreakInterface::dispatch_IsAutoResetEnabled(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsAutoResetEnabled");
    }


  bool p_result{};
  p_result = implementation_.is_auto_reset_enabled();


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
org_workrave_BreakInterface::dispatch_GetLimit(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetLimit");
    }


  int64_t p_result{};
  p_result = implementation_.get_limit();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<int64_t>::encode(p_result));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_BreakInterface::dispatch_IsLimitEnabled(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.IsLimitEnabled");
    }


  bool p_result{};
  p_result = implementation_.is_limit_enabled();


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
org_workrave_BreakInterface::dispatch_GetTimerRemaining(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetTimerRemaining");
    }


  int64_t p_result{};
  p_result = implementation_.get_timer_remaining();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<int32_t>::encode(::workrave::rpc::dbus::checked_dbus_wire_cast<int32_t>(p_result)));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_BreakInterface::dispatch_GetTimerOverdue(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetTimerOverdue");
    }


  int64_t p_result{};
  p_result = implementation_.get_total_overdue_time();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<int32_t>::encode(::workrave::rpc::dbus::checked_dbus_wire_cast<int32_t>(p_result)));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_BreakInterface::dispatch_PostponeBreak(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.PostponeBreak");
    }


  implementation_.postpone_break();


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
org_workrave_BreakInterface::dispatch_SkipBreak(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.SkipBreak");
    }


  implementation_.skip_break();


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
org_workrave_BreakInterface::dispatch_GetBreakState(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.BreakInterface.GetBreakState");
    }


  std::string p_result{};
  p_result = implementation_.get_break_stage();


  std::vector<GVariant *> reply_values;
  ::workrave::rpc::dbus::GioUnixFdList reply_fd_list;

  reply_values.push_back(::workrave::rpc::dbus::GioCodec<std::string>::encode(p_result));


  GVariant *reply = g_variant_new_tuple(
    reply_values.empty() ? nullptr : reply_values.data(), reply_values.size());
  if (reply_fd_list.get() != nullptr)
    g_dbus_method_invocation_return_value_with_unix_fd_list(invocation, reply, reply_fd_list.get());
  else
    g_dbus_method_invocation_return_value(invocation, reply);
}


void
org_workrave_BreakInterface::emit_BreakEvent(workrave::BreakEvent value)
{
  std::vector<GVariant *> values;
  ::workrave::rpc::dbus::GioUnixFdList fd_list;

  values.push_back(::workrave::rpc::dbus::GioCodec<workrave::BreakEvent>::encode(value));

  server_.emit_signal(path_, "org.workrave.BreakInterface", "BreakEvent",
                      g_variant_new_tuple(values.empty() ? nullptr : values.data(), values.size()),
                      fd_list.get());
}
void
org_workrave_BreakInterface::emit_BreakStateChanged(BreakStage value)
{
  std::vector<GVariant *> values;
  ::workrave::rpc::dbus::GioUnixFdList fd_list;

  values.push_back(::workrave::rpc::dbus::GioCodec<BreakStage>::encode(value));

  server_.emit_signal(path_, "org.workrave.BreakInterface", "BreakStateChanged",
                      g_variant_new_tuple(values.empty() ? nullptr : values.data(), values.size()),
                      fd_list.get());
}
} // namespace workrave::core::rpc
