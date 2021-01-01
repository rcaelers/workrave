// X11SystrayAppletWindow.hh --- X11 Applet Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2012 Rob Caelers & Raymond Penners
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

#ifndef X11SYSTRAYAPPLETWINDOW_HH
#define X11SYSTRAYAPPLETWINDOW_HH

#include "preinclude.h"
#include <stdio.h>

#include "IConfiguratorListener.hh"
#include "AppletWindow.hh"
#include "Orientation.hh"

#include <sigc++/trackable.h>
#include <gtkmm/bin.h>
#include <gtkmm/menu.h>
#include <gtkmm/plug.h>
#include <gtkmm/eventbox.h>
#include "gtktrayicon.h"

class TimerBoxControl;
class TimerBoxGtkView;
class AppletControl;

using namespace workrave;

class X11SystrayAppletWindow :
  public IConfiguratorListener,
  public sigc::trackable,
  public AppletWindow
{
public:
  X11SystrayAppletWindow();
  virtual ~X11SystrayAppletWindow();

  bool is_visible() const override;

private:
  //! Gtk timerbox viewer
  TimerBoxGtkView *view { nullptr };

  //! The Gtk+ plug in the panel.
  Gtk::Plug *plug { nullptr };

  //! Container to put the timers in..
  Gtk::Bin *container { nullptr };

  //! Align break orientationly.
  Orientation applet_orientation { ORIENTATION_UP };

  //! Size of the applet
  int applet_size { 0 };

  //! Applet currently visible?
  bool applet_active { false };

  //! Applet embedded?
  bool embedded { false };

  bool enabled { false };

  //! The tray icon
  WRGtkTrayIcon *tray_icon  { nullptr };

private:
  void activate();
  void deactivate();

  static void static_notify_callback(GObject    *gobject,
                                     GParamSpec *arg,
                                     gpointer    user_data);
  void notify_callback();
  void on_menu_restbreak_now();
  void button_clicked(int button);

  // Events.
  void on_embedded();
  bool on_button_press_event(GdkEventButton *event);
  bool on_delete_event(GdkEventAny*);
  void on_size_allocate(Gtk::Allocation &allocation);

  void config_changed_notify(const std::string &key);
  void read_configuration();
};

#endif // X11SYSTRAYAPPLETWINDOW_HH
