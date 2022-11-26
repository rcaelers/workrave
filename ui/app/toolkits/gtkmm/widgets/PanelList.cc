// Copyright (C) 20221 Rob Caelers <robc@krandor.nl>
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

#include "PanelList.hh"
#include "GtkUtil.hh"

PanelList::PanelList()
{
  set_vexpand(true);
  set_size_request(200, -1);
  set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

  list_box = Gtk::manage(new Gtk::ListBox());
  list_box->set_selection_mode(Gtk::SelectionMode::SELECTION_SINGLE);
  list_box->signal_row_activated().connect([this](Gtk::ListBoxRow *row) {
    const char *id = (const char *)row->get_data("id");
    activated_signal(id);
  });

  add(*list_box);
}

void
PanelList::add_row(const std::string &id, const std::string &name, const std::string &image)
{
  auto *grid = Gtk::manage(new Gtk::Grid());
  grid->set_hexpand(true);
  grid->set_margin_bottom(12);
  grid->set_margin_top(12);
  grid->set_margin_start(6);
  grid->set_margin_end(6);
  grid->set_column_spacing(12);

  auto *img = Gtk::manage(GtkUtil::create_image("daily-limit.png"));
  grid->attach(*img, 0, 0, 1, 1);

  auto *label = Gtk::manage(new Gtk::Label(name));
  label->set_hexpand(true);
  label->set_xalign(0.0);
  grid->attach(*label, 1, 0, 1, 1);

  auto *row = Gtk::manage(new Gtk::ListBoxRow());
  row->add(*grid);
  row->set_data("id", g_strdup(id.c_str()), g_free);

  list_box->add(*row);
}

PanelList::activated_signal_t &
PanelList::signal_activated()
{
  return activated_signal;
}
