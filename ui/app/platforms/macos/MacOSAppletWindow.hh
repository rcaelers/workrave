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

#ifndef MACOSAPPLETWINDOW_HH
#define MACOSAPPLETWINDOW_HH

#include <string>

#include "ui/ITimerBoxView.hh"
#include "ui/UiTypes.hh"
#include "ui/IApplicationContext.hh"
#include "AppletWindow.hh"

#import "ColorId.h"

@class MacOSStatusBarView;

class MacOSAppletWindow
  : public AppletWindow
  , public ITimerBoxView
{
public:
  MacOSAppletWindow(std::shared_ptr<IApplicationContext> app);
  virtual ~MacOSAppletWindow();

  bool is_visible() const override;

  void set_slot(workrave::BreakId id, int slot) override;
  void set_time_bar(workrave::BreakId id,
                    int value,
                    TimerColorId primary_color,
                    int primary_value,
                    int primary_max,
                    TimerColorId secondary_color,
                    int secondary_value,
                    int secondary_max) override;

private:
  std::shared_ptr<IApplicationContext> app;
  ColorId convertColorId(TimerColorId colorId);
  MacOSStatusBarView *view;
};

#endif // MACOSAPPLETWINDOW_HH
