// AppletControl.cc --- Applet info Control
//
// Copyright (C) 2006 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

static const char rcsid[] = "$Id$";

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

#ifdef HAVE_GNOME
#include "GnomeAppletWindow.hh"
#endif

#ifdef HAVE_X
#include "X11SystrayAppletWindow.hh"
#endif

#ifdef WIN32
#include "W32AppletWindow.hh"
#endif

#include "GUI.hh"
#include "MainWindow.hh"
#include "TimerBoxControl.hh"

#include "CoreFactory.hh"
#include "ConfiguratorInterface.hh"

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


AppletControl::~AppletControl()
{
  for (int i = 0; i < APPLET_SIZE; i++)
    {
      if (applets[i] != NULL)
        {
          applets[i]->deactivate_applet();
          applets[i]->cleanup_applet();
          delete applets[i];
          applets[i] = NULL;
        }
    }
}

void
AppletControl::init()
{
#ifdef HAVE_KDE
  applets[APPLET_KDE] = new KdeAppletWindow(this);
#endif

#ifdef HAVE_GNOME
  applets[APPLET_GNOME] = new GnomeAppletWindow(this);
#endif

#ifdef HAVE_X
  applets[APPLET_TRAY] = new X11SystrayAppletWindow(this);
#endif

#ifdef WIN32  
  applets[APPLET_W32] = new W32AppletWindow();
#endif
  
  for (int i = 0; i < APPLET_SIZE; i++)
    {
      if (applets[i] != NULL)
        {
          applets[i]->init_applet();
        }
    }
  
  // Read configuration and start monitoring it.
  ConfiguratorInterface *config = CoreFactory::get_configurator();
  config->add_listener(TimerBoxControl::CFG_KEY_TIMERBOX + "applet", this);

  read_configuration();
}


void
AppletControl::show()
{
  bool specific = false;
      
  if (applets[APPLET_GNOME] != NULL)
    {
      bool rc = applets[APPLET_GNOME]->activate_applet();
      if (rc)
        {
          specific = true;
        }
    }

  if (applets[APPLET_KDE] != NULL)
    {
      bool rc = applets[APPLET_KDE]->activate_applet();
      if (rc)
        {
          specific = true;
        }
    }

  if (applets[APPLET_W32] != NULL)
    {
      bool rc = applets[APPLET_W32]->activate_applet();
      if (rc)
        {
          specific = true;
        }
    }
  
#ifdef HAVE_X
  if (applets[APPLET_TRAY] != NULL)
    {
      if (specific)
        {
          applets[APPLET_TRAY]->deactivate_applet();
        }
      else
        {
          applets[APPLET_TRAY]->activate_applet();
        }
    }
#endif
  check_visible();
}


void
AppletControl::show(AppletType type)
{
  bool specific = false;
      
  if (applets[type] != NULL)
    {
      bool rc = applets[type]->activate_applet();
      if (rc)
        {
          specific = true;
        }
    }

#ifdef HAVE_X
  if (applets[APPLET_TRAY] != NULL)
    {
      if ((type == APPLET_KDE || type == APPLET_GNOME)
          && specific)
        
        {
          applets[APPLET_TRAY]->deactivate_applet();
        }
      else
        {
          applets[APPLET_TRAY]->activate_applet();
        }
    }
#endif
  
  check_visible();
}


void
AppletControl::hide()
{
  for (int i = 0; i < APPLET_SIZE; i++)
    {
      if (applets[i] != NULL)
        {
          applets[i]->deactivate_applet();
        }
    }
}

void
AppletControl::hide(AppletType type)
{
  if (applets[type] != NULL)
    {
      applets[type]->deactivate_applet();
    }
}

//! Request to activate specific applet
void
AppletControl::activated(AppletType type)
{
  visible[type] = true;

#ifdef HAVE_X
  if (applets[APPLET_TRAY] != NULL)
    {
      if (type == APPLET_KDE || type == APPLET_GNOME)
        {
          applets[APPLET_TRAY]->deactivate_applet();
        }
    }
#endif

  check_visible();
}


//! Request to activate specific applet
void
AppletControl::deactivated(AppletType type)
{
  visible[type] = false;

  if (!is_visible())
    {
      delayed_show = 5;
    }
  
  check_visible();
}


bool
AppletControl::is_visible(AppletType type)
{
  return visible[type];
}

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

void
AppletControl::heartbeat()
{
  if (delayed_show > 0)
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
}

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
AppletControl::config_changed_notify(string key)
{
  TRACE_ENTER_MSG("AppletControl::config_changed_notify", key);
  (void) key;
  read_configuration();
  TRACE_EXIT();
}

//! 
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

  GUI *gui = GUI::get_instance();
  MainWindow *main = gui->get_main_window();
  if (main != NULL)
    {
      main->set_applet_active( count > 0 );
    }

  TRACE_EXIT();
}
