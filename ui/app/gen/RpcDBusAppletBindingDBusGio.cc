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

#include "RpcDBusAppletBindingDBus.hh"


namespace workrave::rpc::dbus
{
template<>
struct GioSignature<GenericDBusApplet::MenuItem>
{
  static std::string value()
  {
    std::string result = "(";

    result += GioSignature<std::string>::value();

    result += GioSignature<std::string>::value();

    result += GioSignature<std::string>::value();

    result += GioSignature<uint32_t>::value();

    result += GioSignature<uint8_t>::value();

    result += GioSignature<uint8_t>::value();

    result += ")";
    return result;
  }
};

template<>
struct GioCodec<GenericDBusApplet::MenuItem>
{
  static GenericDBusApplet::MenuItem decode(GVariant *variant)
  {
    gio_require_type(variant, GioSignature<GenericDBusApplet::MenuItem>::value());
    GenericDBusApplet::MenuItem result{};

    result.text = gio_decode_child<std::string>(variant, 0);

    result.dynamic_text = gio_decode_child<std::string>(variant, 1);

    result.action = gio_decode_child<std::string>(variant, 2);

    result.command = gio_decode_child<uint32_t>(variant, 3);

    result.type = gio_decode_child<uint8_t>(variant, 4);

    result.flags = gio_decode_child<uint8_t>(variant, 5);

    return result;
  }

  static GVariant *encode(const GenericDBusApplet::MenuItem &value)
  {
    GVariant *fields[] = {

      GioCodec<std::string>::encode(value.text),

      GioCodec<std::string>::encode(value.dynamic_text),

      GioCodec<std::string>::encode(value.action),

      GioCodec<uint32_t>::encode(value.command),

      GioCodec<uint8_t>::encode(value.type),

      GioCodec<uint8_t>::encode(value.flags),

    };
    return g_variant_new_tuple(fields, 6);
  }
};
} // namespace workrave::rpc::dbus

namespace workrave::rpc::dbus
{
template<>
struct GioSignature<GenericDBusApplet::TimerData>
{
  static std::string value()
  {
    std::string result = "(";

    result += GioSignature<std::string>::value();

    result += GioSignature<int>::value();

    result += GioSignature<uint32_t>::value();

    result += GioSignature<uint32_t>::value();

    result += GioSignature<uint32_t>::value();

    result += GioSignature<uint32_t>::value();

    result += GioSignature<uint32_t>::value();

    result += GioSignature<uint32_t>::value();

    result += ")";
    return result;
  }
};

template<>
struct GioCodec<GenericDBusApplet::TimerData>
{
  static GenericDBusApplet::TimerData decode(GVariant *variant)
  {
    gio_require_type(variant, GioSignature<GenericDBusApplet::TimerData>::value());
    GenericDBusApplet::TimerData result{};

    result.bar_text = gio_decode_child<std::string>(variant, 0);

    result.slot = gio_decode_child<int>(variant, 1);

    result.bar_secondary_color = gio_decode_child<uint32_t>(variant, 2);

    result.bar_secondary_val = gio_decode_child<uint32_t>(variant, 3);

    result.bar_secondary_max = gio_decode_child<uint32_t>(variant, 4);

    result.bar_primary_color = gio_decode_child<uint32_t>(variant, 5);

    result.bar_primary_val = gio_decode_child<uint32_t>(variant, 6);

    result.bar_primary_max = gio_decode_child<uint32_t>(variant, 7);

    return result;
  }

  static GVariant *encode(const GenericDBusApplet::TimerData &value)
  {
    GVariant *fields[] = {

      GioCodec<std::string>::encode(value.bar_text),

      GioCodec<int>::encode(value.slot),

      GioCodec<uint32_t>::encode(value.bar_secondary_color),

      GioCodec<uint32_t>::encode(value.bar_secondary_val),

      GioCodec<uint32_t>::encode(value.bar_secondary_max),

      GioCodec<uint32_t>::encode(value.bar_primary_color),

      GioCodec<uint32_t>::encode(value.bar_primary_val),

      GioCodec<uint32_t>::encode(value.bar_primary_max),

    };
    return g_variant_new_tuple(fields, 8);
  }
};
} // namespace workrave::rpc::dbus


namespace workrave::ui::rpc
{

org_workrave_AppletInterface::org_workrave_AppletInterface(::workrave::rpc::dbus::GioServer &server,
                         std::string path,
                         GenericDBusApplet &implementation)

