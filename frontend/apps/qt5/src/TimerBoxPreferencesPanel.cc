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

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include "TimerBoxPreferencesPanel.hh"

#include <QtGui>
#include <QStyle>

#include "debug.hh"
#include "nls.h"

#include "CoreConfig.hh"
#include "GUIConfig.hh"
#include "TimerBoxControl.hh"
#include "UiUtil.hh"
#include "Ui.hh"
#include "CoreFactory.hh"

using namespace workrave;
using namespace workrave::ui;
using namespace workrave::config;

TimerBoxPreferencesPanel::TimerBoxPreferencesPanel(std::string name)
  : name(name)
{
  TRACE_ENTER("TimerBoxPreferencesPanel::TimerBoxPreferencesPanel");

  connector = DataConnector::create();

  ontop_cb = new QCheckBox;
  enabled_cb = new QCheckBox();
  place_button = new QComboBox();
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      timer_display_button[i] = new QComboBox;
    }
  cycle_entry = new QSpinBox;

  init();
  init_config();

  TRACE_EXIT();
}

TimerBoxPreferencesPanel::~TimerBoxPreferencesPanel()
{
  TRACE_ENTER("TimerBoxPreferencesPanel::~TimerBoxPreferencePanel");
  TRACE_EXIT();
}

void
TimerBoxPreferencesPanel::init_config()
{
  connections.add(GUIConfig::timerbox_enabled(name).connect(boost::bind(&TimerBoxPreferencesPanel::enable_buttons, this)));

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      connections.add(CoreConfig::break_enabled(BreakId(i)).connect(boost::bind(&TimerBoxPreferencesPanel::enable_buttons, this)));
    }
}

void
TimerBoxPreferencesPanel::init_enabled()
{
  if (name == "main_window")
    {
      enabled_cb->setText(_("Show status window"));
    }
  else if (name == "applet")
    {
      enabled_cb->setText(_("Applet enabled"));
    }

  connector->connect(GUIConfig::timerbox_enabled(name), dc::wrap(enabled_cb),
                     boost::bind(&TimerBoxPreferencesPanel::on_enabled_toggled, this, _1, _2));
}

void
TimerBoxPreferencesPanel::init_ontop()
{
  ontop_cb->setText(_("The status window stays always on top of other windows"));
  connector->connect(GUIConfig::main_window_always_on_top(), dc::wrap(ontop_cb));
}

void
TimerBoxPreferencesPanel::init_placement()
{
  place_button->addItem(_("Place timers next to each other"));
  place_button->addItem(_("Place micro-break and rest break in one spot"));
  place_button->addItem(_("Place rest break and daily limit in one spot"));
  place_button->addItem(_("Place all timers in one spot"));

  int mp_slot = GUIConfig::timerbox_slot(name, BREAK_ID_MICRO_BREAK)();
  int rb_slot = GUIConfig::timerbox_slot(name, BREAK_ID_REST_BREAK)();
  int dl_slot = GUIConfig::timerbox_slot(name, BREAK_ID_DAILY_LIMIT)();
  int place = 0;
  if (mp_slot < rb_slot && rb_slot < dl_slot)
    {
      place = 0;
    }
  else if (mp_slot == rb_slot && rb_slot == dl_slot)
    {
      place = 3;
    }
  else if (mp_slot == rb_slot)
    {
      place = 1;
    }
  else
    {
      place = 2;
    }
  place_button->setCurrentIndex(place);

  void (QComboBox:: *signal)(int) = &QComboBox::currentIndexChanged;
  QObject::connect(place_button, signal, this, &TimerBoxPreferencesPanel::on_place_changed);
}

void
TimerBoxPreferencesPanel::init_cycle()
{
  cycle_entry->setMinimum(1);
  cycle_entry->setMaximum(999);
  connector->connect(GUIConfig::timerbox_cycle_time(name), dc::wrap(cycle_entry));
}

void
TimerBoxPreferencesPanel::init_timer_display()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      timer_display_button[i] = new QComboBox;
      QComboBox *button = timer_display_button[i];

      button->addItem(_("Hide"));
      button->addItem(_("Show"));
      button->addItem(_("Show only when this timer is first due"));

      connector->connect(GUIConfig::timerbox_flags(name, (BreakId) i), dc::wrap(timer_display_button[i]),
                         boost::bind(&TimerBoxPreferencesPanel::on_timer_display_changed, this, i, _1, _2));
    }
}

