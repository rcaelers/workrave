// StatusIcon.hh --- Status icon
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012 Rob Caelers & Raymond Penners
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
//#if ! GTK_CHECK_VERSION(2,22,1)
#define USE_W32STATUSICON 1
//#endif
#include <gdk/gdkwin32.h>
#endif
#include <gtkmm/statusicon.h>

#include "ICore.hh"
#include "IConfiguratorListener.hh"

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

using namespace workrave;

class W32StatusIcon;

class StatusIcon :
  public IConfiguratorListener
{
public:
  StatusIcon();
  ~StatusIcon();

  void init();
  void set_operation_mode(OperationMode m);
  void set_tooltip(std::string& tip);
  bool is_visible() const;
  void show_balloon(std::string id, const std::string& balloon);

  sigc::signal<void> &signal_visibility_changed();
  sigc::signal<void> &signal_activate();
  sigc::signal<void, std::string> &signal_balloon_activate();

private:
  void set_visible(bool b);
  void insert_icon();
  void on_activate();
  void on_popup_menu(guint button, guint activate_time);
  void on_embedded_changed();

  void config_changed_notify(const std::string &key);
  
#if defined(PLATFORM_OS_WIN32) && defined(USE_W32STATUSICON)
  GdkFilterReturn win32_filter_func(void *xevent, GdkEvent *event);
#endif
  
#if defined(PLATFORM_OS_WIN32) && defined(USE_W32STATUSICON)
  void on_balloon_activate(std::string id);
#endif
  
#ifndef HAVE_STATUSICON_SIGNAL
  static void activate_callback(GtkStatusIcon *si, gpointer callback_data);
  static void popup_menu_callback(GtkStatusIcon *si, guint button, guint activate_time,
                                  gpointer callback_data);
#endif

  Glib::RefPtr<Gdk::Pixbuf> mode_icons[OPERATION_MODE_SIZEOF];

  sigc::signal<void> visibility_changed_signal;
  sigc::signal<void> activate_signal;
  sigc::signal<void, std::string> balloon_activate_signal;

  
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
