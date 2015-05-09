// TimerPreferencesPanel.cc
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "TimerPreferencesPanel.hh"

#include <QtGui>
#include <QStyle>

#include "debug.hh"
#include "nls.h"

#include "CoreConfig.hh"
#include "ICore.hh"
#include "CoreFactory.hh"

#include "TimeEntry.hh"

using namespace workrave;
using namespace workrave::config;

TimerPreferencesPanel::TimerPreferencesPanel(BreakId break_id, SizeGroup* hsize_group, SizeGroup* vsize_group)
  : QWidget(0, Qt::Window),
    break_id(break_id),
    hsize_group(hsize_group),
    vsize_group(vsize_group),
    enabled_cb(NULL),
    auto_natural_cb(NULL),
    has_max_prelude_cb(NULL),
    ignorable_cb(NULL),
    monitor_cb(NULL),
    prelude_cb(NULL),
    skippable_cb(NULL),
    exercises_spin(NULL),
    max_prelude_spin(NULL),
    auto_reset_tim(NULL),
    limit_tim(NULL),
    snooze_tim(NULL)
{
  TRACE_ENTER("TimerPreferencesPanel::TimerPreferencesPanel");

  connector = DataConnector::create();

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setContentsMargins(1, 1, 1, 1);
  setLayout(layout);

  enabled_cb = new QCheckBox(_("Enable timer"));
  connect(enabled_cb, &QCheckBox::stateChanged, this, &TimerPreferencesPanel::on_enabled_toggled);

  layout->addWidget(enabled_cb);

  QGridLayout *grid = new QGridLayout;
  layout->addLayout(grid);

  QWidget *prelude = create_prelude_panel();
  QWidget *timers = create_timers_panel();
  QWidget *option = create_options_panel();

  grid->addWidget(timers, 0, 0);
  grid->addWidget(prelude, 0, 1);
  grid->addWidget(option, 1, 0);

  grid->setRowStretch(0, 0);
  grid->setRowStretch(1, 1);
  grid->setColumnStretch(0, 0);
  grid->setColumnStretch(1, 1);

  connector->connect(CoreConfig::break_enabled(break_id), dc::wrap(enabled_cb));

  TRACE_EXIT();
}


//! Destructor.
TimerPreferencesPanel::~TimerPreferencesPanel()
{
  TRACE_ENTER("TimerPreferencesPanel::~TimerPreferencesPanel");
  TRACE_EXIT();
}


QWidget *
TimerPreferencesPanel::create_prelude_panel()
{
  QGroupBox *box =new QGroupBox(_("Break prompting"));
  QVBoxLayout *layout = new QVBoxLayout;
  box->setLayout(layout);

  prelude_cb = new QCheckBox(_("Prompt before breaking"));
  layout->addWidget(prelude_cb);

  QHBoxLayout *max_box = new QHBoxLayout;
  has_max_prelude_cb = new QCheckBox(_("Maximum number of prompts:"));
  max_prelude_spin = new QSpinBox;
  max_box->addWidget(has_max_prelude_cb);
  max_box->addWidget(max_prelude_spin);
  layout->addLayout(max_box);
  layout->addStretch();

  connector->connect(CoreConfig::break_max_preludes(break_id),
                     dc::wrap(prelude_cb),
                     boost::bind(&TimerPreferencesPanel::on_preludes_changed, this, _1, _2));

  connector->connect(CoreConfig::break_max_preludes(break_id),
                     dc::wrap(has_max_prelude_cb),
                     boost::bind(&TimerPreferencesPanel::on_preludes_changed, this, _1, _2),
                     dc::NO_CONFIG);

  connector->connect(CoreConfig::break_max_preludes(break_id),
                     dc::wrap(max_prelude_spin),
                     boost::bind(&TimerPreferencesPanel::on_preludes_changed, this, _1, _2),
                     dc::NO_CONFIG);
  return box;
}


