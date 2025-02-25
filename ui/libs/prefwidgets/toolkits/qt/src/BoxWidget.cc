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

#include "BoxWidget.hh"

#include <memory>

#include "Widget.hh"
#include "ui/prefwidgets/Widget.hh"
#include "ui/prefwidgets/Widgets.hh"

using namespace ui::prefwidgets::qt;

BoxWidget::BoxWidget(std::shared_ptr<ui::prefwidgets::Box> def,
                     std::shared_ptr<ContainerWidget> container,
                     BuilderRegistry *registry)
  : ContainerWidget(registry)
  , def(def)
{
  widget = new QWidget();
  if (def->get_orientation() == Orientation::Vertical)
    {
      layout = new QVBoxLayout;
    }
  else
    {
      layout = new QHBoxLayout;
    }

  add_to_size_groups(def, widget);
  container->add_widget(widget);
}

BoxWidget::BoxWidget(QLayout *layout)
  : layout(layout)
{
}

void
BoxWidget::add(std::shared_ptr<Widget> w)
{
  children.push_back(w);
}

void
BoxWidget::add_label(const std::string &label, QWidget *widget, bool expand, bool fill)
{
  layout->addWidget(widget);
}

void
BoxWidget::add_widget(QWidget *widget, bool expand, bool fill)
{
  layout->addWidget(widget);
}
