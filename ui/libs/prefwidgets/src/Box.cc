// Copyright (C) 2022 Rob Caelers <robc@krandor.nl>
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

#include "Box.hh"

using namespace ui::prefwidgets;

Box::Box(Orientation orientation)
  : orientation(orientation)
{
}

std::shared_ptr<Box>
Box::create(Orientation orientation)
{
  return std::make_shared<Box>(orientation);
}

Orientation
Box::get_orientation() const
{
  return orientation;
}

int
Box::get_spacing() const
{
  return spacing_;
}

std::shared_ptr<Box>
Box::spacing(int spacing)
{
  spacing_ = spacing;
  return shared_from_base<Box>();
}
