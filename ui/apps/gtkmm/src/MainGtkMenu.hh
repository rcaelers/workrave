// Copyright (C) 2001 - 2009, 2011, 2013 Rob Caelers & Raymond Penners
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

#ifndef MAINGTKMENU_HH
#define MAINGTKMENU_HH

#include <string>

#include <gtkmm.h>

#include "MenuBase.hh"

class MainGtkMenu : public MenuBase
{
public:
  MainGtkMenu(bool show_open);
  ~MainGtkMenu() override = default;

  virtual void create_actions();
  virtual void create_menu();
  virtual void post_init() {}

  void init() override;
  void popup(const guint button, const guint activate_time) override;
  void resync(workrave::OperationMode mode, workrave::UsageMode usage) override;

private:
  void on_menu_mode(int mode);
  void on_menu_reading();

#ifdef PLATFORM_OS_MACOSOS
  void macos_popup_hack_connect(Gtk::Menu *menu);
  static gboolean macos_popup_hack_hide(gpointer data);
  static gboolean macos_popup_hack_leave_enter(GtkWidget *menu,
                                               GdkEventCrossing *event,
                                               void *data);
#endif


protected:
  Glib::RefPtr<Gio::SimpleActionGroup> action_group;

  //!
  std::unique_ptr<Gtk::Menu> popup_menu;

  //!
  bool show_open;
};

#endif // MAINGTKMENU_HH
