// TimerBox.cpp --- Timer box
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

#include "TimerBox.h"
#include "TimeBar.h"

TimerBox::TimerBox(HWND parent, HINSTANCE hinst)
{
  const char *icon_ids[] = { "micropause", "restbreak", "dailylimit" };
  for (int i = 0; i < BREAK_ID_SIZEOF; i++) {
    time_bars[i] = new TimeBar(parent, hinst);
    break_icons[i] = CreateWindowEx
      (WS_EX_TRANSPARENT, "STATIC",
       icon_ids[i], SS_REALSIZEIMAGE | SS_ICON | WS_CHILD
       | WS_CLIPSIBLINGS | WS_BORDER, 0, 0, 16, 16, parent, NULL, hinst, NULL);

    break_time_bars[i] = NULL;
    break_visible[i] = false;
    break_slots[i] = BREAK_ID_NONE;
  }
}


TimerBox::~TimerBox() 
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++) 
    {
      DestroyWindow(break_icons[i]);
      delete time_bars[i];
    }
}


void 
TimerBox::set_slot(int slot, BreakId brk)
{
  BreakId old_brk = break_slots[slot];
  if (old_brk != brk)
    {
      break_slots[slot] = brk;
      if (brk != BREAK_ID_NONE)
        {
          break_visible[brk] = true;
          break_time_bars[brk] = time_bars[slot];
        }

      if (old_brk != BREAK_ID_NONE)
        {
          break_visible[old_brk] = false;
          break_time_bars[old_brk] = NULL;
        }
    }
}

void 
TimerBox::set_time_bar(BreakId timer, time_t value)
{
}

void 
TimerBox::set_size(int w, int h)
{
  width = w;
  height = h;
}

void 
TimerBox::update()
{
  int x = 0, y = 0;
  int bar_w, bar_h;

  time_bars[0]->get_size(bar_w, bar_h);
  int filled_slots = 0;

  for (int i = 0; i < BREAK_ID_SIZEOF; i++) 
    {
      BreakId bid = break_slots[i];

      if (bid != BREAK_ID_NONE) 
        {
          TimeBar *bar = break_time_bars[bid];
          filled_slots++;

          SetWindowPos(break_icons[bid], NULL, x, y, 0, 0,
                       SWP_SHOWWINDOW|SWP_NOZORDER|SWP_NOSIZE);
          bar->set_position(true, x+16, y);
          bar->update();
          x += 16 + bar_w;
          if (x + 16 + bar_w > width)
            {
              x = 0;
              y += bar_h;
            }

        }
    }

  for (int h = 0; h < BREAK_ID_SIZEOF; h++)
    {
      if (! break_visible[h])
        {
          ShowWindow(break_icons[h], SW_HIDE);
        }
    }
  for (int j = filled_slots; j < BREAK_ID_SIZEOF; j++) 
    {
      time_bars[j]->set_position(false, 0, 0);
    }
}



TimeBar *
TimerBox::get_time_bar(BreakId timer)
{
  return break_time_bars[timer];
}
