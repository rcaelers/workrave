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

#ifndef STATUSICON_HH
#define STATUSICON_HH

#include <map>

#ifdef PLATFORM_OS_WINDOWS
#  define USE_WINDOWSSTATUSICON 1
#  include <gdk/gdkwin32.h>
#endif
#include <gtkmm/statusicon.h>

#include "core/ICore.hh"
#include "utils/Signals.hh"
#include "ui/MenuModel.hh"
#include "ToolkitMenu.hh"
#include "ui/AppHold.hh"

class WindowsStatusIcon;

class StatusIcon : public workrave::utils::Trackable
{
public:
  StatusIcon(std::shared_ptr<IApplication> app, std::shared_ptr<ToolkitMenu> status_icon_menu);

  void init();
  void set_operation_mode(workrave::OperationMode m);
  void set_tooltip(const std::string &tip);
  bool is_visible() const;
  void show_balloon(std::string id, const std::string &balloon);

  sigc::signal<void> &signal_visibility_changed();
  sigc::signal<void> &signal_activated();
  sigc::signal<void, std::string> &signal_balloon_activated();

private:
  void set_visible(bool b);
  void insert_icon();
  void on_activate();
  void on_popup_menu(guint button, guint activate_time);
  void on_embedded_changed();

#if defined(PLATFORM_OS_WINDOWS) && !defined(USE_WINDOWSSTATUSICON)
  bool filter_func(MSG *msg);
#endif

#if defined(PLATFORM_OS_WINDOWS) && defined(USE_WINDOWSSTATUSICON)
  void on_balloon_activate(std::string id);
#endif
private:
#if defined(PLATFORM_OS_WINDOWS)
  void win32_popup_hack_connect(Gtk::Widget *menu);
  static gboolean win32_popup_hack_hide(gpointer data);
  static gboolean win32_popup_hack_leave_enter(GtkWidget *menu, GdkEventCrossing *event, void *data);
#endif

  std::shared_ptr<IApplication> app;
  std::shared_ptr<ToolkitMenu> menu;
  AppHold apphold;

  std::map<workrave::OperationMode, Glib::RefPtr<Gdk::Pixbuf>> mode_icons;

  sigc::signal<void> visibility_changed_signal;
  sigc::signal<void> activated_signal;
  sigc::signal<void, std::string> balloon_activated_signal;

#ifdef USE_WINDOWSSTATUSICON
  WindowsStatusIcon *status_icon{nullptr};
#else
  Glib::RefPtr<Gtk::StatusIcon> status_icon;
#  ifdef PLATFORM_OS_WINDOWS
  UINT wm_taskbarcreated{0};
#  endif
#endif
};

#endif // STATUSICON_HH
