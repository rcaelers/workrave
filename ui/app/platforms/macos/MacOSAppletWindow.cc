// Copyright (C) 2009, 2011, 2013 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "commonui/nls.h"
//#include "commonui/Text.hh"
#include "debug.hh"

#include "MacOSAppletWindow.hh"
#include "TimerBoxControl.hh"

#import "MacOSStatusBarView.h"

MacOSAppletWindow::MacOSAppletWindow(std::shared_ptr<IApplicationContext> app)
  : app(app)
{
  TRACE_ENTRY();
  timer_box_view = this;
  timer_box_control = new TimerBoxControl(app, "applet", *this);

  NSMenu *menu = [[NSMenu alloc] init];
  view = [[MacOSStatusBarView alloc] initWithMenu:menu];

  while ([menu numberOfItems] > 0)
    {
      [menu removeItemAtIndex:0];
    }
  NSMenuItem *item;

  item = [[[NSMenuItem alloc] initWithTitle:@"Hello" action:nil keyEquivalent:@""] autorelease];
  // [item setTarget:self];
  [item setEnabled:YES];
  [menu addItem:item];
  [menu addItem:[NSMenuItem separatorItem]];

  [menu update];
}

MacOSAppletWindow::~MacOSAppletWindow()
{
  TRACE_ENTRY();
  delete timer_box_control;
}

void
MacOSAppletWindow::set_slot(workrave::BreakId id, int slot)
{
  TRACE_ENTRY_PAR(id, slot);
}

void
MacOSAppletWindow::set_time_bar(workrave::BreakId id,
                                int value,
                                TimerColorId primary_color,
                                int primary_val,
                                int primary_max,
                                TimerColorId secondary_color,
                                int secondary_val,
                                int secondary_max)
{
  TRACE_ENTRY_PAR(int(id), value);

  NSString *bar_text = [NSString stringWithCString:Text::time_to_string(value) encoding:NSASCIIStringEncoding];

  [view setBreak:id
                 text:bar_text
         primaryColor:convertColorId(primary_color)
         primaryValue:primary_val
      primaryMaxValue:primary_max
       secondaryColor:convertColorId(secondary_color)
       secondaryValue:secondary_val
    secondaryMaxValue:secondary_max];
}

ColorId
MacOSAppletWindow::convertColorId(TimerColorId colorId)
{
  switch (colorId)
    {
    case TimerColorId::Inactive:
      return COLOR_ID_INACTIVE;
    case TimerColorId::Overdue:
      return COLOR_ID_OVERDUE;
    case TimerColorId::Active:
      return COLOR_ID_ACTIVE;
    default:
      return COLOR_ID_ACTIVE;
    }

  return COLOR_ID_INACTIVE;
}

bool
MacOSAppletWindow::is_visible() const
{
  return true;
}
