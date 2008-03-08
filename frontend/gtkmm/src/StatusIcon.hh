// StatusIcon.hh --- Status icon
//
// Copyright (C) 2006, 2007, 2008 Rob Caelers & Raymond Penners
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

#ifndef STATUSICON_HH
#define STATUSICON_HH

#include "preinclude.h"

#ifdef PLATFORM_OS_WIN32
#include <gdk/gdkwin32.h>
#endif
#include <gtkmm/statusicon.h>
#include "ICore.hh"

#ifndef WR_CHECK_VERSION
#define WR_CHECK_VERSION(comp,major,minor,micro)   \
    (comp##_MAJOR_VERSION > (major) || \
     (comp##_MAJOR_VERSION == (major) && comp##_MINOR_VERSION > (minor)) || \
     (comp##_MAJOR_VERSION == (major) && comp##_MINOR_VERSION == (minor) && \
      comp##_MICRO_VERSION >= (micro)))
#endif

#if WR_CHECK_VERSION(GTKMM,2,11,1)
#define HAVE_STATUSICON_SIGNAL 1
#endif

class MainWindow;

class StatusIcon
{
public:
  StatusIcon(MainWindow& mw);
  ~StatusIcon();

  void set_operation_mode(OperationMode m);
  void set_timers_tooltip(std::string& tip);
#ifdef PLATFORM_OS_WIN32
  GdkFilterReturn win32_filter_func (void *xevent, GdkEvent *event);
#endif

private:
  void insert_icon();
  void on_activate();
  void on_popup_menu(guint button, guint activate_time);

#ifndef HAVE_STATUSICON_SIGNAL 
  static void activate_callback(GtkStatusIcon *si, gpointer callback_data);
  static void popup_menu_callback(GtkStatusIcon *si, guint button, guint activate_time,
                                  gpointer callback_data);
#endif
  
  Glib::RefPtr<Gtk::StatusIcon> status_icon;
  MainWindow& main_window;
  Glib::RefPtr<Gdk::Pixbuf> mode_icons[OPERATION_MODE_SIZEOF];
#ifdef PLATFORM_OS_WIN32
  UINT wm_taskbarcreated;
#endif
};


#endif // STATUSICON_HH
