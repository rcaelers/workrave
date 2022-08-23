// Copyright (C) 2002 - 2011 Raymond Penners <raymond@dotsphinx.com>
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

#include <gtkmm/spinbutton.h>

#include "debug.hh"

#include "TimeEntry.hh"

TimeEntry::TimeEntry()
  : GtkCompat::Box(Gtk::Orientation::HORIZONTAL, 1)
  , hours_adjustment(Gtk::Adjustment::create(0, 0, 23))
  , mins_adjustment(Gtk::Adjustment::create(0, 0, 59))
  , secs_adjustment(Gtk::Adjustment::create(0, 0, 59))
{

  secs = Gtk::manage(new Gtk::SpinButton(secs_adjustment));
  secs->set_numeric(true);
  secs->signal_changed().connect(sigc::mem_fun(*this, &TimeEntry::on_changed));
  secs->signal_value_changed().connect(sigc::mem_fun(*this, &TimeEntry::on_value_changed));

  secs->set_width_chars(2);
  secs->set_wrap(true);

  hrs = Gtk::manage(new Gtk::SpinButton(hours_adjustment));
  hrs->set_numeric(true);
  hrs->set_wrap(true);
  hrs->set_width_chars(2);
  hrs->signal_changed().connect(sigc::mem_fun(*this, &TimeEntry::on_changed));
  hrs->signal_value_changed().connect(sigc::mem_fun(*this, &TimeEntry::on_value_changed));

  mins = Gtk::manage(new Gtk::SpinButton(mins_adjustment));
  mins->set_numeric(true);
  mins->set_wrap(true);
  mins->set_width_chars(2);
  mins->signal_changed().connect(sigc::mem_fun(*this, &TimeEntry::on_changed));
  mins->signal_value_changed().connect(sigc::mem_fun(*this, &TimeEntry::on_value_changed));

  Gtk::Label *semi1 = Gtk::manage(new Gtk::Label(":"));
  Gtk::Label *semi2 = Gtk::manage(new Gtk::Label(":"));

  pack_start(*hrs, false, false);
  pack_start(*semi1, false, false);
  pack_start(*mins, false, false);
  pack_start(*semi2, false, false);
  pack_start(*secs, false, false);
}

void
TimeEntry::set_value(time_t t)
{
  hours_adjustment->set_value((double)(t / (60 * 60)));
  mins_adjustment->set_value((double)((t / 60) % 60));
  secs_adjustment->set_value((double)(t % 60));
}

time_t
TimeEntry::get_value()
{
  int s = secs->get_value_as_int();
  int h = hrs->get_value_as_int();
  int m = mins->get_value_as_int();
  return h * 60 * 60 + m * 60 + s;
}

void
TimeEntry::update(Gtk::SpinButton *spin)
{
  // Needless to say, this kinda sucks.
  Glib::ustring s = spin->get_text();
  const gchar *txt = s.c_str();
  if (txt != nullptr && *txt != 0)
    {
      gchar *err = nullptr;
      if (err == nullptr || *err == 0)
        {
          spin->update();
        }
    }
}

void
TimeEntry::on_changed()
{
  update(hrs);
  update(mins);
  update(secs);
}

void
TimeEntry::on_value_changed()
{
  sig_value_changed.emit();
}
