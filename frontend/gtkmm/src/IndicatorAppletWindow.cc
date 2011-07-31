// IndicatorAppletWindow.cc --- Applet info Window
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

#include <libdbusmenu-gtk/menuitem.h>
#include <libdbusmenu-glib/client.h>

#include "IndicatorAppletWindow.hh"
#include "indicator-applet.h"

#include "AppletControl.hh"
#include "TimerBoxControl.hh"
#include "GUI.hh"
#include "Menus.hh"

#include "ICore.hh"
#include "CoreFactory.hh"

#include "DBus.hh"
#include "DBusException.hh"
#include "DBusGUI.hh"

//! Constructor.
IndicatorAppletWindow::IndicatorAppletWindow(AppletControl *control) :
  control(control)
{
  timer_box_control = new TimerBoxControl("applet", *this);
  timer_box_view = this;

  service = NULL;
  menu_server = NULL;
  menu_root = NULL;

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      data[i].bar_text = "";
      data[i].bar_primary_color = 0;
      data[i].bar_primary_val = 0;
      data[i].bar_primary_max = 0;
      data[i].bar_secondary_color = 0;
      data[i].bar_secondary_val = 0;
      data[i].bar_secondary_max = 0;
    }
}


//! Destructor.
IndicatorAppletWindow::~IndicatorAppletWindow()
{
}

void
IndicatorAppletWindow::set_slot(BreakId id, int slot)
{
  TRACE_ENTER_MSG("IndicatorAppletWindow::set_slot", int(id) << ", " << slot);
  data[slot].slot = id;
  TRACE_EXIT();
}

void
IndicatorAppletWindow::set_time_bar(BreakId id,
                                std::string text,
                                ITimeBar::ColorId primary_color,
                                int primary_val, int primary_max,
                                ITimeBar::ColorId secondary_color,
                                int secondary_val, int secondary_max)
{
  TRACE_ENTER_MSG("IndicatorAppletWindow::set_time_bar", int(id) << "=" << text);
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
IndicatorAppletWindow::set_tip(std::string tip)
{
  (void) tip;
}

void
IndicatorAppletWindow::set_icon(IconType type)
{
  (void) type;
}

void
IndicatorAppletWindow::update_view()
{
  TRACE_ENTER("IndicatorAppletWindow::update_view");

  DBus *dbus = CoreFactory::get_dbus();

  if (dbus != NULL && dbus->is_available())
    {
      org_workrave_IndicatorInterface *iface = org_workrave_IndicatorInterface::instance(dbus);

      if (iface != NULL)
        {
          iface->UpdateIndicator(WORKRAVE_INDICATOR_SERVICE_OBJ,
                                 data[BREAK_ID_MICRO_BREAK], data[BREAK_ID_REST_BREAK], data[BREAK_ID_DAILY_LIMIT]);
        }
    }

  TRACE_EXIT();
}

//! Initializes the native gnome applet.
AppletWindow::AppletState
IndicatorAppletWindow::activate_applet()
{
  TRACE_ENTER("IndicatorAppletWindow::activate_applet");
  DBus *dbus = CoreFactory::get_dbus();

  if (dbus != NULL && dbus->is_available())
    {
      dbus->connect(WORKRAVE_INDICATOR_SERVICE_OBJ,
                    WORKRAVE_INDICATOR_SERVICE_IFACE,
                    this);

      // service = indicator_service_new_version(WORKRAVE_INDICATOR_SERVICE_NAME, WORKRAVE_INDICATOR_SERVICE_VERSION);
      // g_signal_connect(service, INDICATOR_SERVICE_SIGNAL_SHUTDOWN, G_CALLBACK(service_shutdown), NULL);

      control->set_applet_state(AppletControl::APPLET_INDICATOR, AppletWindow::APPLET_STATE_VISIBLE);
    }

  TRACE_EXIT();
  return AppletWindow::APPLET_STATE_VISIBLE;
}


//! Destroys the native gnome applet.
void
IndicatorAppletWindow::deactivate_applet()
{
  TRACE_ENTER("IndicatorAppletWindow::deactivate_applet");
  control->set_applet_state(AppletControl::APPLET_INDICATOR,
                            AppletWindow::APPLET_STATE_DISABLED);

	g_object_unref(G_OBJECT(service));
	g_object_unref(G_OBJECT(menu_server));
	g_object_unref(G_OBJECT(menu_root));

  TRACE_EXIT();
}

void
IndicatorAppletWindow::set_enabled(bool enabled)
{
  TRACE_ENTER_MSG("W32AppletWindow::set_enabled", enabled);
  TRACE_EXIT();
}

void
IndicatorAppletWindow::set_geometry(Orientation orientation, int size)
{
  (void) orientation;
  (void) size;
}

void
IndicatorAppletWindow::set_applet_enabled(bool enable)
{
  (void) enable;
}
