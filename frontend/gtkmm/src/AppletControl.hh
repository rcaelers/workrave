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

class AppletWindow;

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
  void activated(AppletType type);
  void deactivated(AppletType type);
  bool is_visible(AppletType type);
  bool is_visible();

  void heartbeat();
  void set_timers_tooltip(std::string& tip);
  AppletWindow *get_applet_window(AppletType type);
  
private:
  //! All known applets
  AppletWindow *applets[APPLET_SIZE];

  //! Did applet acknowledge visibility?
  bool visible[APPLET_SIZE];

  //!
  bool enabled;

  int delayed_show;
  
private:
  void config_changed_notify(std::string key);
  void read_configuration();
  void check_visible();
};

inline AppletWindow *
AppletControl::get_applet_window(AppletType type)
{
  return applets[type];
}

#endif // APPLETCONTROL_HH
