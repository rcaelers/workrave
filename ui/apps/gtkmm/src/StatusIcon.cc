// Copyright (C) 2006 - 2013 Rob Caelers & Raymond Penners
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
#  include "config.h"
#endif

#include "debug.hh"
#include <cassert>
#include <string>

#ifdef PLATFORM_OS_MACOS
#  if HAVE_IGE_MAC_INTEGRATION
#    include "ige-mac-dock.h"
#  endif
#  if HAVE_GTK_MAC_INTEGRATION
#    include "gtk-mac-dock.h"
#  endif
#endif

#ifdef PLATFORM_OS_WINDOWS
#  include "W32StatusIcon.hh"
#endif

#include "StatusIcon.hh"

#include "GUI.hh"
#include "commonui/Backend.hh"
#include "config/IConfigurator.hh"
#include "commonui/GUIConfig.hh"
#include "Menus.hh"
#include "commonui/TimerBoxControl.hh"
#include "GtkUtil.hh"

using namespace std;
using namespace workrave;

StatusIcon::StatusIcon()
{
  TRACE_ENTER("StatusIcon::StatusIcon");
  mode_icons[OperationMode::Normal] = GtkUtil::create_pixbuf("workrave-icon-medium.png");
  mode_icons[OperationMode::Suspended] = GtkUtil::create_pixbuf("workrave-suspended-icon-medium.png");
  mode_icons[OperationMode::Quiet] = GtkUtil::create_pixbuf("workrave-quiet-icon-medium.png");

#if !defined(USE_W32STATUSICON) && defined(PLATFORM_OS_WINDOWS)
  wm_taskbarcreated = RegisterWindowMessageA("TaskbarCreated");
#endif
  TRACE_EXIT();
}

void
StatusIcon::init()
{
  insert_icon();

  GUIConfig::trayicon_enabled().connect(this, [this](bool enabled) {
    if (status_icon->get_visible() != enabled)
      {
        visibility_changed_signal.emit();
        status_icon->set_visible(enabled);
      }
  });

  bool tray_icon_enabled = GUIConfig::trayicon_enabled()();
  status_icon->set_visible(tray_icon_enabled);
}

void
StatusIcon::insert_icon()
{
  // Create status icon
  auto core = Backend::get_core();
  OperationMode mode = core->get_operation_mode_regular();

#ifdef USE_W32STATUSICON
  status_icon = new W32StatusIcon();
  set_operation_mode(mode);
#else
  status_icon = Gtk::StatusIcon::create(mode_icons[mode]);
#endif

#ifdef USE_W32STATUSICON
  status_icon->signal_balloon_activate().connect(sigc::mem_fun(*this, &StatusIcon::on_balloon_activate));
  status_icon->signal_activate().connect(sigc::mem_fun(*this, &StatusIcon::on_activate));
  status_icon->signal_popup_menu().connect(sigc::mem_fun(*this, &StatusIcon::on_popup_menu));
#else
  status_icon->signal_activate().connect(sigc::mem_fun(*this, &StatusIcon::on_activate));
  status_icon->signal_popup_menu().connect(sigc::mem_fun(*this, &StatusIcon::on_popup_menu));
  status_icon->property_embedded().signal_changed().connect(sigc::mem_fun(*this, &StatusIcon::on_embedded_changed));
#endif
}

void
StatusIcon::set_operation_mode(OperationMode m)
{
  TRACE_ENTER_MSG("StatusIcon::set_operation_mode", (int)m);
  if (mode_icons[m])
    {
      status_icon->set(mode_icons[m]);
    }
  TRACE_EXIT();
}

bool
StatusIcon::is_visible() const
{
  return status_icon->is_embedded() && status_icon->get_visible();
}

void
StatusIcon::set_tooltip(std::string &tip)
{
#if defined(HAVE_GTK3) && !defined(USE_W32STATUSICON)
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
  (void)id;
  (void)balloon;
#endif
}

void
StatusIcon::on_popup_menu(guint button, guint activate_time)
{
  (void)button;

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

#if defined(PLATFORM_OS_WINDOWS) && defined(USE_W32STATUSICON)
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

#if !defined(USE_W32STATUSICON) && defined(PLATFORM_OS_WINDOWS)
GdkFilterReturn
StatusIcon::win32_filter_func(void *xevent, GdkEvent *event)
{
  (void)event;
  MSG *msg = (MSG *)xevent;
  GdkFilterReturn ret = GDK_FILTER_CONTINUE;
  if (msg->message == wm_taskbarcreated)
    {
      insert_icon();
      ret = GDK_FILTER_REMOVE;
    }
  return ret;
}
#endif

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
