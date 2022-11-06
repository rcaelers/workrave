// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#include "ui/AppHold.hh"

AppHold::AppHold(std::shared_ptr<IToolkit> toolkit)
  : toolkit(toolkit)
{
}

void
AppHold::set_hold(bool h)
{
  if (h)
    {
      hold();
    }
  else
    {
      release();
    }
}

void
AppHold::hold()
{
  if (!held)
    {
      if (auto tk = toolkit.lock())
        {
          tk->hold();
        }
      held = true;
    }
}

void
AppHold::release()
{
  if (held)
    {
      if (auto tk = toolkit.lock())
        {
          tk->release();
        }
      held = false;
    }
}
