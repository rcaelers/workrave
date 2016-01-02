// W32TrayMenu.cc --- Menus using W32Tray+
//
// Copyright (C) 2001 - 2008, 2012, 2013 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <windows.h>
#include <shellapi.h>
#undef interface

#include "commonui/nls.h"
#include "debug.hh"

#include "W32TrayMenu.hh"

#include <string>

#include <gtkmm/menu.h>
#include <gtkmm/menushell.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkwin32.h>


using namespace std;


//! Constructor.
W32TrayMenu::W32TrayMenu()
  : MainGtkMenu(true)
{
}


//! Destructor.
W32TrayMenu::~W32TrayMenu()
{
}

void
W32TrayMenu::post_init()
{
  win32_popup_hack_connect(popup_menu);
}


void
W32TrayMenu::popup(const guint button, const guint activate_time)
{
  (void) button;

  if (popup_menu != NULL)
    {
      popup_menu->popup(1, activate_time);
    }
}



// /* Taken from Gaim. needs to be gtkmm-ified. */
// /* This is a workaround for a bug in windows GTK+. Clicking outside of the
//    menu does not get rid of it, so instead we get rid of it as soon as the
//    pointer leaves the menu. */

void
W32TrayMenu::win32_popup_hack_connect(Gtk::Widget *menu)
{
  TRACE_ENTER("W32TrayMenu::win32_popup_hack_connect");

  GtkWidget *widget = (GtkWidget*) menu->gobj();
  g_signal_connect(widget, "leave-notify-event",
                   G_CALLBACK(win32_popup_hack_leave_enter), NULL);
  g_signal_connect(widget, "enter-notify-event",
                   G_CALLBACK(win32_popup_hack_leave_enter), NULL);

  TRACE_EXIT();
}

gboolean
W32TrayMenu::win32_popup_hack_hide(gpointer data)
{
  TRACE_ENTER("W32TrayMenu::win32_popup_hack_hide");
  if (data != NULL)
    {
      gtk_menu_popdown(GTK_MENU(data));
    }
  TRACE_EXIT();
  return FALSE;
}


gboolean
W32TrayMenu::win32_popup_hack_leave_enter(GtkWidget *menu, GdkEventCrossing *event,
                                          void *data)
{
  TRACE_ENTER("W32TrayMenu::win32_popup_hack_leave_enter");

  TRACE_MSG(event->type << " " <<  event->detail);

  (void) data;
  static guint hide_docklet_timer = 0;
  if (event->type == GDK_LEAVE_NOTIFY
      && (event->detail == GDK_NOTIFY_ANCESTOR || event->detail == GDK_NOTIFY_UNKNOWN))
    {
      /* Add some slop so that the menu doesn't annoyingly disappear when mousing around */
      TRACE_MSG("leave " << hide_docklet_timer);
      if (hide_docklet_timer == 0)
        {
          hide_docklet_timer = g_timeout_add(500, win32_popup_hack_hide, menu);
        }
    }
  else if (event->type == GDK_ENTER_NOTIFY && event->detail == GDK_NOTIFY_VIRTUAL)
    {
      TRACE_MSG("enter " << hide_docklet_timer);

      if (hide_docklet_timer != 0)
        {
          /* Cancel the hiding if we reenter */
          g_source_remove(hide_docklet_timer);
          hide_docklet_timer = 0;
        }
    }
  TRACE_EXIT();
  return FALSE;
}
