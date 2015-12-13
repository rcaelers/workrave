// OSXAppletWindow.hh --- Applet window
//
// Copyright (C) 2009, 2011 Rob Caelers <robc@krandor.nl>
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

#ifndef OSXAPPLETWINDOW_HH
#define OSXAPPLETWINDOW_HH

#include "commonui/preinclude.h"
#include <string>

#include "commonui/TimerBoxViewBase.hh"
#include "commonui/ITimeBar.hh"
#include "AppletWindow.hh"

#import "ColorId.h"

@class OSXStatusBarView;

class OSXAppletWindow : public AppletWindow, public TimerBoxViewBase
{
public:
  OSXAppletWindow();
  virtual ~OSXAppletWindow();

  virtual AppletState activate_applet();
  virtual void deactivate_applet();

  void set_slot(workrave::BreakId  id, int slot);
  void set_time_bar(workrave::BreakId id,
                    std::string text,
                    ITimeBar::ColorId primary_color,
                    int primary_value, int primary_max,
                    ITimeBar::ColorId secondary_color,
                    int secondary_value, int secondary_max);

private:
  ColorId convertColorId(ITimeBar::ColorId colorId);
  OSXStatusBarView *view;
};

#endif // OSXAPPLETWINDOW_HH
