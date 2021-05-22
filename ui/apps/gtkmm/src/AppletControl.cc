// AppletControl.cc --- Applet info Control
//
// Copyright (C) 2006, 2007, 2008, 2009, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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

#include <type_traits>

#include "debug.hh"

#include "AppletControl.hh"

#ifdef PLATFORM_OS_UNIX
#  include "X11SystrayAppletWindow.hh"
#endif

#ifdef PLATFORM_OS_WINDOWS
#  include "W32AppletWindow.hh"
#endif

#ifdef HAVE_DBUS
#  include "GenericDBusApplet.hh"
#endif

#ifdef PLATFORM_OS_MACOS
#  include "MacOSAppletWindow.hh"
#endif

#include "GUI.hh"
#include "commonui/TimerBoxControl.hh"
#include "Menus.hh" // REFACTOR

using namespace workrave;

void
AppletControl::init()
{
#ifdef PLATFORM_OS_UNIX
  applets[AppletType::Tray] = std::make_shared<X11SystrayAppletWindow>();
#endif

#if defined(PLATFORM_OS_WINDOWS)
  applets[AppletType::Windows] = std::make_shared<W32AppletWindow>();
#elif defined(PLATFORM_OS_MACOS)
  applets[AppletType::MacOS] = std::make_shared<MacOSAppletWindow>();
#elif defined(HAVE_DBUS)
  applets[AppletType::GenericDBus] = std::make_shared<GenericDBusApplet>();
#endif

  for (const auto &kv: applets)
    {
      kv.second->init_applet();
      kv.second->signal_visibility_changed().connect(
        sigc::bind<0>(sigc::mem_fun(*this, &AppletControl::on_applet_visibility_changed), kv.first));
    }
  check_visible();
}

void
AppletControl::on_applet_visibility_changed(AppletType type, bool visible)
{
  TRACE_ENTER_MSG("AppletControl::on_applet_visibility_changed",
                  static_cast<std::underlying_type<AppletType>::type>(type) << " " << visible);

  // TODO: REFACTOR
  if (visible)
    {
      IGUI *gui = GUI::get_instance();
      Menus *menus = gui->get_menus();
      menus->resync();
    }

  check_visible();
  TRACE_EXIT();
}

void
AppletControl::heartbeat()
{
  TRACE_ENTER("AppletControl::heartbeat");
  for (const auto &kv: applets)
    {
      kv.second->update_applet();
    }
  TRACE_EXIT();
}

void
AppletControl::set_tooltip(std::string &tip)
{
  for (const auto &kv: applets)
    {
      kv.second->set_applet_tooltip(tip);
    }
}

bool
AppletControl::is_visible() const
{
  TRACE_ENTER("AppletControl::is_visible");
  int count = 0;

  for (const auto &kv: applets)
    {
      if (kv.second->is_visible())
        {
          TRACE_MSG(static_cast<std::underlying_type<AppletType>::type>(kv.first) << " is visible");
          count++;
        }
    }
  TRACE_RETURN(count);
  return count > 0;
}

void
AppletControl::check_visible()
{
  TRACE_ENTER("AppletControl::check_visible");
  bool new_visible = is_visible();
  if (new_visible != visible)
    {
      visible = new_visible;
      visibility_changed_signal.emit();
    }
  TRACE_EXIT();
}

std::shared_ptr<IAppletWindow>
AppletControl::get_applet_window(AppletType type)
{
  return applets[type];
}

sigc::signal<void> &
AppletControl::signal_visibility_changed()
{
  return visibility_changed_signal;
}
