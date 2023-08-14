// Copyright (C) 2023 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UI_PREFWIDGETS_GTKMM_ENTRYWIDGET_HH
#define WORKRAVE_UI_PREFWIDGETS_GTKMM_ENTRYWIDGET_HH

#include <memory>

#include <gtkmm.h>

#include "ui/prefwidgets/Entry.hh"
#include "ContainerWidget.hh"

namespace ui::prefwidgets::gtkmm
{
  class EntryWidget : public Widget
  {
  public:
    EntryWidget(std::shared_ptr<ui::prefwidgets::Entry> def,
                std::shared_ptr<ContainerWidget> container,
                BuilderRegistry *registry);
    ~EntryWidget() override = default;

  private:
    void init_ui(std::shared_ptr<ContainerWidget> container);

  private:
    std::shared_ptr<ui::prefwidgets::Entry> def;
    Gtk::Entry *widget{};
  };
} // namespace ui::prefwidgets::gtkmm

#endif // WORKRAVE_UI_PREFWIDGETS_GTKMM_ENTRYWIDGET_HH
