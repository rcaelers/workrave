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

#include "RpcDBusAppletBindingDBus.hh"


namespace workrave::rpc::dbus
{
template<>
struct QtCodec<GenericDBusApplet::MenuItem>
{
  static GenericDBusApplet::MenuItem decode(const QVariant &variant)
  {
    const auto arg = variant.value<QDBusArgument>();
    GenericDBusApplet::MenuItem result{};
    arg.beginStructure();

    result.text = QtCodec<std::string>::decode(arg.asVariant());

    result.dynamic_text = QtCodec<std::string>::decode(arg.asVariant());

    result.action = QtCodec<std::string>::decode(arg.asVariant());

    result.command = QtCodec<uint32_t>::decode(arg.asVariant());

    result.type = QtCodec<uint8_t>::decode(arg.asVariant());

    result.flags = QtCodec<uint8_t>::decode(arg.asVariant());

    arg.endStructure();
    return result;
  }
  static void append(QDBusArgument &arg, const GenericDBusApplet::MenuItem &value)
  {
    arg.beginStructure();

    QtCodec<std::string>::append(arg, value.text);

    QtCodec<std::string>::append(arg, value.dynamic_text);

    QtCodec<std::string>::append(arg, value.action);

    QtCodec<uint32_t>::append(arg, value.command);

    QtCodec<uint8_t>::append(arg, value.type);

    QtCodec<uint8_t>::append(arg, value.flags);

    arg.endStructure();
  }
  static QVariant encode(const GenericDBusApplet::MenuItem &value)
  {
    QDBusArgument arg;
    append(arg, value);
    return QVariant::fromValue(arg);
  }
};
} // namespace workrave::rpc::dbus

