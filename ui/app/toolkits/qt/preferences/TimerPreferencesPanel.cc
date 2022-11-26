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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "TimerPreferencesPanel.hh"

#include <QtGui>
#include <QStyle>

#include "debug.hh"
#include "core/CoreConfig.hh"
#include "core/ICore.hh"

#include "TimeEntry.hh"

using namespace workrave;
using namespace workrave::config;

TimerPreferencesPanel::TimerPreferencesPanel(std::shared_ptr<IApplicationContext> app,
                                             BreakId break_id,
                                             std::shared_ptr<SizeGroup> hsize_group,
                                             std::shared_ptr<SizeGroup> vsize_group)
  : break_id(break_id)
  , hsize_group(hsize_group)
  , vsize_group(vsize_group)
{
  connector = std::make_shared<DataConnector>(app);

  auto *layout = new QVBoxLayout();
  layout->setContentsMargins(1, 1, 1, 1);
  setLayout(layout);

  enabled_cb = new QCheckBox(tr("Enable timer"));
  connect(enabled_cb, &QCheckBox::stateChanged, this, &TimerPreferencesPanel::on_enabled_toggled);

  layout->addWidget(enabled_cb);

  auto *grid = new QGridLayout;
  layout->addLayout(grid);

  QWidget *prelude = create_prelude_panel();
  QWidget *timers = create_timers_panel();
  QWidget *option = create_options_panel();

  grid->addWidget(timers, 0, 0);
  grid->addWidget(prelude, 0, 1);
  grid->addWidget(option, 1, 0);

  grid->setColumnStretch(0, 1);
  grid->setColumnStretch(1, 1);

  connector->connect(CoreConfig::break_enabled(break_id), dc::wrap(enabled_cb));
}

auto
TimerPreferencesPanel::create_prelude_panel() -> QWidget *
{
  auto *box = new QGroupBox(tr("Break prompting"));
  auto *layout = new QVBoxLayout;
  box->setLayout(layout);

  prelude_cb = new QCheckBox(tr("Prompt before breaking"));
  layout->addWidget(prelude_cb);

  auto *max_box = new QHBoxLayout;
  has_max_prelude_cb = new QCheckBox(tr("Maximum number of prompts:"));
  max_prelude_spin = new QSpinBox;
  max_box->addWidget(has_max_prelude_cb);
  max_box->addWidget(max_prelude_spin);
  layout->addLayout(max_box);
  layout->addStretch();

  connector->connect(CoreConfig::break_max_preludes(break_id), dc::wrap(prelude_cb), [this](const auto &key, auto write) {
    return on_preludes_changed(key, write);
  });

  connector->connect(
    CoreConfig::break_max_preludes(break_id),
    dc::wrap(has_max_prelude_cb),
    [this](const auto &key, auto write) { return on_preludes_changed(key, write); },
    dc::NO_CONFIG);

  connector->connect(
    CoreConfig::break_max_preludes(break_id),
    dc::wrap(max_prelude_spin),
    [this](const auto &key, auto write) { return on_preludes_changed(key, write); },
    dc::NO_CONFIG);
  return box;
}

auto
TimerPreferencesPanel::create_options_panel() -> QWidget *
{
  auto *box = new QGroupBox(tr("Options"));
  auto *layout = new QVBoxLayout;
  box->setLayout(layout);

  ignorable_cb = new QCheckBox(tr("Show 'Postpone' button"));
  layout->addWidget(ignorable_cb);

  skippable_cb = new QCheckBox(tr("Show 'Skip' button"));
  layout->addWidget(skippable_cb);

  monitor_cb = nullptr;
  if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      monitor_cb = new QCheckBox(tr("Regard micro-breaks as activity"));
      layout->addWidget(monitor_cb);

      connector->connect(CoreConfig::timer_daily_limit_use_micro_break_activity(), dc::wrap(monitor_cb));
    }

  if (break_id == BREAK_ID_REST_BREAK)
    {
      auto *exercises_box = new QHBoxLayout;
      auto *exercises_lab = new QLabel(tr("Number of exercises:"));
      exercises_spin = new QSpinBox;

      exercises_box->addWidget(exercises_lab);
      exercises_box->addWidget(exercises_spin);
      layout->addLayout(exercises_box);

      connector->connect(GUIConfig::break_exercises(break_id), dc::wrap(exercises_spin));

      auto_natural_cb = new QCheckBox(tr("Start restbreak when screen is locked"));
      layout->addWidget(auto_natural_cb);

      connector->connect(GUIConfig::break_auto_natural(break_id), dc::wrap(auto_natural_cb));
    }

  layout->addStretch();

  connector->connect(GUIConfig::break_ignorable(break_id), dc::wrap(ignorable_cb));
  connector->connect(GUIConfig::break_skippable(break_id), dc::wrap(skippable_cb));
  return box;
}

