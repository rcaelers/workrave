// StatusIcon.hh --- Status icon
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011 Rob Caelers & Raymond Penners
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

#ifndef STATUSICON_HH
#define STATUSICON_HH

#include "preinclude.h"

#ifdef PLATFORM_OS_WIN32
#if ! GTK_CHECK_VERSION(2,22,1)
#define USE_W32STATUSICON 1
#endif
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

class W32StatusIcon;
class MainWindow;

class StatusIcon
{
public:
  StatusIcon(MainWindow& mw);
  ~StatusIcon();

  void set_visible(bool b);
  void set_operation_mode(OperationMode m);
  void set_tooltip(std::string& tip);
  bool is_embedded() const;
#ifdef PLATFORM_OS_WIN32
#ifndef USE_W32STATUSICON
  GdkFilterReturn win32_filter_func (void *xevent, GdkEvent *event);
#endif
#endif
  void show_balloon(const std::string& balloon);

private:
  void insert_icon();
  void on_activate();
  void on_popup_menu(guint button, guint activate_time);
  bool on_size_changed(guint size);

#ifndef HAVE_STATUSICON_SIGNAL
  static void activate_callback(GtkStatusIcon *si, gpointer callback_data);
  static void popup_menu_callback(GtkStatusIcon *si, guint button, guint activate_time,
                                  gpointer callback_data);
#endif

  MainWindow& main_window;
  Glib::RefPtr<Gdk::Pixbuf> mode_icons[OPERATION_MODE_SIZEOF];

#ifdef USE_W32STATUSICON
  W32StatusIcon *status_icon;
#else
  Glib::RefPtr<Gtk::StatusIcon> status_icon;
#ifdef PLATFORM_OS_WIN32
  UINT wm_taskbarcreated;
#endif
#endif
};


#endif // STATUSICON_HH
