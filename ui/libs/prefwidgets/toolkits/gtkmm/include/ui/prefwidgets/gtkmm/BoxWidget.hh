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

#ifndef WORKRAVE_UI_PREFWIDGETS_GTKMM_BOXWIDGET_HH
#define WORKRAVE_UI_PREFWIDGETS_GTKMM_BOXWIDGET_HH

#include <memory>
#include <string>
#include <list>

#include <gtkmm.h>

#include "ui/prefwidgets/Box.hh"
#include "ContainerWidget.hh"

namespace ui::prefwidgets::gtkmm
{
  class BoxWidget : public ContainerWidget
  {
  public:
    BoxWidget(std::shared_ptr<ui::prefwidgets::Box> def, std::shared_ptr<ContainerWidget> container, BuilderRegistry *registry);
    explicit BoxWidget(Gtk::Box *box);
    ~BoxWidget() override = default;

    void add(std::shared_ptr<Widget> widget) override;
    void add_label(const std::string &label, Gtk::Widget &widget, bool expand = false, bool fill = false) override;
    void add_widget(Gtk::Widget &widget, bool expand = false, bool fill = false) override;

  private:
    std::shared_ptr<ui::prefwidgets::Box> def;
    Gtk::Box *box{};
    std::list<std::shared_ptr<Widget>> children;
  };
} // namespace ui::prefwidgets::gtkmm

#endif // WORKRAVE_UI_PREFWIDGETS_GTKMM_BOXWIDGET_HH
