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

#include "Choice.hh"

using namespace ui::prefwidgets;

Choice::Choice(const std::string &label)
  : WidgetBase<Choice, int>(label)
{
}

std::shared_ptr<Choice>
Choice::create()
{
  return std::make_shared<Choice>();
}

std::shared_ptr<Choice>
Choice::create(const std::string &label)
{
  return std::make_shared<Choice>(label);
}

std::shared_ptr<Choice>
Choice::assign(const std::vector<std::string> &options)
{
  content = options;
  return shared_from_base<Choice>();
}

std::shared_ptr<Choice>
Choice::assign(std::function<std::vector<std::string>()> func)
{
  fill_func = func;
  return shared_from_base<Choice>();
}

std::vector<std::string>
Choice::get_content() const
{
  if (fill_func)
    {
      return fill_func();
    }
  return content;
}
