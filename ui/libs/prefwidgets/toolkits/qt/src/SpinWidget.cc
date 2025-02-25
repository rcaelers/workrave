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

#include "SpinWidget.hh"

using namespace ui::prefwidgets::qt;

SpinWidget::SpinWidget(std::shared_ptr<ui::prefwidgets::Value> def,
                       std::shared_ptr<ContainerWidget> container,
                       BuilderRegistry *registry)
  : Widget(registry)
  , def(def)
{
  init_ui(container);
}

void
SpinWidget::init_ui(std::shared_ptr<ContainerWidget> container)
{
  widget = new QSpinBox();
  widget->setMinimum(def->get_min());
  widget->setMaximum(def->get_max());
  widget->setValue(def->get_value());

  add_to_size_groups(def, widget);

  widget->setEnabled(def->get_sensitive());

  def->init([this](int v) {
    widget->setValue(v);
    widget->setEnabled(def->get_sensitive());
  });

  void (QSpinBox::*signal)(int) = &QSpinBox::valueChanged;
  connect(widget, signal, [this]() {
    int v = widget->value();
    def->set_value(v);
  });

  container->add_label(def->get_label(), widget);
}
