// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#ifndef MENUBASE_HH
#define MENUBASE_HH

#include "IMenu.hh"

#include <gtkmm.h>

class MenuBase : public IMenu
{
public:
  ~MenuBase() override = default;

  void init() override
  {
  }

  void popup(const guint button, const guint activate_time) override
  {
    (void)button;
    (void)activate_time;
  };
};

#endif // MENUBASE_HH
