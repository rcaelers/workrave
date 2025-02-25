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

#ifndef WORKRAVE_UI_TIMERBOXCONTROL_HH
#define WORKRAVE_UI_TIMERBOXCONTROL_HH

#include <string>

#include "utils/Signals.hh"

#include "core/ICore.hh"
#include "ui/ITimerBoxView.hh"
#include "ui/IApplicationContext.hh"

class TimerBoxControl : public workrave::utils::Trackable
{
public:
  TimerBoxControl(std::shared_ptr<workrave::ICore> core, std::string name, ITimerBoxView *view);
  ~TimerBoxControl();

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
  std::shared_ptr<workrave::ICore> core;
  ITimerBoxView *view{nullptr};
  bool reconfigure{false};
  int cycle_time{10};
  int break_position[workrave::BREAK_ID_SIZEOF]{};
  int break_flags[workrave::BREAK_ID_SIZEOF]{};
  int break_imminent_time[workrave::BREAK_ID_SIZEOF]{};
  int break_slots[workrave::BREAK_ID_SIZEOF][workrave::BREAK_ID_SIZEOF]{};
  int break_slot_cycle[workrave::BREAK_ID_SIZEOF]{};
  std::string name;
  workrave::OperationMode operation_mode{};
  int force_duration{0};
  bool force_empty{false};
};

#endif // WORKRAVE_UI_TIMERBOXCONTROL_HH
