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

#include <gtkmm/statusicon.h>

#include "core/ICore.hh"
#include "utils/Signals.hh"
#include "commonui/MenuModel.hh"
#include "ToolkitMenu.hh"
#include "ui/AppHold.hh"

class WindowsStatusIcon;

class StatusIcon : public workrave::utils::Trackable
{
public:
  using activated_signal_t = sigc::signal<void()>;
  using balloon_activated_signal_t = sigc::signal<void(std::string)>;

  StatusIcon(std::shared_ptr<IApplicationContext> app, std::shared_ptr<ToolkitMenu> status_icon_menu);
  ~StatusIcon();

  void init();
  void set_operation_mode(workrave::OperationMode m);
  void set_tooltip(const std::string &tip);
  bool is_visible() const;
  void show_balloon(std::string id, const std::string &balloon);

  activated_signal_t &signal_activated();
  balloon_activated_signal_t &signal_balloon_activated();

private:
  void set_visible(bool b);
  void insert_icon();
  void on_activate();
  void on_popup_menu(guint button, guint activate_time);
  void on_embedded_changed();

#if defined(PLATFORM_OS_WINDOWS)
  void on_balloon_activate(std::string id);
#endif
private:
  std::shared_ptr<IApplicationContext> app;
  std::shared_ptr<ToolkitMenu> menu;
  AppHold apphold;

  std::map<workrave::OperationMode, Glib::RefPtr<Gdk::Pixbuf>> mode_icons;

  activated_signal_t activated_signal;
  balloon_activated_signal_t balloon_activated_signal;

#if defined(PLATFORM_OS_WINDOWS)
  WindowsStatusIcon *status_icon{nullptr};
#else
  Glib::RefPtr<Gtk::StatusIcon> status_icon;
#endif
};

#endif // STATUSICON_HH
