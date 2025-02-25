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

#ifndef TIMEENTRY_HH
#define TIMEENTRY_HH

#include <gtkmm/box.h>
#include <gtkmm/adjustment.h>

namespace Gtk
{
  class SpinButton;
}

class TimeEntry : public Gtk::HBox
{
public:
  TimeEntry();
  ~TimeEntry() override = default;

  time_t get_value();
  void set_value(time_t time);

  using signal_value_changed_t = sigc::signal<void()>;
  signal_value_changed_t &signal_value_changed();

protected:
  virtual void on_changed();
  virtual void on_value_changed();

  signal_value_changed_t sig_value_changed;

private:
  void update(Gtk::SpinButton *spin);

private:
  Gtk::SpinButton *hrs{nullptr};
  Gtk::SpinButton *mins{nullptr};
  Gtk::SpinButton *secs{nullptr};

  Glib::RefPtr<Gtk::Adjustment> hours_adjustment;
  Glib::RefPtr<Gtk::Adjustment> mins_adjustment;
  Glib::RefPtr<Gtk::Adjustment> secs_adjustment;
};

inline TimeEntry::signal_value_changed_t &
TimeEntry::signal_value_changed()
{
  return sig_value_changed;
}

#endif // TIMEENTRY_HH
