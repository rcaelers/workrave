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

#include "SliderWidget.hh"

#include "Hig.hh"

using namespace ui::prefwidgets::gtkmm;

SliderWidget::SliderWidget(std::shared_ptr<ui::prefwidgets::Value> def,
                           std::shared_ptr<ContainerWidget> container,
                           BuilderRegistry *registry)
  : Widget(registry)
  , def(def)
{
  init_ui(container);
}

void
SliderWidget::init_ui(std::shared_ptr<ContainerWidget> container)
{
  adjustment = Gtk::Adjustment::create(def->get_value(), def->get_min(), def->get_max());
  widget = Gtk::manage(new Gtk::Scale(adjustment));
  widget->set_digits(0);
  widget->set_sensitive(def->get_sensitive());
  add_to_size_groups(def, widget);

  def->init([this](int v) {
    adjustment->set_value(v);
    widget->set_sensitive(def->get_sensitive());
  });
  widget->signal_value_changed().connect([this]() {
    int v = static_cast<int>(widget->get_value());
    def->set_value(v);
  });

  container->add_label(def->get_label(), *widget, true, true);
}