// See the enum case above for why these are at global scope, not nested in
// workrave::rpc::dbus: ADL needs to find them from GenericDBusApplet::MenuItem's own associated
// namespace, not this library's.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const GenericDBusApplet::MenuItem &data)
{
  workrave::rpc::dbus::QtCodec<GenericDBusApplet::MenuItem>::append(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, GenericDBusApplet::MenuItem &data)
{
  data = workrave::rpc::dbus::QtCodec<GenericDBusApplet::MenuItem>::decode(QVariant::fromValue(arg));
  return arg;
}

namespace workrave::rpc::dbus
{
template<>
struct QtCodec<GenericDBusApplet::TimerData>
{
  static GenericDBusApplet::TimerData decode(const QVariant &variant)
  {
    const auto arg = variant.value<QDBusArgument>();
    GenericDBusApplet::TimerData result{};
    arg.beginStructure();

    result.bar_text = QtCodec<std::string>::decode(arg.asVariant());

    result.slot = QtCodec<int>::decode(arg.asVariant());

    result.bar_secondary_color = QtCodec<uint32_t>::decode(arg.asVariant());

    result.bar_secondary_val = QtCodec<uint32_t>::decode(arg.asVariant());

    result.bar_secondary_max = QtCodec<uint32_t>::decode(arg.asVariant());

    result.bar_primary_color = QtCodec<uint32_t>::decode(arg.asVariant());

    result.bar_primary_val = QtCodec<uint32_t>::decode(arg.asVariant());

    result.bar_primary_max = QtCodec<uint32_t>::decode(arg.asVariant());

    arg.endStructure();
    return result;
  }
  static void append(QDBusArgument &arg, const GenericDBusApplet::TimerData &value)
  {
    arg.beginStructure();

    QtCodec<std::string>::append(arg, value.bar_text);

    QtCodec<int>::append(arg, value.slot);

    QtCodec<uint32_t>::append(arg, value.bar_secondary_color);

    QtCodec<uint32_t>::append(arg, value.bar_secondary_val);

    QtCodec<uint32_t>::append(arg, value.bar_secondary_max);

    QtCodec<uint32_t>::append(arg, value.bar_primary_color);

    QtCodec<uint32_t>::append(arg, value.bar_primary_val);

    QtCodec<uint32_t>::append(arg, value.bar_primary_max);

    arg.endStructure();
  }
  static QVariant encode(const GenericDBusApplet::TimerData &value)
  {
    QDBusArgument arg;
    append(arg, value);
    return QVariant::fromValue(arg);
  }
};
} // namespace workrave::rpc::dbus

// See the enum case above for why these are at global scope, not nested in
// workrave::rpc::dbus: ADL needs to find them from GenericDBusApplet::TimerData's own associated
// namespace, not this library's.
[[maybe_unused]] static QDBusArgument &operator<<(QDBusArgument &arg, const GenericDBusApplet::TimerData &data)
{
  workrave::rpc::dbus::QtCodec<GenericDBusApplet::TimerData>::append(arg, data);
  return arg;
}

[[maybe_unused]] static const QDBusArgument &operator>>(const QDBusArgument &arg, GenericDBusApplet::TimerData &data)
{
  data = workrave::rpc::dbus::QtCodec<GenericDBusApplet::TimerData>::decode(QVariant::fromValue(arg));
  return arg;
}


namespace workrave::ui::rpc
{

org_workrave_AppletInterface::org_workrave_AppletInterface(::workrave::rpc::dbus::QtServer &server,
                         std::string path,
                         GenericDBusApplet &implementation)

  : server_(server)
  , path_(std::move(path))

  , implementation_(implementation)
{


  qDBusRegisterMetaType<GenericDBusApplet::MenuItem>();

  qDBusRegisterMetaType<GenericDBusApplet::TimerData>();

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

  "\n"

  "    <method name=\"Embed\">\n"

  "\n"

  "      <arg type=\"b\" name=\"enabled\" direction=\"in\" />\n"

  "\n"

  "      <arg type=\"s\" name=\"sender\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"Command\">\n"

  "\n"

  "      <arg type=\"i\" name=\"command\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"MenuAction\">\n"

  "\n"

  "      <arg type=\"s\" name=\"action\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"ButtonClicked\">\n"

  "\n"

  "      <arg type=\"u\" name=\"button\" direction=\"in\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetMenu\">\n"

  "\n"

  "      <arg type=\"a(sssuyy)\" name=\"menuitems\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "    <method name=\"GetTrayIconEnabled\">\n"

  "\n"

  "      <arg type=\"b\" name=\"enabled\" direction=\"out\" />\n"

  "\n"

  "    </method>\n"

  "\n"

  "\n"

  "    <signal name=\"TimersUpdated\">\n"

  "\n"

  "      <arg type=\"(siuuuuuu)\" name=\"micro\" />\n"

  "\n"

  "      <arg type=\"(siuuuuuu)\" name=\"rest\" />\n"

  "\n"

  "      <arg type=\"(siuuuuuu)\" name=\"daily\" />\n"

  "\n"

  "    </signal>\n"

  "\n"

  "    <signal name=\"MenuUpdated\">\n"

  "\n"

  "      <arg type=\"a(sssuyy)\" name=\"menuitems\" />\n"

  "\n"

  "    </signal>\n"

  "\n"

  "    <signal name=\"MenuItemUpdated\">\n"

  "\n"

  "      <arg type=\"(sssuyy)\" name=\"menuitem\" />\n"

  "\n"

  "    </signal>\n"

  "\n"

  "    <signal name=\"TrayIconUpdated\">\n"

  "\n"

  "      <arg type=\"b\" name=\"enabled\" />\n"

  "\n"

  "    </signal>\n"

  "\n"

  "  </interface>\n";
  return xml;
}

bool
org_workrave_AppletInterface::dispatch(const QDBusMessage &message, const QDBusConnection &connection)
{
  using Method = void (org_workrave_AppletInterface::*)(const QDBusMessage &, const QDBusConnection &);
  struct Entry
  {
    std::string_view name;
    Method method;
  };
  static constexpr std::array<Entry, 6> methods =
  { {

      {.name = "Embed", .method = &org_workrave_AppletInterface::dispatch_Embed},

      {.name = "Command", .method = &org_workrave_AppletInterface::dispatch_Command},

      {.name = "MenuAction", .method = &org_workrave_AppletInterface::dispatch_MenuAction},

      {.name = "ButtonClicked", .method = &org_workrave_AppletInterface::dispatch_ButtonClicked},

      {.name = "GetMenu", .method = &org_workrave_AppletInterface::dispatch_GetMenu},

      {.name = "GetTrayIconEnabled", .method = &org_workrave_AppletInterface::dispatch_GetTrayIconEnabled},

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
org_workrave_AppletInterface::dispatch_Embed(const QDBusMessage &message, const QDBusConnection &connection)
{

  bool p_enabled{};

  std::string p_sender{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 2)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.AppletInterface.Embed");
    }



  p_enabled = ::workrave::rpc::dbus::QtCodec<bool>::decode(message.arguments().at(0));



  p_sender = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(1));




  implementation_.applet_embed(p_enabled, p_sender);


  QDBusMessage reply = message.createReply();







  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.AppletInterface.Embed");
    }
}


void
org_workrave_AppletInterface::dispatch_Command(const QDBusMessage &message, const QDBusConnection &connection)
{

  int p_command{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.AppletInterface.Command");
    }



  p_command = ::workrave::rpc::dbus::QtCodec<int>::decode(message.arguments().at(0));




  implementation_.applet_command(p_command);


  QDBusMessage reply = message.createReply();





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.AppletInterface.Command");
    }
}


void
org_workrave_AppletInterface::dispatch_MenuAction(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::string p_action{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.AppletInterface.MenuAction");
    }



  p_action = ::workrave::rpc::dbus::QtCodec<std::string>::decode(message.arguments().at(0));




  implementation_.applet_menu_action(p_action);


  QDBusMessage reply = message.createReply();





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.AppletInterface.MenuAction");
    }
}


