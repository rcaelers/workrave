// StatusIcon.cc --- Status icon
//
// Copyright (C) 2006, 2007, 2008, 2009 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id$";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <assert.h>

#ifdef PLATFORM_OS_OSX
#include "ige-mac-dock.h"
#endif

#include "MainWindow.hh"
#include "CoreFactory.hh"
#include "StatusIcon.hh"
#include "Menus.hh"
#include "Util.hh"

StatusIcon::StatusIcon(MainWindow& mw)
  : main_window(mw)
{
  // Preload icons
  const char *mode_files[] =
    {
      "workrave-icon-medium.png",
      "workrave-suspended-icon-medium.png",
      "workrave-quiet-icon-medium.png",
    };
  assert(sizeof(mode_files)/sizeof(mode_files[0]) == OPERATION_MODE_SIZEOF);
  for (size_t i = 0; i < OPERATION_MODE_SIZEOF; i++)
    {
      std::string file = Util::complete_directory(mode_files[i],
                                                  Util::SEARCH_PATH_IMAGES);
      try
        {
          mode_icons[i] = Gdk::Pixbuf::create_from_file(file);
        }
      catch(...)
        {
        }
    }

#ifdef PLATFORM_OS_WIN32
  wm_taskbarcreated = RegisterWindowMessage("TaskbarCreated");
#endif

  insert_icon();
}

StatusIcon::~StatusIcon()
{
}

void
StatusIcon::insert_icon()
{
  // Create status icon
  ICore *core = CoreFactory::get_core();
  OperationMode mode = core->get_operation_mode();
  status_icon = Gtk::StatusIcon::create(mode_icons[mode]);

  status_icon->signal_size_changed().connect(sigc::mem_fun(*this, &StatusIcon::on_size_changed));
  
#ifdef HAVE_STATUSICON_SIGNAL 
  status_icon->signal_activate().connect(sigc::mem_fun(*this, &StatusIcon::on_activate));
  status_icon->signal_popup_menu().connect(sigc::mem_fun(*this, &StatusIcon::on_popup_menu));
#else
  // Hook up signals, missing from gtkmm
  GtkStatusIcon *gobj = status_icon->gobj();

  g_signal_connect(gobj, "activate",
                   reinterpret_cast<GCallback>(activate_callback), this);
  g_signal_connect(gobj, "popup-menu",
                   reinterpret_cast<GCallback>(popup_menu_callback), this);
#endif
}

void StatusIcon::set_operation_mode(OperationMode m)
{
  status_icon->set(mode_icons[m]);
}

bool StatusIcon::is_embedded() const
{
  return status_icon->is_embedded();
}

void StatusIcon::on_activate()
{
  main_window.on_activate();
}

void StatusIcon::on_popup_menu(guint button, guint activate_time)
{
  (void) button;

  // Note the 1 is a hack. It used to be 'button'. See bugzilla 598
  Menus::get_instance()->popup(Menus::MENU_APPLET, 1, activate_time);
}

bool StatusIcon::on_size_changed(guint size)
{
  (void) size;
  main_window.status_icon_changed();
  return true;
}

#ifndef HAVE_STATUSICON_SIGNAL 
void StatusIcon::activate_callback(GtkStatusIcon *,
                                   gpointer callback_data)
{
  static_cast<StatusIcon*>(callback_data)->on_activate();
}

void StatusIcon::popup_menu_callback(GtkStatusIcon *,
                                     guint button,
                                     guint activate_time,
                                     gpointer callback_data)
{
  static_cast<StatusIcon*>(callback_data)->on_popup_menu(button, activate_time);
}
#endif

void StatusIcon::set_timers_tooltip(std::string& tip)
{
  status_icon->set_tooltip(tip);
}

#ifdef PLATFORM_OS_WIN32
GdkFilterReturn
StatusIcon::win32_filter_func (void     *xevent,
                               GdkEvent *event)
{
  (void) event;
  MSG *msg = (MSG *) xevent;
  GdkFilterReturn ret = GDK_FILTER_CONTINUE;
  if (msg->message == wm_taskbarcreated)
    {
      insert_icon();
      ret = GDK_FILTER_REMOVE;
    }
  return ret;
}
#endif
