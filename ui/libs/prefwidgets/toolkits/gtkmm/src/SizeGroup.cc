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

#include "SizeGroup.hh"

#include <gtkmm.h>

#include "ui/prefwidgets/Widgets.hh"

using namespace ui::prefwidgets::gtkmm;

SizeGroup::SizeGroup(std::shared_ptr<ui::prefwidgets::SizeGroup> def)
  : def(def)
{
  if (def->get_orientation() == Orientation::Vertical)
    {
#if GTK_CHECK_VERSION(4,0,0)
      group = Gtk::SizeGroup::create(Gtk::SizeGroup::Mode::VERTICAL);
#else
      group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_VERTICAL);
#endif
    }
  else
    {
#if GTK_CHECK_VERSION(4,0,0)
      group = Gtk::SizeGroup::create(Gtk::SizeGroup::Mode::HORIZONTAL);
#else
      group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
#endif
    }
}

void
SizeGroup::add(Gtk::Widget *widget)
{
  group->add_widget(*widget);
}
