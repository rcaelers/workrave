// OSXAppletWindow.cc --- Applet info Window
//
// Copyright (C) 2009 Rob Caelers <robc@krandor.nl>
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "OSXAppletWindow.hh"
#include "TimerBoxControl.hh"
#include "GUI.hh"
#include "Menus.hh"

#import "OSXStatusBarView.h"

OSXAppletWindow::OSXAppletWindow()
{
  TRACE_ENTER("OSXAppletWindow::OSXAppletWindow");

  timer_box_view = this;
  timer_box_control = new TimerBoxControl("applet", *this);

  NSMenu *menu = [[NSMenu alloc] init];
  view = [[OSXStatusBarView alloc] initWithMenu:menu];


  int i;
  while ([menu numberOfItems] > 0) {
    [menu removeItemAtIndex:0];
  }
  NSMenuItem *item;

  item = [[[NSMenuItem alloc] initWithTitle:@"Hello"
                              action:nil keyEquivalent:@""] autorelease];
  // [item setTarget:self];
  [item setEnabled:YES];
  [menu addItem:item];
  [menu addItem:[NSMenuItem separatorItem]];

  [menu update];

  TRACE_EXIT();
}

OSXAppletWindow::~OSXAppletWindow()
{
  TRACE_ENTER("OSXAppletWindow::~OSXAppletWindow");
  delete timer_box_control;
  TRACE_EXIT();
}


void
OSXAppletWindow::set_slot(BreakId id, int slot)
{
  TRACE_ENTER_MSG("OSXAppletWindow::set_slot", int(id) << ", " << slot);
  TRACE_EXIT();
}


void
OSXAppletWindow::set_time_bar(BreakId id,
                              std::string text,
                              ITimeBar::ColorId primary_color,
                              int primary_val, int primary_max,
                              ITimeBar::ColorId secondary_color,
                              int secondary_val, int secondary_max)
{
  TRACE_ENTER_MSG("OSXAppletWindow::set_time_bar", int(id) << "=" << text);

  NSString *bar_text = [NSString stringWithCString: text.c_str() encoding: NSASCIIStringEncoding];

  [view setBreak: id
        text: bar_text
        primaryColor: convertColorId(primary_color)
        primaryValue: primary_val
        primaryMaxValue: primary_max
        secondaryColor: convertColorId(secondary_color)
        secondaryValue: secondary_val
        secondaryMaxValue: secondary_max ];

  TRACE_EXIT();
}

ColorId
OSXAppletWindow::convertColorId(ITimeBar::ColorId colorId)
{
  switch (colorId)
    {
    case ITimeBar::COLOR_ID_INACTIVE:
      return COLOR_ID_INACTIVE;
    case ITimeBar::COLOR_ID_OVERDUE:
      return COLOR_ID_OVERDUE;
    case ITimeBar::COLOR_ID_ACTIVE:
      return COLOR_ID_ACTIVE;
    default:
      return COLOR_ID_ACTIVE;
    }

  return COLOR_ID_INACTIVE;
}

void
OSXAppletWindow::set_tip(std::string tip)
{
  (void) tip;
}


void
OSXAppletWindow::set_icon(IconType type)
{
  (void) type;
}


void
OSXAppletWindow::update_view()
{
  TRACE_ENTER("OSXAppletWindow::update_view");
  TRACE_EXIT();
}


void
OSXAppletWindow::set_enabled(bool enabled)
{
  TRACE_ENTER_MSG("OSXAppletWindow::set_enabled", enabled);
  TRACE_EXIT();
}


AppletWindow::AppletState
OSXAppletWindow::activate_applet()
{
  return APPLET_STATE_VISIBLE;
}


void
OSXAppletWindow::deactivate_applet()
{
}


void
OSXAppletWindow::set_geometry(Orientation orientation, int size)
{
  (void) orientation;
  (void) size;
}

