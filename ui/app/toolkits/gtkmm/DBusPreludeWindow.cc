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

#include <giomm.h>

#include "GtkUtil.hh"
#include "utils/Exception.hh"
#include "utils/Enum.hh"

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
          "org.workrave.Workrave",
          "/org/workrave/Workrave/Prelude",
          "org.workrave.Workrave.IPreludeWindow");
      }
    catch (const Glib::Error &e)
      {
        throw workrave::utils::Exception("Failed to create D-Bus proxy for prelude window: " + std::string(e.what()));
      }
  }

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
    return Glib::Variant<std::tuple<std::decay_t<Args>...>>::create(std::make_tuple(std::forward<Args>(args)...));
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
  std::string icon = GtkUtil::get_image_filename("prelude-hint.png");
  std::string sad_icon = GtkUtil::get_image_filename("prelude-hint-sad.png");

  Gdk::RGBA color_warn = Gdk::RGBA("orange");
  Gdk::RGBA color_alert = Gdk::RGBA("red");

  GtkUtil::override_color("workrave-flash-warn", "prelude", color_warn);
  GtkUtil::override_color("workrave-flash-alert", "prelude", color_alert);

  impl->callMethod("Init", icon, sad_icon, color_warn.to_string(), color_alert.to_string());

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
}

void
DBusPreludeWindow::set_stage(workrave::IApp::PreludeStage stage)
{
  impl->callMethod("SetStage", stage_to_string(stage));
}

void
DBusPreludeWindow::set_progress_text(workrave::IApp::PreludeProgressText text)
{
  impl->callMethod("SetProgressText", progress_text_to_string(text));
}

std::string
DBusPreludeWindow::stage_to_string(workrave::IApp::PreludeStage stage)
{
  return std::string(workrave::utils::enum_to_string(stage));
}

std::string
DBusPreludeWindow::progress_text_to_string(workrave::IApp::PreludeProgressText text)
{
  return get_progress_text(text);
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
      return dbus->is_running("org.gnome.GnomeShellApplet");
    }
  catch (const workrave::dbus::DBusException &)
    {
      return false;
    }
}
