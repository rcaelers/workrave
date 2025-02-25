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

#include "Widget.hh"
#include "ui/prefwidgets/Widget.hh"
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "BoxWidget.hh"

#include <memory>

#include <gtkmm.h>

#include "ui/prefwidgets/Widgets.hh"
#include "Hig.hh"

using namespace ui::prefwidgets::gtkmm;

BoxWidget::BoxWidget(std::shared_ptr<ui::prefwidgets::Box> def,
                     std::shared_ptr<ContainerWidget> container,
                     BuilderRegistry *registry)
  : ContainerWidget(registry)
  , def(def)
{
  if (def->get_orientation() == Orientation::Vertical)
    {
      box = Gtk::manage(new Gtk::VBox(false, def->get_spacing()));
    }
  else
    {
      box = Gtk::manage(new Gtk::HBox(false, def->get_spacing()));
    }
  add_to_size_groups(def, box);
  container->add_widget(*box);
}

BoxWidget::BoxWidget(Gtk::Box *box)
  : box(box)
{
}

void
BoxWidget::add(std::shared_ptr<Widget> w)
{
  children.push_back(w);
}

void
BoxWidget::add_label(const std::string &label, Gtk::Widget &widget, bool expand, bool fill)
{
  box->pack_start(widget, expand, fill, 0);
}

void
BoxWidget::add_widget(Gtk::Widget &widget, bool expand, bool fill)
{
  box->pack_start(widget, expand, fill, 0);
}
