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

const int ICON_WIDTH = 16;
const int ICON_HEIGHT = 16;
const int PADDING_X = 2;
const int PADDING_Y = 2;

TimerBox::TimerBox(HWND parent, HINSTANCE hinst)
{
  const char *icon_ids[] = { "micropause", "restbreak", "dailylimit" };
  sheep_icon = CreateWindowEx
      (WS_EX_TRANSPARENT, "STATIC",
       "workrave", SS_REALSIZEIMAGE | SS_ICON | WS_CHILD
       | WS_CLIPSIBLINGS, 0, 0, ICON_WIDTH, ICON_HEIGHT, parent, NULL, hinst, NULL);
  for (int i = 0; i < BREAK_ID_SIZEOF; i++) {
    slot_to_time_bar[i] = new TimeBar(parent, hinst);
    break_to_icon[i] = CreateWindowEx
      (WS_EX_TRANSPARENT, "STATIC",
       icon_ids[i], SS_REALSIZEIMAGE | SS_ICON | WS_CHILD
       | WS_CLIPSIBLINGS, 0, 0, ICON_WIDTH, ICON_HEIGHT, parent, NULL, hinst, NULL);

    break_visible[i] = false;
    slot_to_break[i] = BREAK_ID_NONE;
    break_to_slot[i] = -1;
  }
  filled_slots = 0;
  enabled = false;
}


TimerBox::~TimerBox() 
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++) 
    {
      DestroyWindow(break_to_icon[i]);
      delete slot_to_time_bar[i];
    }
}


void 
TimerBox::set_slot(int slot, BreakId brk)
{
  BreakId old_brk = slot_to_break[slot];
  if (old_brk != brk)
    {
      if (old_brk != BREAK_ID_NONE)
        {
          break_visible[old_brk] = false;
          break_to_slot[old_brk] = -1;
          filled_slots--;
        }
      slot_to_break[slot] = brk;
      if (brk != BREAK_ID_NONE)
        {
          int old_slot = break_to_slot[brk];
          if (old_slot >= 0)
            {
              slot_to_break[old_slot] = BREAK_ID_NONE;
            }
          else
            {
              filled_slots++;
            }
          break_visible[brk] = true;
          break_to_slot[brk] = slot;
        }
    }
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
  update_sheep();
  update_time_bars();
}


void 
TimerBox::update_sheep()
{
  if (enabled && (filled_slots != 0))
    {
      ShowWindow(sheep_icon, SW_HIDE);
    }
  else
    {
      int x = (width - ICON_WIDTH)/2;
      int y = (height- ICON_HEIGHT)/2;
      SetWindowPos(sheep_icon, NULL, x, y, 0, 0,
                   SWP_SHOWWINDOW|SWP_NOZORDER|SWP_NOSIZE);
    }
}

void 
TimerBox::update_time_bars()
{
  if (enabled)
    {
      int x = 0, y = 0;
      int bar_w, bar_h;

      slot_to_time_bar[0]->get_size(bar_w, bar_h);
      int icon_bar_w = ICON_WIDTH + PADDING_X + bar_w;
      int columns = __max(1, width / icon_bar_w);
      int rows = (filled_slots + columns - 1) / columns;
      int box_h = rows * __max(ICON_HEIGHT, bar_h) + (rows -1) * PADDING_Y;
      y = __max(0, (height - box_h)/2);
  
      for (int i = 0; i < BREAK_ID_SIZEOF; i++) 
        {
          BreakId bid = slot_to_break[i];

          if (bid != BREAK_ID_NONE) 
            {
              TimeBar *bar = get_time_bar(bid);

              SetWindowPos(break_to_icon[bid], NULL, x, y, 0, 0,
                           SWP_SHOWWINDOW|SWP_NOZORDER|SWP_NOSIZE);
              bar->set_position(true, x+ICON_WIDTH+PADDING_X, y);
              bar->update();
              x += icon_bar_w;
              if (x + icon_bar_w > width)
                {
                  x = 0;
                  y += __max(ICON_HEIGHT, bar_h) + PADDING_Y;
                }

            }
        }
    }
  for (int h = 0; h < BREAK_ID_SIZEOF; h++)
    {
      if ((! enabled) || (! break_visible[h]))
        {
          ShowWindow(break_to_icon[h], SW_HIDE);
        }
    }
  for (int j = enabled ? filled_slots : 0; j < BREAK_ID_SIZEOF; j++) 
    {
      slot_to_time_bar[j]->set_position(false, 0, 0);
    }
}



TimeBar *
TimerBox::get_time_bar(BreakId timer)
{
  TimeBar *ret = NULL;
  int slot = break_to_slot[timer];
  if (slot >= 0)
    {
      ret = slot_to_time_bar[slot];
    }
  return ret;
}


void
TimerBox::set_enabled(bool ena)
{
  enabled = ena;
}