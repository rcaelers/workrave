// UnityAppletWindow.hh --- X11 Applet Window
//
// Copyright (C) 2001 - 2009, 2011 Rob Caelers & Raymond Penners
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

#ifndef UNITYAPPLETWINDOW_HH
#define UNITYAPPLETWINDOW_HH

#include "preinclude.h"
#include <stdio.h>

#include "ITimerBoxView.hh"
#include "ITimeBar.hh"
#include "AppletWindow.hh"

#include <string>

struct TimerData
{
  std::string bar_text;
  int slot;
  int bar_secondary_color;
  int bar_secondary_val;
  int bar_secondary_max;
  int bar_primary_color;
  int bar_primary_val;
  int bar_primary_max;
};

class AppletControl;

class UnityAppletWindow : public AppletWindow , ITimerBoxView
{
public:
  UnityAppletWindow(AppletControl *control);
  virtual ~UnityAppletWindow();

  virtual AppletState activate_applet();
  virtual void deactivate_applet();

  virtual void set_applet_enabled(bool enable);
  
  virtual void set_slot(BreakId  id, int slot);
  virtual void set_time_bar(BreakId id,
                            std::string text,
                            ITimeBar::ColorId primary_color,
                            int primary_value, int primary_max,
                            ITimeBar::ColorId secondary_color,
                            int secondary_value, int secondary_max);
  virtual void set_tip(std::string tip);
  virtual void set_icon(IconType icon);
  virtual void update_view();
  virtual void set_enabled(bool enabled);
  virtual void set_geometry(Orientation orientation, int size);

private:
  bool enabled;
  TimerData data[BREAK_ID_SIZEOF];

  //! Controller
  AppletControl *control;
};

#endif // UNITYAPPLETWINDOW_HH
