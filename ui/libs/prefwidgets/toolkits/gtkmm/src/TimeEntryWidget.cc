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

#include "TimeEntryWidget.hh"

#include "Hig.hh"

using namespace ui::prefwidgets::gtkmm;

TimeEntryWidget::TimeEntryWidget(std::shared_ptr<ui::prefwidgets::Time> def,
                                 std::shared_ptr<ContainerWidget> container,
                                 BuilderRegistry *registry)
  : Widget(registry)
  , def(def)
{
  init_ui(container);
}

void
TimeEntryWidget::set_value(int32_t t)
{
  hours_adjustment->set_value((double)(t / (60 * 60)));
  mins_adjustment->set_value((double)((t / 60) % 60));
  secs_adjustment->set_value((double)(t % 60));
}

int32_t
TimeEntryWidget::get_value()
{
  int s = secs->get_value_as_int();
  int h = hrs->get_value_as_int();
  int m = mins->get_value_as_int();
  return h * 60 * 60 + m * 60 + s;
}

void
TimeEntryWidget::init_ui(std::shared_ptr<ContainerWidget> container)
{
  // adjustment = Gtk::Adjustment::create(def->get_value(), def->get_min(), def->get_max());
  // widget = Gtk::manage(new Gtk::TimeEntryButton(adjustment));

  box = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::HORIZONTAL, 1));
  add_to_size_groups(def, box);

  secs = Gtk::manage(new Gtk::SpinButton(secs_adjustment));
  secs->set_numeric(true);
  secs->signal_changed().connect(sigc::mem_fun(*this, &TimeEntryWidget::on_changed));
  secs->signal_value_changed().connect(sigc::mem_fun(*this, &TimeEntryWidget::on_value_changed));

  secs->set_width_chars(2);
  secs->set_wrap(true);

  hrs = Gtk::manage(new Gtk::SpinButton(hours_adjustment));
  hrs->set_numeric(true);
  hrs->set_wrap(true);
  hrs->set_width_chars(2);
  hrs->signal_changed().connect(sigc::mem_fun(*this, &TimeEntryWidget::on_changed));
  hrs->signal_value_changed().connect(sigc::mem_fun(*this, &TimeEntryWidget::on_value_changed));

  mins = Gtk::manage(new Gtk::SpinButton(mins_adjustment));
  mins->set_numeric(true);
  mins->set_wrap(true);
  mins->set_width_chars(2);
  mins->signal_changed().connect(sigc::mem_fun(*this, &TimeEntryWidget::on_changed));
  mins->signal_value_changed().connect(sigc::mem_fun(*this, &TimeEntryWidget::on_value_changed));

  Gtk::Label *semi1 = Gtk::manage(new Gtk::Label(":"));
  Gtk::Label *semi2 = Gtk::manage(new Gtk::Label(":"));

  box->pack_start(*hrs, false, false);
  box->pack_start(*semi1, false, false);
  box->pack_start(*mins, false, false);
  box->pack_start(*semi2, false, false);
  box->pack_start(*secs, false, false);

  box->set_sensitive(def->get_sensitive());
  set_value(def->get_value());

  def->init([this](int v) {
    set_value(v);
    box->set_sensitive(def->get_sensitive());
  });

  container->add_label(def->get_label(), *box);
}

void
TimeEntryWidget::on_changed()
{
  int v = get_value();
  def->set_value(v);
}

void
TimeEntryWidget::on_value_changed()
{
  int v = get_value();
  def->set_value(v);
}
