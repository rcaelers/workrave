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

#ifndef WORKRAVE_UI_PREFWIDGETS_GTKMM_CONTAINERWIDGET_HH
#define WORKRAVE_UI_PREFWIDGETS_GTKMM_CONTAINERWIDGET_HH

#include <memory>
#include <string>

#include <gtkmm.h>

#include "Widget.hh"

namespace ui::prefwidgets::gtkmm
{
  class ContainerWidget : public Widget
  {
  public:
    ContainerWidget() = default;
    explicit ContainerWidget(BuilderRegistry *registry);
    ~ContainerWidget() override = default;

    virtual void add(std::shared_ptr<Widget> widget) = 0;
    virtual void add_label(const std::string &label, Gtk::Widget &widget, bool expand = false, bool fill = false) = 0;
    virtual void add_widget(Gtk::Widget &widget, bool expand = false, bool fill = false) = 0;
  };
} // namespace ui::prefwidgets::gtkmm

#endif // WORKRAVE_UI_PREFWIDGETS_GTKMM_CONTAINERWIDGET_HH
