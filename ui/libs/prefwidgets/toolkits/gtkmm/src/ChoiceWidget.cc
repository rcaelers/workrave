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

#include "ChoiceWidget.hh"

#include "Hig.hh"

using namespace ui::prefwidgets::gtkmm;

ChoiceWidget::ChoiceWidget(std::shared_ptr<ui::prefwidgets::Choice> def,
                           std::shared_ptr<ContainerWidget> container,
                           BuilderRegistry *registry)
  : Widget(registry)
  , def(def)
{
  init_ui(container);
}

void
ChoiceWidget::init_ui(std::shared_ptr<ContainerWidget> container)
{
  widget = Gtk::manage(new Gtk::ComboBoxText());
  add_to_size_groups(def, widget);

  for (const auto &s: def->get_content())
    {
      widget->append(s);
    }

  widget->set_active(def->get_value());
  widget->set_sensitive(def->get_sensitive());

  def->init([this](int idx) {
    widget->set_active(idx);
    widget->set_sensitive(def->get_sensitive());
  });
  widget->signal_changed().connect([this]() {
    int idx = widget->get_active_row_number();
    def->set_value(idx);
  });

  auto label = def->get_label();
  if (!label.empty())
    {
      container->add_label(label, *widget);
    }
  else
    {
      container->add_widget(*widget);
    }
}
