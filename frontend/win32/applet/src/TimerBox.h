// TimerBox.h --- Timer box
//
// Copyright (C) 2004 Raymond Penners <raymond@dotsphinx.com>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$

#ifndef TIMERBOX_H
#define TIMERBOX_H

#include <windows.h>
#include <time.h>

#include "CoreInterface.hh"
#include "Util.h"

class TimeBar;
class Icon;

class TimerBox
{
 public:
  TimerBox(HWND parent, HINSTANCE hinst);
  ~TimerBox();

  void set_slot(int slot, BreakId brk);
  TimeBar *get_time_bar(BreakId timer);
  void set_size(int width, int height);
  void update();
  void set_enabled(bool enabled);

 private:
  void update_sheep(TransparentDamageControl &ctrl);
  void update_time_bars(TransparentDamageControl &ctrl);

  TimeBar *slot_to_time_bar[BREAK_ID_SIZEOF];
  HWND parent_window;
  Icon *sheep_icon;
  Icon *break_to_icon[BREAK_ID_SIZEOF];
  BreakId slot_to_break[BREAK_ID_SIZEOF];
  short break_to_slot[BREAK_ID_SIZEOF];
  bool break_visible[BREAK_ID_SIZEOF];
  bool enabled;
  short filled_slots;
  int width;
  int height;
};

#endif // TIMERBOX_H