void
org_workrave_AppletInterface::dispatch_ButtonClicked(const QDBusMessage &message, const QDBusConnection &connection)
{

  uint32_t p_button{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 1)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.AppletInterface.ButtonClicked");
    }



  p_button = ::workrave::rpc::dbus::QtCodec<uint32_t>::decode(message.arguments().at(0));




  implementation_.button_clicked(p_button);


  QDBusMessage reply = message.createReply();





  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.AppletInterface.ButtonClicked");
    }
}


void
org_workrave_AppletInterface::dispatch_GetMenu(const QDBusMessage &message, const QDBusConnection &connection)
{

  std::list<GenericDBusApplet::MenuItem> p_menuitems{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.AppletInterface.GetMenu");
    }






  implementation_.get_menu(p_menuitems);


  QDBusMessage reply = message.createReply();



  reply << ::workrave::rpc::dbus::QtCodec<std::list<GenericDBusApplet::MenuItem>>::encode(p_menuitems);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.AppletInterface.GetMenu");
    }
}


void
org_workrave_AppletInterface::dispatch_GetTrayIconEnabled(const QDBusMessage &message, const QDBusConnection &connection)
{

  bool p_enabled{};



  const auto num_in_args = message.arguments().size();
  if (num_in_args != 0)
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::invalid_args),
        "Incorrect number of input parameters for org.workrave.AppletInterface.GetTrayIconEnabled");
    }






  implementation_.get_tray_icon_enabled(p_enabled);


  QDBusMessage reply = message.createReply();



  reply << ::workrave::rpc::dbus::QtCodec<bool>::encode(p_enabled);



  if (!connection.send(reply))
    {
      throw ::workrave::rpc::dbus::Error(
        std::string(::workrave::rpc::dbus::error_names::failed),
        "Failed to send reply for org.workrave.AppletInterface.GetTrayIconEnabled");
    }
}


void
org_workrave_AppletInterface::emit_TimersUpdated(GenericDBusApplet::TimerData micro, GenericDBusApplet::TimerData rest, GenericDBusApplet::TimerData daily)
{
  QVariantList arguments;

  arguments << ::workrave::rpc::dbus::QtCodec<GenericDBusApplet::TimerData>::encode(micro);

  arguments << ::workrave::rpc::dbus::QtCodec<GenericDBusApplet::TimerData>::encode(rest);

  arguments << ::workrave::rpc::dbus::QtCodec<GenericDBusApplet::TimerData>::encode(daily);

  server_.emit_signal(path_, "org.workrave.AppletInterface", "TimersUpdated", arguments);
}
void
org_workrave_AppletInterface::emit_MenuUpdated(std::list<GenericDBusApplet::MenuItem> menuitems)
{
  QVariantList arguments;

  arguments << ::workrave::rpc::dbus::QtCodec<std::list<GenericDBusApplet::MenuItem>>::encode(menuitems);

  server_.emit_signal(path_, "org.workrave.AppletInterface", "MenuUpdated", arguments);
}
void
org_workrave_AppletInterface::emit_MenuItemUpdated(GenericDBusApplet::MenuItem menuitem)
{
  QVariantList arguments;

  arguments << ::workrave::rpc::dbus::QtCodec<GenericDBusApplet::MenuItem>::encode(menuitem);

  server_.emit_signal(path_, "org.workrave.AppletInterface", "MenuItemUpdated", arguments);
}
void
org_workrave_AppletInterface::emit_TrayIconUpdated(bool enabled)
{
  QVariantList arguments;

  arguments << ::workrave::rpc::dbus::QtCodec<bool>::encode(enabled);

  server_.emit_signal(path_, "org.workrave.AppletInterface", "TrayIconUpdated", arguments);
}
} // namespace workrave::ui::rpc
