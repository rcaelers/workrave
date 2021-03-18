// TimerBoxControl.hh --- All timers
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2012, 2013 Rob Caelers & Raymond Penners
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

#ifndef TIMERBOXCONTROL_HH
#define TIMERBOXCONTROL_HH

#include <string>

#include "ICore.hh"
#include "IConfiguratorListener.hh"
#include "ITimerBoxView.hh"

using namespace workrave;

class TimerBoxControl : public IConfiguratorListener
{
public:
  TimerBoxControl(std::string name, ITimerBoxView &view);
  ~TimerBoxControl() override;

  void init();
  void update();
  void force_cycle();
  void set_force_empty(bool s);

  static const std::string get_timer_config_key(std::string name, BreakId timer, const std::string &key);
  static int get_cycle_time(std::string name);
  static void set_cycle_time(std::string name, int time);
  static int get_timer_imminent_time(std::string name, BreakId timer);
  static void set_timer_imminent_time(std::string name, BreakId timer, int time);
  static int get_timer_slot(std::string name, BreakId timer);
  static void set_timer_slot(std::string name, BreakId timer, int slot);
  static int get_timer_flags(std::string name, BreakId timer);
  static void set_timer_flags(std::string name, BreakId timer, int flags);
  static bool is_enabled(std::string name);
  static void set_enabled(std::string name, bool enabled);

public:
  static const std::string CFG_KEY_TIMERBOX;
  static const std::string CFG_KEY_TIMERBOX_HORIZONTAL;
  static const std::string CFG_KEY_TIMERBOX_CYCLE_TIME;
  static const std::string CFG_KEY_TIMERBOX_POSITION;
  static const std::string CFG_KEY_TIMERBOX_FLAGS;
  static const std::string CFG_KEY_TIMERBOX_IMMINENT;
  static const std::string CFG_KEY_TIMERBOX_ENABLED;

  enum SlotType
  {
    BREAK_WHEN_IMMINENT = 1,
    BREAK_WHEN_FIRST = 2,
    BREAK_SKIP = 4,
    BREAK_EXCLUSIVE = 8,
    BREAK_DEFAULT = 16,
    BREAK_HIDE = 32
  };

private:
  // IConfiguratorListener
  void config_changed_notify(const std::string &key) override;
  void update_widgets();
  void init_table();
  void init_icon();

  void read_configuration();

  void init_slot(int slot);
  void cycle_slots();

private:
  //! View
  ITimerBoxView *view{nullptr};

  //! Reconfigure the panel.
  bool reconfigure{false};

  //! Duration of each cycle.
  int cycle_time{10};

  //! Positions for the break timers.
  int break_position[BREAK_ID_SIZEOF]{};

  //! Flags for the break timers.
  int break_flags[BREAK_ID_SIZEOF]{};

  //! Imminent threshold for the timers.
  int break_imminent_time[BREAK_ID_SIZEOF]{};

  //! Computed slot contents.
  int break_slots[BREAK_ID_SIZEOF][BREAK_ID_SIZEOF]{};

  //! Current cycle for each slot.
  int break_slot_cycle[BREAK_ID_SIZEOF]{};

  //! Name
  std::string name;

  //! Last known operation mode
  OperationMode operation_mode;

  //!
  int force_duration{0};

  //! Never show any timers.
  bool force_empty{false};
};

#endif // TIMERBOXCONTROL_HH
