// Copyright (C) 2025 Rob Caelers
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

#include "dbus/DBusException.hh"

using namespace workrave;
using namespace workrave::utils;

class DBusPreludeWindow::Impl
{
public:
  Impl()
  {
    proxy_ = Gio::DBus::Proxy::create_for_bus_sync(
#if GLIBMM_CHECK_VERSION(2, 68, 0)
      Gio::DBus::BusType::SESSION,
#else
      Gio::DBus::BUS_TYPE_SESSION,
#endif
      "org.workrave.Workrave",
      "/org/workrave/Workrave/Prelude",
      "org.workrave.Workrave.IPreludeWindow");
  }

  void callMethod(const std::string &method)
  {
    proxy_->call_sync(method, Glib::VariantContainerBase(), -1, Gio::DBus::CALL_FLAGS_NONE);
  }

  template<typename... Args>
  void callMethod(const std::string &method, Args &&...args)
  {
    auto variant = create_variant(std::forward<Args>(args)...);
    proxy_->call_sync(method, variant, -1, Gio::DBus::CALL_FLAGS_NONE);
  }

private:
  Glib::RefPtr<Gio::DBus::Proxy> proxy_;

  Glib::VariantContainerBase create_variant()
  {
    return {};
  }
  template<typename... Args>
  Glib::VariantContainerBase create_variant(Args &&...args)
  {
    return Glib::Variant<std::tuple<std::decay_t<Args>...>>::create(std::make_tuple(std::forward<Args>(args)...));
  }
};

DBusPreludeWindow::DBusPreludeWindow()
  : impl_(std::make_unique<Impl>())
{
}

DBusPreludeWindow::~DBusPreludeWindow() = default;

void
DBusPreludeWindow::start()
{
  impl_->callMethod("Start");
}

void
DBusPreludeWindow::stop()
{
  impl_->callMethod("Stop");
}

void
DBusPreludeWindow::refresh()
{
  impl_->callMethod("Refresh");
}

void
DBusPreludeWindow::set_progress(int value, int max_value)
{
  impl_->callMethod("SetProgress", value, max_value);
}

void
DBusPreludeWindow::set_stage(workrave::IApp::PreludeStage stage)
{
  impl_->callMethod("SetStage", stage_to_string(stage));
}

void
DBusPreludeWindow::set_progress_text(workrave::IApp::PreludeProgressText text)
{
  impl_->callMethod("SetProgressText", progress_text_to_string(text));
}

std::string
DBusPreludeWindow::stage_to_string(workrave::IApp::PreludeStage stage)
{
  switch (stage)
    {
    case workrave::IApp::STAGE_INITIAL:
      return "initial";
    case workrave::IApp::STAGE_MOVE_OUT:
      return "move-out";
    case workrave::IApp::STAGE_WARN:
      return "warn";
    case workrave::IApp::STAGE_ALERT:
      return "alert";
    default:
      return "unknown";
    }
}

std::string
DBusPreludeWindow::progress_text_to_string(workrave::IApp::PreludeProgressText text)
{
  switch (text)
    {
    case workrave::IApp::PROGRESS_TEXT_BREAK_IN:
      return "break_in";
    case workrave::IApp::PROGRESS_TEXT_DISAPPEARS_IN:
      return "disappears_in";
    case workrave::IApp::PROGRESS_TEXT_SILENT_IN:
      return "silent_in";
    default:
      return "unknown";
    }
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
      return dbus->is_running("org.gnome.Shell");
    }
  catch (const workrave::dbus::DBusException &)
    {
      return false;
    }
}