auto
TimerPreferencesPanel::create_timers_panel() -> QWidget *
{
  auto *box = new QGroupBox(tr("Timers"));
  auto *layout = new QGridLayout;
  box->setLayout(layout);

  int row = 0;

  limit_tim = new TimeEntry();
  auto *limit_lab = new QLabel(break_id == BREAK_ID_DAILY_LIMIT ? tr("Time before end:") : tr("Time between breaks:"));

  layout->addWidget(limit_lab, row, 0);
  layout->addWidget(limit_tim, row, 1);

  hsize_group->add_widget(limit_lab);
  row++;

  if (break_id != BREAK_ID_DAILY_LIMIT)
    {
      auto_reset_tim = new TimeEntry();
      auto *auto_reset_lab = new QLabel(tr("Break duration:"));

      layout->addWidget(auto_reset_lab, row, 0);
      layout->addWidget(auto_reset_tim, row, 1);

      hsize_group->add_widget(auto_reset_lab);
      row++;

      connector->connect(CoreConfig::timer_auto_reset(break_id), dc::wrap(auto_reset_tim));
    }

  snooze_tim = new TimeEntry;

  auto *snooze_lab = new QLabel(tr("Postpone time:"));
  hsize_group->add_widget(snooze_lab);

  layout->addWidget(snooze_lab, row, 0);
  layout->addWidget(snooze_tim, row, 1);
  row++;

  layout->addWidget(new QWidget, row, 0);
  layout->setRowStretch(row, 1000);

  vsize_group->add_widget(box);

  connector->connect(CoreConfig::timer_limit(break_id), dc::wrap(limit_tim));
  connector->connect(CoreConfig::timer_snooze(break_id), dc::wrap(snooze_tim));

  return box;
}

void
TimerPreferencesPanel::set_prelude_sensitivity()
{
  bool on = enabled_cb->checkState() == Qt::Checked;
  bool has_preludes = prelude_cb->checkState() == Qt::Checked;
  bool has_max = has_max_prelude_cb->checkState() == Qt::Checked;
  has_max_prelude_cb->setEnabled(has_preludes && on);
  max_prelude_spin->setEnabled(has_preludes && has_max && on);
}

auto
TimerPreferencesPanel::on_preludes_changed(const std::string &key, bool write) -> bool
{
  static bool inside = false;

  if (inside)
    {
      return true;
    }

  inside = true;

  if (write)
    {
      int mp = 0;
      if (prelude_cb->checkState() == Qt::Checked)
        {
          if (has_max_prelude_cb->checkState() == Qt::Checked)
            {
              mp = static_cast<int>(max_prelude_spin->value());
            }
          else
            {
              mp = -1;
            }
        }
      else
        {
          mp = 0;
        }
      CoreConfig::break_max_preludes(break_id).set(mp);
      set_prelude_sensitivity();
    }
  else
    {
      int value = CoreConfig::break_max_preludes(break_id)();
      if (value == -1)
        {
          prelude_cb->setCheckState(Qt::Checked);
          has_max_prelude_cb->setCheckState(Qt::Unchecked);
        }
      else if (value == 0)
        {
          prelude_cb->setCheckState(Qt::Unchecked);
          has_max_prelude_cb->setCheckState(Qt::Unchecked);
        }
      else
        {
          prelude_cb->setCheckState(Qt::Checked);
          has_max_prelude_cb->setCheckState(Qt::Checked);
          max_prelude_spin->setValue(value);
        }

      set_prelude_sensitivity();
    }

  inside = false;

  return true;
}

void
TimerPreferencesPanel::on_enabled_toggled()
{
  enable_buttons();
  set_prelude_sensitivity();
}

void
TimerPreferencesPanel::enable_buttons()
{
  bool on = enabled_cb->checkState() == Qt::Checked;

  ignorable_cb->setEnabled(on);
  skippable_cb->setEnabled(on);

  if (monitor_cb != nullptr)
    {
      monitor_cb->setEnabled(on);
    }

  prelude_cb->setEnabled(on);
  has_max_prelude_cb->setEnabled(on);
  limit_tim->setEnabled(on);
  if (break_id == BREAK_ID_REST_BREAK)
    {
      auto_reset_tim->setEnabled(on);
      auto_natural_cb->setEnabled(on);
      exercises_spin->setEnabled(on);
    }
  else
    {
      if (auto_reset_tim != nullptr)
        {
          auto_reset_tim->setEnabled(on);
        }
    }
  snooze_tim->setEnabled(on);
  max_prelude_spin->setEnabled(on);
}
