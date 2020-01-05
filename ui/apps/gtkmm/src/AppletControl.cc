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
#include "config.h"
#endif

#include "debug.hh"

#include "AppletControl.hh"

#ifdef HAVE_PANELAPPLET4
#include "GnomeAppletWindow.hh"
#endif

#ifdef PLATFORM_OS_UNIX
#include "X11SystrayAppletWindow.hh"
#endif

#ifdef PLATFORM_OS_WINDOWS
#include "W32AppletWindow.hh"
#endif

#ifdef HAVE_DBUS
#include "GenericDBusApplet.hh"
#endif

#ifdef PLATFORM_OS_OSX
#include "OSXAppletWindow.hh"
#endif

#include "GUI.hh"
#include "commonui/TimerBoxControl.hh"
#include "Menus.hh"

#include "commonui/Backend.hh"
#include "config/IConfigurator.hh"

using namespace workrave;
using namespace workrave::config;
using namespace std;

//! Constructor.
AppletControl::AppletControl()
  : visible(false),
    enabled(false),
    delayed_show(-1)
{
  for (int i = 0; i < APPLET_SIZE; i++)
    {
      applets[i] = nullptr;
      applet_state[i] = AppletWindow::APPLET_STATE_DISABLED;
    }
}


//! Destructor
AppletControl::~AppletControl()
{
  for (int i = 0; i < APPLET_SIZE; i++)
    {
      if (applets[i] != nullptr)
        {
          deactivate_applet((AppletType)i);
          delete applets[i];
          applets[i] = nullptr;
        }
    }
}


//! Initializes the Applet Controller.
void
AppletControl::init()
{
#ifdef HAVE_DBUS_GIO
  applets[APPLET_GENERIC_DBUS] = new GenericDBusApplet();
#endif

#ifdef PLATFORM_OS_UNIX
  applets[APPLET_TRAY] = new X11SystrayAppletWindow();
#endif

#ifdef PLATFORM_OS_WINDOWS
  applets[APPLET_W32] = new W32AppletWindow();
#endif

#ifdef PLATFORM_OS_OSX
  applets[APPLET_OSX] = new OSXAppletWindow();
#endif

  for (int i = 0; i < APPLET_SIZE; i++)
    {
      if (applets[i] != nullptr)
        {
          applets[i]->init_applet();
          applets[i]->signal_state_changed().connect(sigc::bind<0>(sigc::mem_fun(*this, &AppletControl::on_applet_state_changed), (AppletType)i));
          applets[i]->signal_request_activate().connect(sigc::bind<0>(sigc::mem_fun(*this, &AppletControl::on_applet_request_activate), (AppletType)i));
        }
    }

  // Read configuration and start monitoring it.
  connections.add(GUIConfig::key_timerbox("applet").connect(std::bind(&AppletControl::read_configuration, this)));

  read_configuration();
}


//! Show the best possible applet.
void
AppletControl::show()
{
  TRACE_ENTER("AppletControl::show");

  bool specific = false;
  AppletState rc;

  rc = activate_applet(APPLET_GENERIC_DBUS);
  TRACE_MSG("Generic" << rc);
  if (rc == AppletWindow::APPLET_STATE_ACTIVE)
    {
      specific = true;
    }

  rc = activate_applet(APPLET_W32);
  TRACE_MSG("Win32 " << rc);
  if (rc != AppletWindow::APPLET_STATE_DISABLED)
    {
      specific = true;
    }

  rc = activate_applet(APPLET_OSX);
  TRACE_MSG("OSX " << rc);
  if (rc != AppletWindow::APPLET_STATE_DISABLED)
    {
      specific = true;
    }

#ifdef PLATFORM_OS_UNIX
  if (specific)
    {
      deactivate_applet(APPLET_TRAY);
      TRACE_MSG("X11 deact ");
    }
  else
    {
      rc = activate_applet(APPLET_TRAY);
      TRACE_MSG("X11 act " << rc);
    }
#else
  (void) specific;
#endif
  check_visible();

  TRACE_EXIT();
}


//! Show the specified applet.
void
AppletControl::show(AppletType type)
{
  bool specific = false;

  AppletState rc = activate_applet(type);
  if (rc != AppletWindow::APPLET_STATE_DISABLED)
    {
      // Applet now active or pending.
      // Don't try to activate generic applet (systray)
      specific = true;
    }

#ifdef PLATFORM_OS_UNIX
  if (applets[APPLET_TRAY] != nullptr)
    {
      if (specific)
        {
          deactivate_applet(APPLET_TRAY);
        }
      else
        {
          activate_applet(APPLET_TRAY);
        }
    }
#else
  (void) specific;
#endif

  check_visible();
}


//! Hide all applets.
void
AppletControl::hide()
{
  for (int i = 0; i < APPLET_SIZE; i++)
    {
      deactivate_applet((AppletType)i);
    }
}


