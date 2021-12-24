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

#include "StatusIcon.hh"

#include <string>

#ifdef PLATFORM_OS_WINDOWS
#  include "WindowsStatusIcon.hh"
#  include "ui/windows/IToolkitWindows.hh"
#endif

#include "GtkUtil.hh"
#include "ui/GUIConfig.hh"
#include "ui/TimerBoxControl.hh"
#include "debug.hh"
#include "ui/IApplication.hh"

using namespace std;
using namespace workrave;

StatusIcon::StatusIcon(std::shared_ptr<IApplication> app, std::shared_ptr<ToolkitMenu> status_icon_menu)
  : app(app)
  , menu(status_icon_menu)
  , apphold(app->get_toolkit())
{
  TRACE_ENTER("StatusIcon::StatusIcon");
  mode_icons[OperationMode::Normal] = GtkUtil::create_pixbuf("workrave-icon-medium.png");
  mode_icons[OperationMode::Suspended] = GtkUtil::create_pixbuf("workrave-suspended-icon-medium.png");
  mode_icons[OperationMode::Quiet] = GtkUtil::create_pixbuf("workrave-quiet-icon-medium.png");

#if !defined(USE_WINDOWSSTATUSICON) && defined(PLATFORM_OS_WINDOWS)
  wm_taskbarcreated = RegisterWindowMessageA("TaskbarCreated");
  auto toolkit_win = std::dynamic_pointer_cast<IToolkitWindows>(toolkit);
  if (toolkit_win)
    {
      workrave::utils::connect(toolkit_win->hook_event(), this, [this](MSG *msg) { return filter_func(msg); });
    }
#endif

  workrave::utils::connect(app->get_core()->signal_operation_mode_changed(), this, [this](auto mode) { set_operation_mode(mode); });

#if defined(PLATFORM_OS_WINDOWS)
  win32_popup_hack_connect(status_icon_menu->get_menu().get());
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
        apphold.set_hold(is_visible());
      }
  });

  bool tray_icon_enabled = GUIConfig::trayicon_enabled()();
  status_icon->set_visible(tray_icon_enabled);
  apphold.set_hold(is_visible());
}

void
StatusIcon::insert_icon()
{
  // Create status icon
  auto core = app->get_core();
  OperationMode mode = core->get_regular_operation_mode();

#ifdef USE_WINDOWSSTATUSICON
  status_icon = new WindowsStatusIcon();
  set_operation_mode(mode);
#else
  status_icon = Gtk::StatusIcon::create(mode_icons[mode]);

#endif
#ifdef USE_WINDOWSSTATUSICON
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
StatusIcon::set_tooltip(const std::string &tip)
{
#if !defined(USE_WINDOWSSTATUSICON)
  status_icon->set_tooltip_text(tip);
#else
  status_icon->set_tooltip(tip);
#endif
}

void
StatusIcon::show_balloon(string id, const string &balloon)
{
#ifdef USE_WINDOWSSTATUSICON
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
  menu->get_menu()->popup(button, activate_time);
}

void
StatusIcon::on_embedded_changed()
{
  visibility_changed_signal.emit();
}

#if defined(PLATFORM_OS_WINDOWS) && defined(USE_WINDOWSSTATUSICON)
void
StatusIcon::on_balloon_activate(string id)
{
  balloon_activated_signal.emit(id);
}
#endif

void
StatusIcon::on_activate()
{
  activated_signal.emit();
}

#if !defined(USE_WINDOWSSTATUSICON) && defined(PLATFORM_OS_WINDOWS)
bool
StatusIcon::filter_func(MSG *msg)
{
  bool ret = true;
  if (msg->message == wm_taskbarcreated)
    {
      insert_icon();
      ret = false;
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
StatusIcon::signal_activated()
{
  return activated_signal;
}

sigc::signal<void, string> &
StatusIcon::signal_balloon_activated()
{
  return balloon_activated_signal;
}

#if defined(PLATFORM_OS_WINDOWS)
// /* Taken from Gaim. needs to be gtkmm-ified. */
// /* This is a workaround for a bug in windows GTK+. Clicking outside of the
//    menu does not get rid of it, so instead we get rid of it as soon as the
//    pointer leaves the menu. */

void
StatusIcon::win32_popup_hack_connect(Gtk::Widget *menu)
{
  TRACE_ENTER("WINDOWSTrayMenu::win32_popup_hack_connect");

  GtkWidget *widget = (GtkWidget *)menu->gobj();
  g_signal_connect(widget, "leave-notify-event", G_CALLBACK(win32_popup_hack_leave_enter), NULL);
  g_signal_connect(widget, "enter-notify-event", G_CALLBACK(win32_popup_hack_leave_enter), NULL);

  TRACE_EXIT();
}

gboolean
StatusIcon::win32_popup_hack_hide(gpointer data)
{
  TRACE_ENTER("WINDOWSTrayMenu::win32_popup_hack_hide");
  if (data != NULL)
    {
      gtk_menu_popdown(GTK_MENU(data));
    }
  TRACE_EXIT();
  return FALSE;
}

gboolean
StatusIcon::win32_popup_hack_leave_enter(GtkWidget *menu, GdkEventCrossing *event, void *data)
{
  TRACE_ENTER("WINDOWSTrayMenu::win32_popup_hack_leave_enter");

  TRACE_MSG(event->type << " " << event->detail);

  (void)data;
  static guint hide_docklet_timer = 0;
  if (event->type == GDK_LEAVE_NOTIFY && (event->detail == GDK_NOTIFY_ANCESTOR || event->detail == GDK_NOTIFY_UNKNOWN))
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
#endif
