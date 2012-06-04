// IMenu.hh --- Menu interface
//
// Copyright (C) 2001 - 2009, 2011, 2012 Rob Caelers & Raymond Penners
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

#ifndef IMENU_HH
#define IMENU_HH

#include <string>

#include "ICore.hh"

#include <gtkmm/window.h>

class IMenu
{
public:
  IMenu() {}
  virtual ~IMenu() {}

  virtual void init() = 0;
  virtual void add_accel(Gtk::Window &window) = 0;
  virtual void popup(const guint button, const guint activate_time) = 0;
  virtual void resync(workrave::OperationMode mode, workrave::UsageMode usage, bool show_log) = 0;
};

#endif // IMENU_HH
