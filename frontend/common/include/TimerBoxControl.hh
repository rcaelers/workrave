// TimerBoxControl.hh --- All timers
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

#ifndef TIMERBOXCONTROL_HH
#define TIMERBOXCONTROL_HH

#include <string>

#include "utils/ScopedConnections.hh"

#include "ICore.hh"
#include "ITimerBoxView.hh"

class TimerBoxControl
{
public:
  TimerBoxControl(std::string name, ITimerBoxView &view);

  void init();
  void update();
  void force_cycle();
  void set_force_empty(bool s);

private:
  void update_widgets();
  void init_table();
  void init_icon();

  void load_configuration();

  void init_slot(int slot);
  void cycle_slots();


private:
  //! View
  ITimerBoxView *view;

  //! Reconfigure the panel.
  bool reconfigure;

  //! Duration of each cycle.
  int cycle_time;

  //! Positions for the break timers.
  int break_position[workrave::BREAK_ID_SIZEOF];

  //! Flags for the break timers.
  int break_flags[workrave::BREAK_ID_SIZEOF];

  //! Imminent threshold for the timers.
  int break_imminent_time[workrave::BREAK_ID_SIZEOF];

  //! Computed slot contents.
  int break_slots[workrave::BREAK_ID_SIZEOF][workrave::BREAK_ID_SIZEOF];

  //! Current cycle for each slot.
  int break_slot_cycle[workrave::BREAK_ID_SIZEOF];

  //! Name
  std::string name;

  //! Last known operation mode
  workrave::OperationMode operation_mode;

  //!
  int force_duration;

  //! Never show any timers.
  bool force_empty;

  scoped_connections connections;
};

#endif // TIMERBOXCONTROL_HH
