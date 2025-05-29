// Copyright (C) 2025 Rob Caelers <robc@krandor.nl>
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "DBusPreludeWindow.hh"

#include <spdlog/spdlog.h>
#include <giomm.h>

#include "GtkUtil.hh"
#include "utils/Exception.hh"
#include "utils/Enum.hh"
#include "commonui/Text.hh"

using namespace workrave;
using namespace workrave::utils;

class DBusPreludeWindow::Impl
{
public:
  Impl()
  {
    try
      {
        proxy = Gio::DBus::Proxy::create_for_bus_sync(
#if GLIBMM_CHECK_VERSION(2, 68, 0)
          Gio::DBus::BusType::SESSION,
#else
          Gio::DBus::BUS_TYPE_SESSION,
#endif
          "org.workrave.GnomeShellApplet",
          "/org/workrave/Workrave/Preludes",
          "org.workrave.Preludes");

        if (!proxy)
          {
            throw workrave::utils::Exception("Failed to create D-Bus proxy for prelude window");
          }
        std::string icon = GtkUtil::get_image_filename("prelude-hint.png");
        std::string sad_icon = GtkUtil::get_image_filename("prelude-hint-sad.png");

        Gdk::RGBA color_warn = Gdk::RGBA("orange");
        Gdk::RGBA color_alert = Gdk::RGBA("red");

        GtkUtil::override_color("workrave-flash-warn", "prelude", color_warn);
        GtkUtil::override_color("workrave-flash-alert", "prelude", color_alert);

        callMethod("Init", icon, sad_icon, color_warn.to_string(), color_alert.to_string());
      }
    catch (const Glib::Error &e)
      {
        spdlog::error("Failed to create D-Bus proxy for prelude window: {}", e.what().c_str());
        throw workrave::utils::Exception("Failed to create D-Bus proxy for prelude window");
      }
  }
  ~Impl()
  {
    try
      {
        callMethod("Terminate");
      }
    catch (...)
      {
        spdlog::error("Failed to terminate D-Bus prelude window");
      }
  };

  void callMethod(const std::string &method)
  {
    proxy->call_sync(method, Glib::VariantContainerBase(), -1, Gio::DBus::CALL_FLAGS_NONE);
  }

  template<typename... Args>
  void callMethod(const std::string &method, Args &&...args)
  {
    auto variant = create_variant(std::forward<Args>(args)...);
    proxy->call_sync(method, variant, -1, Gio::DBus::CALL_FLAGS_NONE);
  }

private:
  Glib::VariantContainerBase create_variant()
  {
    return {};
  }
  template<typename... Args>
  Glib::VariantContainerBase create_variant(Args &&...args)
  {
    auto tuple = std::make_tuple(convert_arg(std::forward<Args>(args))...);
    return Glib::Variant<decltype(tuple)>::create(tuple);
  }

  template<typename T>
  auto convert_arg(T &&arg)
  {
    if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
      {
        return Glib::ustring(std::forward<T>(arg));
      }
    else
      {
        return std::forward<T>(arg);
      }
  }

private:
  Glib::RefPtr<Gio::DBus::Proxy> proxy;
};

DBusPreludeWindow::DBusPreludeWindow(BreakId break_id)
  : impl(std::make_unique<Impl>())
  , break_id(break_id)
{
}

DBusPreludeWindow::~DBusPreludeWindow() = default;

void
DBusPreludeWindow::start()
{
  std::string title = get_title(break_id);
  impl->callMethod("Start", title);
}

void
DBusPreludeWindow::stop()
{
  impl->callMethod("Stop");
}

void
DBusPreludeWindow::refresh()
{
  impl->callMethod("Refresh");
}

void
DBusPreludeWindow::set_progress(int value, int max_value)
{
  impl->callMethod("SetProgress", value, max_value);
  auto text = fmt::format(fmt::runtime(progress_text), Text::time_to_string(max_value - value));
  impl->callMethod("SetProgressText", text);
}

void
DBusPreludeWindow::set_stage(workrave::IApp::PreludeStage stage)
{
  impl->callMethod("SetStage", std::string(workrave::utils::enum_to_string(stage)));
}

void
DBusPreludeWindow::set_progress_text(workrave::IApp::PreludeProgressText text)
{
  progress_text = get_progress_text(text);
}

bool
DBusPreludeWindow::is_gnome_shell_applet_available(workrave::dbus::IDBus::Ptr dbus)
{
  if (!dbus || !dbus->is_available())
    {
      return false;
    }

  try
    {
      return dbus->is_running("org.workrave.GnomeShellApplet");
    }
  catch (const workrave::dbus::DBusException &)
    {
      return false;
    }
  return false;
}
