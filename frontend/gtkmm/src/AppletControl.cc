// AppletControl.cc --- Applet info Control
//
// Copyright (C) 2006, 2007, 2008, 2009 Rob Caelers & Raymond Penners
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "AppletControl.hh"

#ifdef HAVE_KDE
#include "KdeAppletWindow.hh"
#endif

#ifdef HAVE_GNOMEAPPLET
#include "GnomeAppletWindow.hh"
#endif

#ifdef PLATFORM_OS_UNIX
#include "X11SystrayAppletWindow.hh"
#endif

#ifdef PLATFORM_OS_WIN32
#include "W32AppletWindow.hh"
#endif

#ifdef PLATFORM_OS_OSX
#include "OSXAppletWindow.hh"
#endif

#include "GUI.hh"
#include "MainWindow.hh"
#include "TimerBoxControl.hh"

#include "CoreFactory.hh"
#include "IConfigurator.hh"

using namespace workrave;
using namespace std;

//! Constructor.
AppletControl::AppletControl()
  : enabled(false),
    delayed_show(-1)
{
  for (int i = 0; i < APPLET_SIZE; i++)
    {
      applets[i] = NULL;
      visible[i] = false;
    }
}


//! Destructor
AppletControl::~AppletControl()
{
  for (int i = 0; i < APPLET_SIZE; i++)
    {
      if (applets[i] != NULL)
        {
          deactivate_applet((AppletType)i);
          delete applets[i];
          applets[i] = NULL;
        }
    }
}


//! Initializes the Applet Controller.
void
AppletControl::init()
{
#ifdef HAVE_KDE
  applets[APPLET_KDE] = new KdeAppletWindow(this);
#endif

#ifdef HAVE_GNOMEAPPLET
  applets[APPLET_GNOME] = new GnomeAppletWindow(this);
#endif

#ifdef PLATFORM_OS_UNIX
  applets[APPLET_TRAY] = new X11SystrayAppletWindow(this);
#endif

#ifdef PLATFORM_OS_WIN32
  applets[APPLET_W32] = new W32AppletWindow();
#endif

#ifdef PLATFORM_OS_OSX
  applets[APPLET_OSX] = new OSXAppletWindow();
#endif

  // Read configuration and start monitoring it.
  IConfigurator *config = CoreFactory::get_configurator();
  config->add_listener(TimerBoxControl::CFG_KEY_TIMERBOX + "applet", this);

  read_configuration();
}


//! Show the best possible applet.
void
AppletControl::show()
{
  TRACE_ENTER("AppletControl::show");
  
  bool specific = false;
  AppletState rc;

  rc = activate_applet(APPLET_GNOME);
  TRACE_MSG("Gnome " << rc);
  if (rc != AppletWindow::APPLET_STATE_DISABLED)
    {
      // Applet now visible or pending.
      // Don't try to activate generic applet (systray)
      specific = true;
    }

  rc = activate_applet(APPLET_KDE);
  TRACE_MSG("kde " << rc);
  if (rc != AppletWindow::APPLET_STATE_DISABLED)
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
      // Applet now visible or pending.
      // Don't try to activate generic applet (systray)
      specific = true;
    }

#ifdef PLATFORM_OS_UNIX
  if (applets[APPLET_TRAY] != NULL)
    {
      if ((type == APPLET_KDE || type == APPLET_GNOME)
          && specific)

        {
          deactivate_applet(APPLET_TRAY);
        }
      else
        {
          activate_applet(APPLET_TRAY);
        }
    }
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


//! Hide a specific applet.
void
AppletControl::hide(AppletType type)
{
  deactivate_applet(type);
}


//! The specified applet is not active.
void
AppletControl::set_applet_state(AppletType type, AppletWindow::AppletState state)
{
  TRACE_ENTER_MSG("AppletControl::set_applet_state", type << " " << state);
  switch (state)
    {
    case AppletWindow::APPLET_STATE_DISABLED:
      visible[type] = false;
      break;

    case AppletWindow::APPLET_STATE_VISIBLE:
      visible[type] = true;
      break;

    case AppletWindow::APPLET_STATE_PENDING:
      visible[type] = false;
      break;
    }

#ifdef PLATFORM_OS_UNIX
  if (visible[type] &&
      (type == APPLET_KDE || type == APPLET_GNOME))
    {
      deactivate_applet(APPLET_TRAY);
    }
#endif

  if (enabled && !is_visible())
    {
      delayed_show = 5;
    }

  check_visible();
  TRACE_EXIT();
}


//! Is at least a single applet visible.
bool
AppletControl::is_visible(AppletType type)
{
  return visible[type];
}


//! Is the specified applet visible.
bool
AppletControl::is_visible()
{
  bool ret = false;

    for (int i = 0; i < APPLET_SIZE; i++)
    {
      if (visible[i])
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
  if (delayed_show < 0 && enabled && !is_visible())
    {
      delayed_show = 60;
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

  for (int i = 0; i < APPLET_SIZE; i++)
    {
      if (applets[i] != NULL && visible[i])
        {
          applets[i]->update_applet();
        }
    }
  TRACE_EXIT();
}


//! Sets the tooltip of all visible applets.
void
AppletControl::set_timers_tooltip(std::string& tip)
{
  for (int i = 0; i < APPLET_SIZE; i++)
    {
      if (applets[i] != NULL && visible[i])
        {
          applets[i]->set_timers_tooltip(tip);
        }
    }
}


//! Reads the applet configuration.
void
AppletControl::read_configuration()
{
  bool previous_enabled = enabled;
  enabled = TimerBoxControl::is_enabled("applet");

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


//! Callback that the configuration has changed.
void
AppletControl::config_changed_notify(const string &key)
{
  TRACE_ENTER_MSG("AppletControl::config_changed_notify", key);
  (void) key;
  read_configuration();
  TRACE_EXIT();
}


//! Make sure the main window is visible when no applets are.
void
AppletControl::check_visible()
{
  TRACE_ENTER("AppletControl::check_visible");
  int count = 0;

  for (int i = 0; i < APPLET_SIZE; i++)
    {
      if (visible[i])
        {
          count++;
        }
    }

#ifdef PLATFORM_OS_OSX
  count++;
#endif
    
  GUI *gui = GUI::get_instance();
  MainWindow *main = gui->get_main_window();
  if (main != NULL)
    {
      main->set_applet_active( count > 0 );
    }

  TRACE_EXIT();
}

AppletWindow::AppletState
AppletControl::activate_applet(AppletType type)
{
  AppletState r = AppletWindow::APPLET_STATE_DISABLED;

  if (applets[type] != NULL)
    {
      r = applets[type]->activate_applet();
      if (r == AppletWindow::APPLET_STATE_VISIBLE)
        {
          visible[type] = true;
        }
    }

  return r;
}

void
AppletControl::deactivate_applet(AppletType type)
{
 if (applets[type] != NULL)
    {
      applets[type]->deactivate_applet();
      visible[type] = false;
    }
}
