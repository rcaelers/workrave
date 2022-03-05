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

#include "Widget.hh"

#include <gtkmm.h>

using namespace ui::prefwidgets::gtkmm;

Widget::Widget(BuilderRegistry *registry)
  : registry(registry)
{
}

void
Widget::add_to_size_groups(std::shared_ptr<ui::prefwidgets::Widget> def, Gtk::Widget *widget)
{
  if (registry != nullptr)
    {
      for (auto sg: def->get_size_groups())
        {
          auto size_group = registry->get_size_group(sg);
          size_group->add(widget);
        }
    }
}
