// Copyright (C) 2002 - 2013 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef TIMERPREFERENCEPANEL_HH
#define TIMERPREFERENCEPANEL_HH

#include <cstdio>
#include <string>

#include <gtkmm/box.h>
#include <gtkmm/sizegroup.h>
#include <gtkmm/adjustment.h>

#include "core/ICore.hh"
#include "ui/IApplicationContext.hh"

class TimeEntry;
class DataConnector;

namespace Gtk
{
  class CheckButton;
  class SpinButton;
} // namespace Gtk

class TimerPreferencePanel : public Gtk::VBox
{
public:
  TimerPreferencePanel(std::shared_ptr<IApplicationContext> app,
                       workrave::BreakId timer,
                       Glib::RefPtr<Gtk::SizeGroup> hsize_group,
                       Glib::RefPtr<Gtk::SizeGroup> vsize_group);
  ~TimerPreferencePanel() override;

private:
  bool on_preludes_changed(const std::string &key, bool write);
  void on_exercises_changed();

  Gtk::Widget *create_prelude_panel();
  Gtk::Widget *create_options_panel();
  Gtk::Widget *create_timers_panel(Glib::RefPtr<Gtk::SizeGroup> hsize_group, Glib::RefPtr<Gtk::SizeGroup> vsize_group);
  void set_prelude_sensitivity();

  void on_enabled_toggled();
  void enable_buttons();

  workrave::BreakId break_id;
  std::shared_ptr<DataConnector> connector;

  Gtk::CheckButton *ignorable_cb{nullptr};
  Gtk::CheckButton *skippable_cb{nullptr};
  Gtk::CheckButton *monitor_cb{nullptr};
  Gtk::CheckButton *prelude_cb{nullptr};
  Gtk::CheckButton *has_max_prelude_cb{nullptr};
  TimeEntry *limit_tim{nullptr}, *auto_reset_tim{nullptr}, *snooze_tim{nullptr};
  Gtk::SpinButton *max_prelude_spin{nullptr};
  Glib::RefPtr<Gtk::Adjustment> max_prelude_adjustment{Gtk::Adjustment::create(0, 1, 100)};
  Gtk::CheckButton *allow_shutdown_cb{nullptr};
  Gtk::CheckButton *enabled_cb{nullptr};
  Gtk::CheckButton *auto_natural_cb{nullptr};
  Gtk::SpinButton *exercises_spin{nullptr};
  Glib::RefPtr<Gtk::Adjustment> exercises_adjustment{Gtk::Adjustment::create(0, 0, 10)};
};

#endif // TIMERPREFERENCEPANEL_HH
