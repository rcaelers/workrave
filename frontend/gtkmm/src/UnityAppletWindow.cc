// UnityAppletWindow.cc --- Applet info Window
//
// Copyright (C) 2001 - 2011 Rob Caelers & Raymond Penners
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

using namespace std;

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <gtkmm.h>

#include "UnityAppletWindow.hh"

#include "TimerBoxControl.hh"
#include "GUI.hh"
#include "Menus.hh"

#include "ICore.hh"
#include "CoreFactory.hh"

#include "DBus.hh"
#include "DBusException.hh"
#include "DBusGUI.hh"

//! Constructor.
UnityAppletWindow::UnityAppletWindow()
{
}


//! Destructor.
UnityAppletWindow::~UnityAppletWindow()
{
}

void
UnityAppletWindow::set_slot(BreakId id, int slot)
{
  TRACE_ENTER_MSG("UnityAppletWindow::set_slot", int(id) << ", " << slot);
  data[id].slot = slot;
  TRACE_EXIT();
}

void
UnityAppletWindow::set_time_bar(BreakId id,
                                std::string text,
                                ITimeBar::ColorId primary_color,
                                int primary_val, int primary_max,
                                ITimeBar::ColorId secondary_color,
                                int secondary_val, int secondary_max)
{
  TRACE_ENTER_MSG("UnityAppletWindow::set_time_bar", int(id) << "=" << text);
  data[id].bar_text = text;
  data[id].bar_primary_color = primary_color;
  data[id].bar_primary_val = primary_val;
  data[id].bar_primary_max = primary_max;
  data[id].bar_secondary_color = secondary_color;
  data[id].bar_secondary_val = secondary_val;
  data[id].bar_secondary_max = secondary_max;
  TRACE_EXIT();
}

void
UnityAppletWindow::set_tip(std::string tip)
{
  (void) tip;
}

void
UnityAppletWindow::set_icon(IconType type)
{
  (void) type;
}

void
UnityAppletWindow::update_view()
{
  TRACE_ENTER("UnityAppletWindow::update_view");

  DBus *dbus = CoreFactory::get_dbus();
  
  if (dbus != NULL && dbus->is_available())
    {
      org_workrave_UnityInterface *iface = org_workrave_UnityInterface::instance(dbus);
      
      if (iface != NULL)
        {
          iface->UpdateIndicator("/org/workrave/Workrave/UI",
                                 data[BREAK_ID_MICRO_BREAK], data[BREAK_ID_REST_BREAK], data[BREAK_ID_DAILY_LIMIT]);
        }
    }
  
  TRACE_EXIT();
}

//! Initializes the native gnome applet.
AppletWindow::AppletState
UnityAppletWindow::activate_applet()
{
  TRACE_ENTER("UnityAppletWindow::activate_applet");
  DBus *dbus = CoreFactory::get_dbus();

  if (dbus != NULL && dbus->is_available())
    {
      dbus->connect("/org/workrave/Workrave/UI",
                    "org.workrave.UnityInterface",
                    this);
    }
      
  TRACE_EXIT();
  return AppletWindow::APPLET_STATE_VISIBLE;
}


//! Destroys the native gnome applet.
void
UnityAppletWindow::deactivate_applet()
{
  TRACE_ENTER("UnityAppletWindow::deactivate_applet");
  TRACE_EXIT();
}

void
UnityAppletWindow::set_enabled(bool enabled)
{
  TRACE_ENTER_MSG("W32AppletWindow::set_enabled", enabled);
  TRACE_EXIT();
}

void
UnityAppletWindow::set_geometry(Orientation orientation, int size)
{
  (void) orientation;
  (void) size;
}

void UnityAppletWindow::set_applet_enabled(bool enable)
{
}