  : server_(server)
  , path_(std::move(path))

  , implementation_(implementation)
{

  signal_connections_.emplace_back(implementation_.signal_timers_updated().connect(
    [this](GenericDBusApplet::TimerData micro, GenericDBusApplet::TimerData rest, GenericDBusApplet::TimerData daily) {
      emit_TimersUpdated(micro, rest, daily);
    }));

  signal_connections_.emplace_back(implementation_.signal_menu_updated().connect(
    [this](std::list<GenericDBusApplet::MenuItem> menuitems) {
      emit_MenuUpdated(menuitems);
    }));

  signal_connections_.emplace_back(implementation_.signal_menu_item_updated().connect(
    [this](GenericDBusApplet::MenuItem menuitem) {
      emit_MenuItemUpdated(menuitem);
    }));

  signal_connections_.emplace_back(implementation_.signal_tray_icon_updated().connect(
    [this](bool enabled) {
      emit_TrayIconUpdated(enabled);
    }));

}

std::string_view
org_workrave_AppletInterface::name() const noexcept
{
  return "org.workrave.AppletInterface";
}

std::string_view
org_workrave_AppletInterface::introspection() const noexcept
{
  static constexpr std::string_view xml =
  "  <interface name=\"org.workrave.AppletInterface\">\n"

  "    <method name=\"Embed\">\n"

  "      <arg type=\"b\" name=\"enabled\" direction=\"in\" />\n"

  "      <arg type=\"s\" name=\"sender\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"Command\">\n"

  "      <arg type=\"i\" name=\"command\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"MenuAction\">\n"

  "      <arg type=\"s\" name=\"action\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"ButtonClicked\">\n"

  "      <arg type=\"u\" name=\"button\" direction=\"in\" />\n"

  "    </method>\n"

  "    <method name=\"GetMenu\">\n"

  "      <arg type=\"a(sssuyy)\" name=\"menuitems\" direction=\"out\" />\n"

  "    </method>\n"

  "    <method name=\"GetTrayIconEnabled\">\n"

  "      <arg type=\"b\" name=\"enabled\" direction=\"out\" />\n"

  "    </method>\n"


  "    <signal name=\"TimersUpdated\">\n"

  "      <arg type=\"(siuuuuuu)\" name=\"micro\" />\n"

  "      <arg type=\"(siuuuuuu)\" name=\"rest\" />\n"

  "      <arg type=\"(siuuuuuu)\" name=\"daily\" />\n"

  "    </signal>\n"

  "    <signal name=\"MenuUpdated\">\n"

  "      <arg type=\"a(sssuyy)\" name=\"menuitems\" />\n"

  "    </signal>\n"

  "    <signal name=\"MenuItemUpdated\">\n"

  "      <arg type=\"(sssuyy)\" name=\"menuitem\" />\n"

  "    </signal>\n"

  "    <signal name=\"TrayIconUpdated\">\n"

  "      <arg type=\"b\" name=\"enabled\" />\n"

  "    </signal>\n"

  "  </interface>\n";
  return xml;
}

void
org_workrave_AppletInterface::dispatch(std::string_view method, GVariant *parameters, GDBusMethodInvocation *invocation)
{
  using Method = void (org_workrave_AppletInterface::*)(GVariant *, GDBusMethodInvocation *);
  struct Entry { std::string_view name; Method method; };
  static constexpr std::array<Entry, 6> methods = { {

    {.name = "Embed", .method = &org_workrave_AppletInterface::dispatch_Embed},

    {.name = "Command", .method = &org_workrave_AppletInterface::dispatch_Command},

    {.name = "MenuAction", .method = &org_workrave_AppletInterface::dispatch_MenuAction},

    {.name = "ButtonClicked", .method = &org_workrave_AppletInterface::dispatch_ButtonClicked},

    {.name = "GetMenu", .method = &org_workrave_AppletInterface::dispatch_GetMenu},

    {.name = "GetTrayIconEnabled", .method = &org_workrave_AppletInterface::dispatch_GetTrayIconEnabled},

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
org_workrave_AppletInterface::dispatch_Embed(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.AppletInterface.Embed");
    }

  bool p_enabled{};

  p_enabled = ::workrave::rpc::dbus::gio_decode_child<bool>(parameters, 0);


  std::string p_sender{};

  p_sender = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 1);



  implementation_.applet_embed(p_enabled, p_sender);


  std::vector<GVariant *> reply_values;






  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_AppletInterface::dispatch_Command(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.AppletInterface.Command");
    }

  int p_command{};

  p_command = ::workrave::rpc::dbus::gio_decode_child<int>(parameters, 0);



  implementation_.applet_command(p_command);


  std::vector<GVariant *> reply_values;




  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_AppletInterface::dispatch_MenuAction(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.AppletInterface.MenuAction");
    }

  std::string p_action{};

  p_action = ::workrave::rpc::dbus::gio_decode_child<std::string>(parameters, 0);



  implementation_.applet_menu_action(p_action);


  std::vector<GVariant *> reply_values;




  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_AppletInterface::dispatch_ButtonClicked(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.AppletInterface.ButtonClicked");
    }

  uint32_t p_button{};

  p_button = ::workrave::rpc::dbus::gio_decode_child<uint32_t>(parameters, 0);



  implementation_.button_clicked(p_button);


  std::vector<GVariant *> reply_values;




  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_AppletInterface::dispatch_GetMenu(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.AppletInterface.GetMenu");
    }

  std::list<GenericDBusApplet::MenuItem> p_menuitems{};



  implementation_.get_menu(p_menuitems);


  std::vector<GVariant *> reply_values;



  reply_values.push_back(::workrave::rpc::dbus::GioCodec<std::list<GenericDBusApplet::MenuItem>>::encode(p_menuitems));


  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_AppletInterface::dispatch_GetTrayIconEnabled(GVariant *parameters, GDBusMethodInvocation *invocation)
{
  if (parameters == nullptr || !g_variant_is_of_type(parameters, G_VARIANT_TYPE_TUPLE)
      || g_variant_n_children(parameters) != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.AppletInterface.GetTrayIconEnabled");
    }

  bool p_enabled{};



  implementation_.get_tray_icon_enabled(p_enabled);


  std::vector<GVariant *> reply_values;



  reply_values.push_back(::workrave::rpc::dbus::GioCodec<bool>::encode(p_enabled));


  g_dbus_method_invocation_return_value(
    invocation,
    g_variant_new_tuple(reply_values.empty() ? nullptr : reply_values.data(), reply_values.size()));
}


void
org_workrave_AppletInterface::emit_TimersUpdated(GenericDBusApplet::TimerData micro, GenericDBusApplet::TimerData rest, GenericDBusApplet::TimerData daily)
{
  std::vector<GVariant *> values;

  values.push_back(::workrave::rpc::dbus::GioCodec<GenericDBusApplet::TimerData>::encode(micro));

  values.push_back(::workrave::rpc::dbus::GioCodec<GenericDBusApplet::TimerData>::encode(rest));

  values.push_back(::workrave::rpc::dbus::GioCodec<GenericDBusApplet::TimerData>::encode(daily));

  server_.emit_signal(path_, "org.workrave.AppletInterface", "TimersUpdated",
                      g_variant_new_tuple(values.empty() ? nullptr : values.data(), values.size()));
}
void
org_workrave_AppletInterface::emit_MenuUpdated(std::list<GenericDBusApplet::MenuItem> menuitems)
{
  std::vector<GVariant *> values;

  values.push_back(::workrave::rpc::dbus::GioCodec<std::list<GenericDBusApplet::MenuItem>>::encode(menuitems));

  server_.emit_signal(path_, "org.workrave.AppletInterface", "MenuUpdated",
                      g_variant_new_tuple(values.empty() ? nullptr : values.data(), values.size()));
}
void
org_workrave_AppletInterface::emit_MenuItemUpdated(GenericDBusApplet::MenuItem menuitem)
{
  std::vector<GVariant *> values;

  values.push_back(::workrave::rpc::dbus::GioCodec<GenericDBusApplet::MenuItem>::encode(menuitem));

  server_.emit_signal(path_, "org.workrave.AppletInterface", "MenuItemUpdated",
                      g_variant_new_tuple(values.empty() ? nullptr : values.data(), values.size()));
}
void
org_workrave_AppletInterface::emit_TrayIconUpdated(bool enabled)
{
  std::vector<GVariant *> values;

  values.push_back(::workrave::rpc::dbus::GioCodec<bool>::encode(enabled));

  server_.emit_signal(path_, "org.workrave.AppletInterface", "TrayIconUpdated",
                      g_variant_new_tuple(values.empty() ? nullptr : values.data(), values.size()));
}
} // namespace workrave::ui::rpc
