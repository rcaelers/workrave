// AppletControl.hh --- Applet window
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
// $Id$
//

#ifndef APPLETCONTROL_HH
#define APPLETCONTROL_HH

#include "preinclude.h"
#include "ConfiguratorListener.hh"
#include "AppletWindow.hh"

class AppletControl :
  public ConfiguratorListener
{
public:
  enum AppletType
    {
      APPLET_NONE = -1,
      APPLET_TRAY,
      APPLET_GNOME,
      APPLET_KDE,
      APPLET_W32,
      APPLET_SIZE
    };

  AppletControl();
  ~AppletControl();

  void init();
  void show();
  void show(AppletType type);
  void hide();
  void hide(AppletType type);
  bool is_visible(AppletType type);
  bool is_visible();

  // callback from appletwindow
  void set_applet_state(AppletType type, AppletWindow::AppletState);

  void heartbeat();
  void set_timers_tooltip(std::string& tip);
  AppletWindow *get_applet_window(AppletType type);

protected:

private:
  //! All known applets
  AppletWindow *applets[APPLET_SIZE];

  //! Did applet acknowledge visibility?
  bool visible[APPLET_SIZE];

  //!
  bool enabled;

  int delayed_show;

private:
  typedef AppletWindow::AppletState AppletState;

  AppletState activate_applet(AppletType type);
  void deactivate_applet(AppletType type);
  
  void config_changed_notify(std::string key);
  void read_configuration();
  void check_visible();
};

//! Return the specified applet.
inline AppletWindow *
AppletControl::get_applet_window(AppletType type)
{
  return applets[type];
}

#endif // APPLETCONTROL_HH
