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
  set_policy(GtkCompat::POLICY_NEVER, GtkCompat::POLICY_AUTOMATIC);

  list_box = Gtk::manage(new Gtk::ListBox());
#if GTK_CHECK_VERSION(4, 0, 0)
  list_box->set_selection_mode(Gtk::SelectionMode::SINGLE);
#else
  list_box->set_selection_mode(Gtk::SELECTION_SINGLE);
#endif
  list_box->signal_row_activated().connect([this](Gtk::ListBoxRow *row) {
    const char *id = (const char *)row->get_data("id");
    activated_signal(id);
  });

  GtkCompat::set_child(*this, *list_box);
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

  Gtk::Image *img = Gtk::manage(new Gtk::Image());
#if GTK_CHECK_VERSION(4, 0, 0)
  img->set_from_icon_name(image);
#else
  img->set_from_icon_name(image, Gtk::IconSize(Gtk::ICON_SIZE_INVALID));
#endif
  img->set_pixel_size(24);
  grid->attach(*img, 0, 0, 1, 1);

  auto *label = Gtk::manage(new Gtk::Label(name));
  label->set_hexpand(true);
  label->set_xalign(0.0);
  grid->attach(*label, 1, 0, 1, 1);

  auto *row = Gtk::manage(new Gtk::ListBoxRow());
  GtkCompat::set_child(*row, *grid);
  row->set_data("id", g_strdup(id.c_str()), g_free);

#if GTK_CHECK_VERSION(4, 0, 0)
  list_box->append(*row);
#else
  list_box->add(*row);
#endif
  if (!has_rows)
    {
      has_rows = true;
      list_box->select_row(*row);
    }
}

PanelList::activated_signal_t &
PanelList::signal_activated()
{
  return activated_signal;
}
