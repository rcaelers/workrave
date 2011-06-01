// GnomeAppletWindow.hh --- X11 Applet Window
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

#ifndef GNOMEAPPLETWINDOW_HH
#define GNOMEAPPLETWINDOW_HH

#include "preinclude.h"
#include <stdio.h>

#include "AppletWindow.hh"

#include <sigc++/trackable.h>

#include <string>

#include <gdkmm.h>
#include <gtkmm.h>
#include <gtkmm/plug.h>

class TimerBoxGtkView;
class AppletControl;
class org_workrave_GnomeAppletInterface;

class GnomeAppletWindow :
  public sigc::trackable,
  public AppletWindow
{
public:
  GnomeAppletWindow(AppletControl *control);
  virtual ~GnomeAppletWindow();

  //! Menus items to be synced.
  enum MenuSyncs
    {
      MENUSYNC_MODE_NORMAL,
      MENUSYNC_MODE_SUSPENDED,
      MENUSYNC_MODE_QUIET,
      MENUSYNC_MODE_READING,
      MENUSYNC_SHOW_LOG,
      MENUSYNC_SIZEOF
    };
  
  void fire_gnome_applet();

  void on_menu_restbreak_now();
  void button_clicked(int button);

  void set_menu_active(int menu, bool active);
  bool get_menu_active(int menu);
  
  // DBUS methods
  void set_applet_orientation(Orientation orientation);
  void set_applet_size(int size);
  void set_applet_background(int type, GdkColor &color, long xid);

private:
  //! Gtk timerbox viewer
  TimerBoxGtkView *view;

  //! The Gtk+ plug in the panel.
  Gtk::Plug *plug;

  //! Container to put the timers in..
  Gtk::Bin *container;

  //! Allign break orientationly.
  Orientation applet_orientation;

  //! Size of the applet
  int applet_size;

  //!
  org_workrave_GnomeAppletInterface *applet_control;

  //!
  AppletControl *control;

  //!
  bool applet_active;

private:
  void deactivate_applet();
  AppletState activate_applet();

  bool init_gnome_applet();
  void destroy_gnome_applet();

  static gboolean destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);

  // Events.
  void on_embedded();
  bool on_button_press_event(GdkEventButton *event);
  bool on_delete_event(GdkEventAny*);
  bool delete_event(GdkEventAny *event);
};

#endif // GNOMEAPPLETWINDOW_HH