QWidget *
TimerPreferencesPanel::create_options_panel()
{
  QGroupBox *box = new QGroupBox(_("Options"));
  QVBoxLayout *layout = new QVBoxLayout;
  box->setLayout(layout);

  // Ignorable
  ignorable_cb = new QCheckBox(_("Show 'Postpone' button"));
  layout->addWidget(ignorable_cb);

  // Skippable
  skippable_cb = new QCheckBox(_("Show 'Skip' button"));
  layout->addWidget(skippable_cb);

  // Break specific options
  monitor_cb = NULL;
  if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      monitor_cb = new QCheckBox(_("Regard micro-breaks as activity"));
      layout->addWidget(monitor_cb);

      connector->connect(CoreConfig::timer_daily_limit_use_micro_break_activity(), dc::wrap(monitor_cb));
    }

  if (break_id == BREAK_ID_REST_BREAK)
    {
      QHBoxLayout *exercises_box = new QHBoxLayout;
      QLabel *exercises_lab = new QLabel(_("Number of exercises:"));
      exercises_spin = new QSpinBox;

      exercises_box->addWidget(exercises_lab);
      exercises_box->addWidget(exercises_spin);
      layout->addLayout(exercises_box);

      connector->connect(GUIConfig::break_exercises(break_id), dc::wrap(exercises_spin));

      auto_natural_cb = new QCheckBox(_("Start restbreak when screen is locked"));
      layout->addWidget(auto_natural_cb);

      connector->connect(GUIConfig::break_auto_natural(break_id), dc::wrap(auto_natural_cb));
    }

  layout->addStretch();

  connector->connect(GUIConfig::break_ignorable(break_id), dc::wrap(ignorable_cb));
  connector->connect(GUIConfig::break_skippable(break_id), dc::wrap(skippable_cb));
  return box;
}

QWidget *
TimerPreferencesPanel::create_timers_panel()
{
  QGroupBox *box = new QGroupBox(_("Timers"));
  QGridLayout *layout = new QGridLayout;
  box->setLayout(layout);

  int row = 0;

  // Limit time
  limit_tim = new TimeEntry();
  QLabel *limit_lab = new QLabel(break_id == BREAK_ID_DAILY_LIMIT
                                 ? _("Time before end:")
                                 : _("Time between breaks:"));

  layout->addWidget(limit_lab, row, 0);
  layout->addWidget(limit_tim, row, 1);
  layout->setRowStretch(row, 0);

  hsize_group->addWidget(limit_lab);
  row++;

  // Auto-reset time
  if (break_id != BREAK_ID_DAILY_LIMIT)
    {
      auto_reset_tim = new TimeEntry();
      QLabel *auto_reset_lab = new QLabel(_("Break duration:"));

      layout->addWidget(auto_reset_lab, row, 0);
      layout->addWidget(auto_reset_tim, row, 1);
      layout->setRowStretch(row, 0);

      hsize_group->addWidget(auto_reset_lab);
      row++;

      connector->connect(CoreConfig::timer_auto_reset(break_id), dc::wrap(auto_reset_tim));
    }

  // Snooze time
  snooze_tim = new TimeEntry;

  QLabel *snooze_lab = new QLabel(_("Postpone time:"));
  layout->addWidget(snooze_lab, row, 0);
  layout->addWidget(snooze_tim, row, 1);
  layout->setRowStretch(row, 0);
  row++;

  layout->addWidget(new QWidget, row, 0);
  layout->setRowStretch(row, 1);

  layout->setColumnStretch(1, 0);
  hsize_group->addWidget(snooze_lab);
  vsize_group->addWidget(box);

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


bool
TimerPreferencesPanel::on_preludes_changed(const std::string &key, bool write)
{
  static bool inside = false;

  if (inside)
    return true;

  inside = true;

  if (write)
    {
      int mp;
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


//! Enable widgets
void
TimerPreferencesPanel::enable_buttons()
{
  bool on = enabled_cb->checkState() == Qt::Checked;

  ignorable_cb->setEnabled(on);
  skippable_cb->setEnabled(on);

  if (monitor_cb != NULL)
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
      if (auto_reset_tim != NULL)
        {
          auto_reset_tim->setEnabled(on);
        }
    }
  snooze_tim->setEnabled(on);
  max_prelude_spin->setEnabled(on);
}
