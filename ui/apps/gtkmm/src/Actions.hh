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

#ifndef ACTIONS_HH
#define ACTIONS_HH

#include <string>

#include <gtkmm.h>

#include "MenuBase.hh"

class Actions
{
public:
  Actions(Glib::RefPtr<Gtk::Application> app);
  virtual ~MainGtkMenu() = default;

  void resync(workrave::OperationMode mode, workrave::UsageMode usage);

private:
  void on_menu_mode(int mode);
  void on_menu_reading();

  void init(Glib::RefPtr<Gtk::Application> app);
  
  Glib::RefPtr<Gio::SimpleActionGroup> action_group;
};

#endif // MAINGTKMENU_HH