void
TimerBoxPreferencesPanel::init()
{
  init_enabled();
  init_ontop();
  init_placement();
  init_cycle();
  init_timer_display();

  layout = new QVBoxLayout;
  setLayout(layout);

  layout->addWidget(enabled_cb);

  QGroupBox *display_box = new QGroupBox(_("Display"));
  QVBoxLayout *display_layout = new QVBoxLayout;
  display_box->setLayout(display_layout);
  layout->addWidget(display_box);

  display_layout->addWidget(enabled_cb);

  if (name == "main_window")
    {
      display_layout->addWidget(ontop_cb);
    }

  UiUtil::add_widget(display_layout, _("Placement:"), place_button);
  UiUtil::add_widget(display_layout, _("Cycle time:"), cycle_entry);

  QGroupBox *timers_box = new QGroupBox(_("Timers"));
  QVBoxLayout *timers_layout = new QVBoxLayout;
  timers_box->setLayout(timers_layout);
  layout->addWidget(timers_box);

  UiUtil::add_widget(timers_layout, Ui::get_break_name(BREAK_ID_MICRO_BREAK), timer_display_button[0]);
  UiUtil::add_widget(timers_layout, Ui::get_break_name(BREAK_ID_REST_BREAK), timer_display_button[1]);
  UiUtil::add_widget(timers_layout, Ui::get_break_name(BREAK_ID_DAILY_LIMIT), timer_display_button[2]);

  layout->addStretch();
}


bool
TimerBoxPreferencesPanel::on_enabled_toggled(const std::string &key, bool write)
{
  if (write)
    {
      bool on = enabled_cb->checkState() == Qt::Checked;
      GUIConfig::timerbox_enabled(name).set(on);
      enable_buttons();
    }
  return false;
}


bool
TimerBoxPreferencesPanel::on_timer_display_changed(int break_id, const std::string &key, bool write)
{
  if (write)
    {
      int sel = timer_display_button[break_id]->currentIndex();
      int flags = 0;
      switch (sel)
        {
        case 0:
          flags |= GUIConfig::BREAK_HIDE;
          break;
        case 1:
          flags = 0;
          break;
        default:
          flags = GUIConfig::BREAK_WHEN_FIRST;
          break;
        }

      GUIConfig::timerbox_flags(name, (BreakId) break_id).set(flags);
      enable_buttons();
    }
  else
    {
      int flags = GUIConfig::timerbox_flags(name, (BreakId) break_id)();
      int showhide = 0;
      if (flags & GUIConfig::BREAK_HIDE)
        {
          showhide = 0;
        }
      else if (flags & GUIConfig::BREAK_WHEN_FIRST)
        {
          showhide = 2;
        }
      else
        {
          showhide = 1;
        }
      timer_display_button[break_id]->setCurrentIndex(showhide);
    }

  return true;
}

void
TimerBoxPreferencesPanel::on_place_changed()
{
  int slot[BREAK_ID_SIZEOF];
  int idx = place_button->currentIndex();
  switch (idx)
    {
    case 0:
      slot[BREAK_ID_MICRO_BREAK] = 0;
      slot[BREAK_ID_REST_BREAK] = 1;
      slot[BREAK_ID_DAILY_LIMIT] = 2;
      break;
    case 1:
      slot[BREAK_ID_MICRO_BREAK] = 0;
      slot[BREAK_ID_REST_BREAK] = 0;
      slot[BREAK_ID_DAILY_LIMIT] = 1;
      break;
    case 2:
      slot[BREAK_ID_MICRO_BREAK] = 0;
      slot[BREAK_ID_REST_BREAK] = 1;
      slot[BREAK_ID_DAILY_LIMIT] = 1;
      break;
    case 3:
      slot[BREAK_ID_MICRO_BREAK] = 0;
      slot[BREAK_ID_REST_BREAK] = 0;
      slot[BREAK_ID_DAILY_LIMIT] = 0;
      break;
    default:
      slot[BREAK_ID_MICRO_BREAK] = -1;
      slot[BREAK_ID_REST_BREAK] = -1;
      slot[BREAK_ID_DAILY_LIMIT] = -1;
    }

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      GUIConfig::timerbox_slot(name, (BreakId) i).set(slot[i]);
    }
}

void
TimerBoxPreferencesPanel::enable_buttons(void)
{
  int num_disabled = 0;
  for (auto &button : timer_display_button)
    {
      if (button->currentIndex() == 0)
        {
          num_disabled++;
        }
    }

  if (name == "applet")
    {
      bool on = enabled_cb->checkState() == Qt::Checked;

      place_button->setEnabled(on && num_disabled != 3);
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          ICore::Ptr core = CoreFactory::get_core();
          IBreak::Ptr b = core->get_break(BreakId(i));

          bool timer_on = b->is_enabled();
          timer_display_button[i]->setEnabled(on && timer_on);
        }
      cycle_entry->setEnabled(on && num_disabled != 3);

    }
  else if (name == "main_window")
    {
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          ICore::Ptr core = CoreFactory::get_core();
          IBreak::Ptr b = core->get_break(BreakId(i));
          timer_display_button[i]->setEnabled(b->is_enabled());
        }
      if (num_disabled == 3)
        {
          GUIConfig::timerbox_enabled(name).set(false);
          //enabled_cb->setCheckState(Qt::Checked);
        }
      enabled_cb->setEnabled(num_disabled != 3);
      place_button->setEnabled(num_disabled != 3);
      cycle_entry->setEnabled(num_disabled != 3);
      ontop_cb->setEnabled(num_disabled != 3);
    }
}
