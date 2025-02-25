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

#ifndef WORKRAVE_UI_PREFWIDGETS_GTKMM_SIZEGROUPWIDGET_HH
#define WORKRAVE_UI_PREFWIDGETS_GTKMM_SIZEGROUPWIDGET_HH

#include <memory>
#include <string>
#include <list>

#include <gtkmm.h>

#include "ui/prefwidgets/SizeGroup.hh"

namespace ui::prefwidgets::gtkmm
{
  class Widget;

  class SizeGroup
  {
  public:
    explicit SizeGroup(std::shared_ptr<ui::prefwidgets::SizeGroup> def);
    ~SizeGroup() = default;

    void add(Gtk::Widget *widget);

  private:
    std::shared_ptr<ui::prefwidgets::SizeGroup> def;
    Glib::RefPtr<Gtk::SizeGroup> group;
  };
} // namespace ui::prefwidgets::gtkmm

#endif // WORKRAVE_UI_PREFWIDGETS_GTKMM_SIZEGROUPWIDGET_HH
