// Copyright (C) 2001 -2013 Rob Caelers <robc@krandor.nl>
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

#ifndef ITOOLKIT_PRIVATE_HH
#define ITOOLKIT_PRIVATE_HH

#include <gtkmm.h>
#include <optional>

#include "HeadInfo.hh"

class IToolkitPrivate
{
public:
  virtual ~IToolkitPrivate() = default;

  virtual std::optional<HeadInfo> get_head_info(int screen_index) const = 0;
  virtual void attach_menu(Gtk::Menu *menu) = 0;
};

#endif // ITOOLKIT_PRIVATE_HH
