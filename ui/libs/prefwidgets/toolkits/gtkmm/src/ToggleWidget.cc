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

#include "ToggleWidget.hh"

using namespace ui::prefwidgets::gtkmm;

ToggleWidget::ToggleWidget(std::shared_ptr<ui::prefwidgets::Toggle> def,
                           std::shared_ptr<ContainerWidget> container,
                           BuilderRegistry *registry)
  : Widget(registry)
  , def(def)
{
  init_ui(container);
}

void
ToggleWidget::init_ui(std::shared_ptr<ContainerWidget> container)
{
  auto label = def->get_label();
  if (!label.empty())
    {
      widget = Gtk::manage(new Gtk::CheckButton(label));
    }
  else
    {
      widget = Gtk::manage(new Gtk::CheckButton());
    }
  add_to_size_groups(def, widget);

  bool idx = def->get_value();
  widget->set_active(idx);

  def->init([this](bool on) { widget->set_active(on); });
  widget->signal_toggled().connect([this]() {
    bool on = widget->get_active();
    def->set_value(on);
  });

  container->add_widget(*widget);
}
