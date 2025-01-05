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

#include "DataConnector.hh"
#include "SizeGroup.hh"

class TimerPreferencesPanel : public QWidget
{
  Q_OBJECT

public:
  TimerPreferencesPanel(std::shared_ptr<IApplicationContext> app,
                        workrave::BreakId break_id,
                        std::shared_ptr<SizeGroup> hsize_group,
                        std::shared_ptr<SizeGroup> vsize_group);

private:
  auto on_preludes_changed(const std::string &key, bool write) -> bool;

  void on_exercises_changed();
  void set_prelude_sensitivity();
  void on_enabled_toggled();
  void enable_buttons();

  auto create_prelude_panel() -> QWidget *;
  auto create_options_panel() -> QWidget *;
  auto create_timers_panel() -> QWidget *;

private:
  workrave::BreakId break_id;
  DataConnector::Ptr connector;

  std::shared_ptr<SizeGroup> hsize_group;
  std::shared_ptr<SizeGroup> vsize_group;

  QCheckBox *enabled_cb{nullptr};
  QCheckBox *auto_natural_cb{nullptr};
  QCheckBox *has_max_prelude_cb{nullptr};
  QCheckBox *ignorable_cb{nullptr};
  QCheckBox *monitor_cb{nullptr};
  QCheckBox *prelude_cb{nullptr};
  QCheckBox *skippable_cb{nullptr};
  QSpinBox *exercises_spin{nullptr};
  QSpinBox *max_prelude_spin{nullptr};
  TimeEntry *auto_reset_tim{nullptr};
  TimeEntry *limit_tim{nullptr};
  TimeEntry *snooze_tim{nullptr};
};

#endif // TIMERPREFERENCESPANEL_HH
