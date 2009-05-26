// Menu.hh --- Menu using +
//
// Copyright (C) 2001 - 2008 Rob Caelers & Raymond Penners
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

#ifndef MENU_HH
#define MENU_HH

#include "config.h"

#include <string>

#include "ICore.hh"

#include <gtkmm/window.h>

class Menu
{
public:
  Menu() {}
  virtual ~Menu() {}

  virtual void init() = 0;
  virtual void add_accel(Gtk::Window &window) = 0;
  virtual void popup(const guint button, const guint activate_time) = 0;
  virtual void resync(workrave::OperationMode mode, bool show_log) = 0;
};

#endif // MENU_HH
