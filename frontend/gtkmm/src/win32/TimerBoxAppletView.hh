// TimerBoxtGtkView.hh --- All timers
//
// Copyright (C) 2001, 2002, 2003, 2004 Rob Caelers & Raymond Penners
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

#ifndef TIMERBOXAPPLETVIEW_HH
#define TIMERBOXAPPLETVIEW_HH

#include <windows.h>
#include <string>

#include "TimerBoxView.hh"
#include "TimeBarInterface.hh"
#include "Applet.hh"

class TimerBoxAppletView : public TimerBoxView
{
public:  
  TimerBoxAppletView();
  virtual ~TimerBoxAppletView();

  void set_slot(BreakId  id, int slot);
  void set_time_bar(BreakId id,
                    std::string text,
                    TimeBarInterface::ColorId primary_color,
                    int primary_value, int primary_max,
                    TimeBarInterface::ColorId secondary_color,
                    int secondary_value, int secondary_max);
  void set_tip(std::string tip);
  void set_icon(IconType icon);
  void update();
  void update_time_bars();
  void update_menu();
  void set_enabled(bool enabled);

  void init_menu(HWND dest);
  void add_menu(const char *text, short cmd, int flags);

  enum MenuFlag
  {
    MENU_FLAG_TOGGLE = APPLET_MENU_FLAG_TOGGLE,
    MENU_FLAG_SELECTED = APPLET_MENU_FLAG_SELECTED,
    MENU_FLAG_POPUP = APPLET_MENU_FLAG_POPUP
  };
  
private:
  HWND get_applet_window();

  HWND applet_window;
  bool menu_sent;
  AppletHeartbeatData heartbeat_data;
  AppletMenuData menu_data;
};



#endif // TIMERBOXAPPLETVIEW_HH
