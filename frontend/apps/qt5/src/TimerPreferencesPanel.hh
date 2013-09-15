// TimerPreferencesPanel.hh
//
// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#include <QtGui>
#include <QtWidgets>

#include "ICore.hh"
#include "GUIConfig.hh"

#include "DataConnector.hh"
#include "SizeGroup.hh"

class TimerPreferencesPanel : public QWidget
{
  Q_OBJECT
  
public:
  TimerPreferencesPanel(workrave::BreakId break_id, SizeGroup* hsize_group, SizeGroup* vsize_group);
  virtual ~TimerPreferencesPanel();

protected:

private:

  bool on_monitor_changed(const std::string &key, bool write);
  bool on_preludes_changed(const std::string &key, bool write);
  
  void on_exercises_changed();
  void set_prelude_sensitivity();
  void on_enabled_toggled();
  void enable_buttons();

  QWidget *create_prelude_panel();
  QWidget *create_options_panel();
  QWidget *create_timers_panel();

private:
  BreakId break_id;
  DataConnector *connector;

  SizeGroup* hsize_group;
  SizeGroup* vsize_group;

  QCheckBox *enabled_cb;
  QCheckBox *auto_natural_cb;
  QCheckBox *has_max_prelude_cb;
  QCheckBox *ignorable_cb;
  QCheckBox *monitor_cb;
  QCheckBox *prelude_cb;
  QCheckBox *skippable_cb;
  QSpinBox *exercises_spin;
  QSpinBox *max_prelude_spin;
  TimeEntry *auto_reset_tim;
  TimeEntry *limit_tim;
  TimeEntry *snooze_tim;
};

#endif // TIMERPREFERENCESPANEL_HH
