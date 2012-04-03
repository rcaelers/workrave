// StatusIcon.cc --- Status icon
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <string>
#include <assert.h>

#ifdef PLATFORM_OS_OSX
#include "ige-mac-dock.h"
#endif

#ifdef PLATFORM_OS_WIN32
#include "W32StatusIcon.hh"
#endif

#include "StatusIcon.hh"

#include "GUI.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"
#include "GUIConfig.hh"
#include "Menus.hh"
#include "Util.hh"
#include "TimerBoxControl.hh"

StatusIcon::StatusIcon()
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

#if !defined(USE_W32STATUSICON) && defined(PLATFORM_OS_WIN32)
  wm_taskbarcreated = RegisterWindowMessage("TaskbarCreated");
#endif
}

StatusIcon::~StatusIcon()
{
}

void
StatusIcon::init()
{
  insert_icon();

  CoreFactory::get_configurator()->add_listener(GUIConfig::CFG_KEY_TRAYICON_ENABLED, this);
  
  bool tray_icon_enabled = GUIConfig::get_trayicon_enabled();
  if (!tray_icon_enabled)
    {
      // Recover from bug in 1.9.3 where tray icon AND mainwindow could be disabled
      TimerBoxControl::set_enabled("main_window", true);
    }

  status_icon->set_visible(tray_icon_enabled);
}

void
StatusIcon::insert_icon()
{
  // Create status icon
  ICore *core = CoreFactory::get_core();
  OperationMode mode = core->get_operation_mode_regular();

#ifdef USE_W32STATUSICON
  status_icon = new W32StatusIcon();
  set_operation_mode(mode);
#else
  status_icon = Gtk::StatusIcon::create(mode_icons[mode]);
  status_icon->property_embedded().signal_changed().connect(sigc::mem_fun(*this, &StatusIcon::on_embedded_changed));
#endif

#ifdef HAVE_STATUSICON_SIGNAL
#ifdef USE_W32STATUSICON
  status_icon->signal_balloon_activate().connect(sigc::mem_fun(*this, &StatusIcon::on_balloon_activate));
#endif
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

void
StatusIcon::set_operation_mode(OperationMode m)
{
  status_icon->set(mode_icons[m]);
}

bool
StatusIcon::is_visible() const
{
  return status_icon->is_embedded() && status_icon->get_visible();
}

void
StatusIcon::set_tooltip(std::string& tip)
{
#ifdef HAVE_GTK3
  status_icon->set_tooltip_text(tip);
#else
  status_icon->set_tooltip(tip);
#endif
}

void
StatusIcon::show_balloon(string id, const string &balloon)
{
#ifdef USE_W32STATUSICON
  status_icon->show_balloon(id, balloon);
#else
  (void) id;
  (void) balloon;
#endif
}

void
StatusIcon::on_popup_menu(guint button, guint activate_time)
{
  (void) button;

  // Note the 1 is a hack. It used to be 'button'. See bugzilla 598
  IGUI *gui = GUI::get_instance();
  Menus *menus = gui->get_menus();
  menus->popup(Menus::MENU_MAINAPPLET, 1, activate_time);
}

void
StatusIcon::on_embedded_changed()
{
  visibility_changed_signal.emit();
}

#ifndef HAVE_STATUSICON_SIGNAL
void
StatusIcon::activate_callback(GtkStatusIcon *,
                                   gpointer callback_data)
{
  static_cast<StatusIcon*>(callback_data)->on_activate();
}

void
StatusIcon::popup_menu_callback(GtkStatusIcon *,
                                     guint button,
                                     guint activate_time,
                                     gpointer callback_data)
{
  static_cast<StatusIcon*>(callback_data)->on_popup_menu(button, activate_time);
}
#endif

#if defined(PLATFORM_OS_WIN32) && defined(USE_W32STATUSICON)
void
StatusIcon::on_balloon_activate(string id)
{
  balloon_activate_signal.emit(id);
}
#endif

void
StatusIcon::on_activate()
{
  activate_signal.emit();
}

#if !defined(USE_W32STATUSICON) && defined(PLATFORM_OS_WIN32)
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

void
StatusIcon::config_changed_notify(const std::string &key)
{
  TRACE_ENTER_MSG("StatusIcon::config_changed_notify", key);

  if (key == GUIConfig::CFG_KEY_TRAYICON_ENABLED)
    {
      bool visible = GUIConfig::get_trayicon_enabled();
      if (status_icon->get_visible() != visible)
        {
          visibility_changed_signal.emit();
          status_icon->set_visible(visible);
        }
    }

  TRACE_EXIT();
}

sigc::signal<void> &
StatusIcon::signal_visibility_changed()
{
  return visibility_changed_signal;
}

sigc::signal<void> &
StatusIcon::signal_activate()
{
  return activate_signal;
}

sigc::signal<void, string> &
StatusIcon::signal_balloon_activate()
{
  return balloon_activate_signal;
}
