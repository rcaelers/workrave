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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "EntryWidget.hh"

#include "Hig.hh"

using namespace ui::prefwidgets::gtkmm;

EntryWidget::EntryWidget(std::shared_ptr<ui::prefwidgets::Entry> def,
                         std::shared_ptr<ContainerWidget> container,
                         BuilderRegistry *registry)
  : Widget(registry)
  , def(def)
{
  init_ui(container);
}

void
EntryWidget::init_ui(std::shared_ptr<ContainerWidget> container)
{
  widget = Gtk::manage(new Gtk::Entry());
  add_to_size_groups(def, widget);

  widget->set_width_chars(60);
  widget->set_text(def->get_content());

  widget->set_sensitive(def->get_sensitive());

  def->init([this](std::string txt) {
    widget->set_text(txt);
    widget->set_sensitive(def->get_sensitive());
  });

  widget->signal_changed().connect([this]() {
    auto txt = widget->get_text();
    def->set_value(txt);
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
