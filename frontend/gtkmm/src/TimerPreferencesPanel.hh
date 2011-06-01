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
#include "config.h"
#endif

#include <stdio.h>
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
}

using namespace workrave;

class TimerPreferencesPanel
  : public Gtk::VBox
{
public:
  TimerPreferencesPanel(BreakId timer,
                        Glib::RefPtr<Gtk::SizeGroup> hsize_group,
                        Glib::RefPtr<Gtk::SizeGroup> vsize_group);
  ~TimerPreferencesPanel();

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
  Gtk::Widget *create_timers_panel(Glib::RefPtr<Gtk::SizeGroup> hsize_group,
                                   Glib::RefPtr<Gtk::SizeGroup> vsize_group);
  void set_prelude_sensitivity();

  void on_enabled_toggled();
  void enable_buttons();

  BreakId break_id;
  DataConnector *connector;

  Gtk::CheckButton *ignorable_cb;
  Gtk::CheckButton *activity_sensitive_cb;
#ifdef HAVE_MICRO_BREAK_ACTIVITY
  Gtk::CheckButton *monitor_cb;
#endif
  Gtk::CheckButton *prelude_cb;
  Gtk::CheckButton *has_max_prelude_cb;
  TimeEntry *limit_tim, *auto_reset_tim, *snooze_tim;
  Gtk::SpinButton *max_prelude_spin;
#ifdef HAVE_GTK3
  Glib::RefPtr<Gtk::Adjustment> max_prelude_adjustment;
#else
  Gtk::Adjustment max_prelude_adjustment;
#endif  
  Gtk::CheckButton *enabled_cb;
  Gtk::CheckButton *auto_natural_cb;
#ifdef HAVE_EXERCISES
  Gtk::SpinButton *exercises_spin;
#ifdef HAVE_GTK3
  Glib::RefPtr<Gtk::Adjustment> exercises_adjustment;
#else
  Gtk::Adjustment exercises_adjustment;
#endif
#endif
};

#endif // TIMERPREFERENCESPANEL_HH
