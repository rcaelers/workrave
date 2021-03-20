// TimerPreferencesPanel.hh --- Preferences widgets for a timer
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2011 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef TIMERPREFERENCESPANEL_HH
#define TIMERPREFERENCESPANEL_HH

#include "preinclude.h"

#ifdef HAVE_CONFIG
#  include "config.h"
#endif

#include <cstdio>
#include <string>

#include <gtkmm/box.h>
#include <gtkmm/sizegroup.h>
#include <gtkmm/adjustment.h>

#include "ICore.hh"

#define HAVE_MICRO_BREAK_ACTIVITY 1

class TimeEntry;
class DataConnector;

namespace Gtk
{
  class CheckButton;
  class SpinButton;
} // namespace Gtk

using namespace workrave;

class TimerPreferencesPanel : public Gtk::VBox
{
public:
  TimerPreferencesPanel(BreakId timer, Glib::RefPtr<Gtk::SizeGroup> hsize_group, Glib::RefPtr<Gtk::SizeGroup> vsize_group);
  ~TimerPreferencesPanel() override;

private:
#ifdef HAVE_MICRO_BREAK_ACTIVITY
  bool on_monitor_changed(const std::string &key, bool write);
#endif
  bool on_activity_sensitive_toggled(const std::string &key, bool write);
  bool on_preludes_changed(const std::string &key, bool write);
#ifdef HAVE_EXERCISES
  void on_exercises_changed();
#endif
  Gtk::Widget *create_prelude_panel();
  Gtk::Widget *create_options_panel();
  Gtk::Widget *create_timers_panel(Glib::RefPtr<Gtk::SizeGroup> hsize_group, Glib::RefPtr<Gtk::SizeGroup> vsize_group);
  void set_prelude_sensitivity();

  void on_enabled_toggled();
  void enable_buttons();

  BreakId break_id;
  DataConnector *connector{nullptr};

  Gtk::CheckButton *ignorable_cb{nullptr};
  Gtk::CheckButton *skippable_cb{nullptr};
  Gtk::CheckButton *activity_sensitive_cb{nullptr};
#ifdef HAVE_MICRO_BREAK_ACTIVITY
  Gtk::CheckButton *monitor_cb{nullptr};
#endif
  Gtk::CheckButton *prelude_cb{nullptr};
  Gtk::CheckButton *has_max_prelude_cb{nullptr};
  TimeEntry *limit_tim{nullptr}, *auto_reset_tim{nullptr}, *snooze_tim{nullptr};
  Gtk::SpinButton *max_prelude_spin{nullptr};
#ifdef HAVE_GTK3
  Glib::RefPtr<Gtk::Adjustment> max_prelude_adjustment{Gtk::Adjustment::create(0, 1, 100)};
#else
  Gtk::Adjustment max_prelude_adjustment{0, 1, 100};
#endif
  Gtk::CheckButton *allow_shutdown_cb{nullptr};
  Gtk::CheckButton *enabled_cb{nullptr};
  Gtk::CheckButton *auto_natural_cb{nullptr};
#ifdef HAVE_EXERCISES
  Gtk::SpinButton *exercises_spin{nullptr};
#  ifdef HAVE_GTK3
  Glib::RefPtr<Gtk::Adjustment> exercises_adjustment{Gtk::Adjustment::create(0, 0, 10)};
#  else
  Gtk::Adjustment exercises_adjustment{0, 0, 10};
#  endif
#endif
};

#endif // TIMERPREFERENCESPANEL_HH
