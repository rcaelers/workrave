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

#include "Toggle.hh"

using namespace ui::prefwidgets;

Toggle::Toggle(const std::string &label)
  : WidgetBase<Toggle, bool>(label)
{
}

std::shared_ptr<Toggle>
Toggle::create(const std::string &label)
{
  return std::make_shared<Toggle>(label);
}

std::shared_ptr<Toggle>
Toggle::create()
{
  return std::make_shared<Toggle>();
}
