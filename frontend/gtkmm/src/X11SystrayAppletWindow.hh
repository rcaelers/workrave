// X11SystrayAppletWindow.hh --- X11 Applet Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Rob Caelers & Raymond Penners
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
// $Id$
//

#ifndef X11SYSTRAYAPPLETWINDOW_HH
#define X11SYSTRAYAPPLETWINDOW_HH

#include "preinclude.h"
#include <stdio.h>

#include "AppletWindow.hh"

#include <sigc++/compatibility.h>

#include <gtkmm/bin.h>
#include <gtkmm/menu.h>
#include <gtkmm/plug.h>
#include <gtkmm/eventbox.h>

class TimerBoxControl;
class TimerBoxGtkView;
class AppletControl;

#include "eggtrayicon.h"

class X11SystrayAppletWindow :
  public SigC::Object,
  public AppletWindow
{
public:
  X11SystrayAppletWindow(AppletControl *control);
  virtual ~X11SystrayAppletWindow();

  void on_menu_restbreak_now();
  void button_clicked(int button);

private:
  //! Gtk timerbox viewer
  TimerBoxGtkView *view;

  //! The Gtk+ plug in the panel.
  Gtk::Plug *plug;

  //! Container to put the timers in..
  Gtk::Bin *container;

  //! Align break orientationly.
  Orientation applet_orientation;

  //! Size of the applet
  int applet_size;

  //! Applet currently visible?
  bool applet_active;

  //! Controller
  AppletControl *control;

  //! The tray icon
  EggTrayIcon *tray_icon;

private:
  AppletState activate_applet();
  void deactivate_applet();

  static void static_notify_callback(GObject    *gobject,
                                     GParamSpec *arg,
                                     gpointer    user_data);
  void notify_callback();

  // Evenyts.
  void on_embedded();
  bool on_button_press_event(GdkEventButton *event);
  bool on_delete_event(GdkEventAny*);
  bool delete_event(GdkEventAny *event);
};

#endif // X11SYSTRAYAPPLETWINDOW_HH