//! The specified applet is not active.
void
AppletControl::on_applet_state_changed(AppletType type, AppletWindow::AppletState state)
{
  TRACE_ENTER_MSG("AppletControl::on_applet_state_changed", type << " " << state);
  applet_state[type] = state;

#ifdef PLATFORM_OS_UNIX
  if (is_active(type) && (type == APPLET_GENERIC_DBUS))
    {
      TRACE_MSG("Deactivate tray");
      deactivate_applet(APPLET_TRAY);
      delayed_show = -1;
    }
#endif

  if (enabled && !is_active())
    {
      TRACE_MSG("none active, show in 5s");
      delayed_show = 5;
    }

  // REFACTOR
  if (state == AppletWindow::APPLET_STATE_ACTIVE)
    {
      IGUI *gui = GUI::get_instance();
      Menus *menus = gui->get_menus();
      menus->resync();
    }

  check_visible();
  TRACE_EXIT();
}


void
AppletControl::on_applet_request_activate(AppletType type)
{
  show(type);
}

//! Is the specified applet active.
bool
AppletControl::is_active(AppletType type)
{
  return
    applet_state[type] == AppletWindow::APPLET_STATE_ACTIVE;
}


//! Is the specified applet visible.
bool
AppletControl::is_visible(AppletType type)
{
  return
    applet_state[type] == AppletWindow::APPLET_STATE_ACTIVE ||
    applet_state[type] == AppletWindow::APPLET_STATE_VISIBLE;
}


//! Is at least a single applet active.
bool
AppletControl::is_active()
{
  bool ret = false;

  for (int i = APPLET_FIRST; i < APPLET_SIZE; i++)
    {
      if (is_active((AppletType)i))
        {
          ret = true;
        }
    }
    return ret;
}


//! Is at least a single applet visible.
bool
AppletControl::is_visible()
{
  bool ret = false;

  for (int i = APPLET_FIRST; i < APPLET_SIZE; i++)
    {
      if (is_visible((AppletType)i))
        {
          ret = true;
        }
    }
    return ret;
}


//! Periodic heartbeat.
void
AppletControl::heartbeat()
{
  TRACE_ENTER("AppletControl::heartbeat");
  if (delayed_show < 0 && enabled && !is_active())
    {
      delayed_show = 30;
    }
  else if (delayed_show > 0)
    {
      delayed_show--;
    }
  else if (delayed_show == 0)
    {
      delayed_show = -1;
      show();
    }

  for (int i = APPLET_FIRST; i < APPLET_SIZE; i++)
    {
      if (applets[i] != nullptr && is_active((AppletType)i))
        {
          applets[i]->update_applet();
        }
    }
  TRACE_EXIT();
}


//! Sets the tooltip of all visible applets.
void
AppletControl::set_tooltip(std::string& tip)
{
  for (int i = APPLET_FIRST; i < APPLET_SIZE; i++)
    {
      if (applets[i] != nullptr && is_visible((AppletType)i))
        {
          applets[i]->set_applet_tooltip(tip);
        }
    }
}


//! Reads the applet configuration.
void
AppletControl::read_configuration()
{
  bool previous_enabled = enabled;
  enabled = GUIConfig::timerbox_enabled("applet")();

  if (!enabled)
    {
      delayed_show = -1;
    }

  if (!previous_enabled && enabled)
    {
      show();
    }
  else if (previous_enabled && !enabled)
    {
      hide();
    }
}


//! Make sure the main window is visible when no applets are.
void
AppletControl::check_visible()
{
  TRACE_ENTER("AppletControl::check_visible");
  int count = 0;

  for (int i = APPLET_FIRST; i < APPLET_SIZE; i++)
    {
      if (is_visible((AppletType)i))
        {
          TRACE_MSG(i << " is visible");
          count++;
        }
    }

#ifdef PLATFORM_OS_OSX
  count++;
#endif

  bool new_visible = count > 0;
  if (new_visible != visible)
    {
      visible = new_visible;
      visibility_changed_signal.emit();
    }

  TRACE_EXIT();
}

AppletWindow::AppletState
AppletControl::activate_applet(AppletType type)
{
  AppletState state = AppletWindow::APPLET_STATE_DISABLED;

  if (applets[type] != nullptr)
    {
      state = applets[type]->activate_applet();
      applet_state[type] = state;
    }

  return state;
}


void
AppletControl::deactivate_applet(AppletType type)
{
 if (applets[type] != nullptr)
    {
      applets[type]->deactivate_applet();
      applet_state[type] = AppletWindow::APPLET_STATE_DISABLED;
    }
}


sigc::signal<void> &
AppletControl::signal_visibility_changed()
{
  return visibility_changed_signal;
}
