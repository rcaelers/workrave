// AppletWindow.hh --- Applet window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Rob Caelers & Raymond Penners
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

#ifndef APPLETWINDOW_HH
#define APPLETWINDOW_HH

#include "preinclude.h"

class TimerBoxControl;
class TimerBoxView;

class AppletWindow 
{
public:  
  enum AppletMode
    {
      APPLET_DISABLED,
      APPLET_TRAY,
      APPLET_GNOME,
      APPLET_KDE,
      APPLET_W32
    };

  AppletWindow();
  virtual ~AppletWindow();

  virtual bool activate_applet() = 0;
  virtual void deactivate_applet() = 0;
  
  virtual void update_applet();

  virtual void set_timers_tooltip(std::string& tip);
  
protected:
  //! Box container all the timers.
  TimerBoxView *timer_box_view;
  
  //! Box container controller.
  TimerBoxControl *timer_box_control;

};

#endif // APPLETWINDOW_HH
