// TimerBox.cpp --- Timer box
//
// Copyright (C) 2004, 2005, 2009, 2010 Raymond Penners <raymond@dotsphinx.com>
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
// $Id$

#include "TimerBox.h"
#include "TimeBar.h"
#include "Util.h"
#include "Icon.h"
#include "Debug.h"

using namespace workrave;

const int PADDING_X = 2;
const int PADDING_Y = 2;

TimerBox::TimerBox(HWND parent, HINSTANCE hinst, CDeskBand *deskband)
{
  const char *icon_ids[] = { "micropause", "restbreak", "dailylimit" };
  sheep_icon = new Icon(parent, hinst, "workrave", deskband);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      slot_to_time_bar[i] = new TimeBar(parent, hinst, deskband);
      break_to_icon[i] = new Icon(parent, hinst, icon_ids[i], deskband);

      break_visible[i] = false;
      slot_to_break[i] = BREAK_ID_NONE;
      break_to_slot[i] = -1;
    }
  filled_slots = 0;
  enabled = false;
  parent_window = parent;
}


TimerBox::~TimerBox()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      delete break_to_icon[i];
      delete slot_to_time_bar[i];
    }
  delete sheep_icon;
}


void
TimerBox::set_slot(int slot, BreakId brk)
{
	TRACE_ENTER_MSG("TimerBox::set_slot", slot << " " << brk);
  BreakId old_brk = slot_to_break[slot];
  TRACE_MSG(old_brk);
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
  TRACE_EXIT();
}


void
TimerBox::set_size(int w, int h)
{
	TRACE_ENTER_MSG("TimerBox::set_size", w << " " << h);
  width = w;
  height = h;
  TRACE_EXIT();
}


void
TimerBox::update(bool repaint)
{
	TRACE_ENTER_MSG("TimerBox::update", repaint);
  TransparentDamageControl ctrl;
  TRACE_MSG("begin paint");
  ctrl.BeginPaint(repaint);
  TRACE_MSG("bars");
  update_time_bars(ctrl);
  TRACE_MSG("sheep");
  update_sheep(ctrl);
  TRACE_MSG("end paint");
  ctrl.EndPaint();
  TRACE_EXIT();
}


void
TimerBox::update_sheep(TransparentDamageControl &ctrl)
{
  TRACE_ENTER("TimerBox::update_sheep");
  if (enabled && (filled_slots != 0))
    {
      TRACE_MSG("hide");
      ctrl.HideWindow(sheep_icon->get_handle());
    }
  else
    {
      TRACE_MSG("show");
      int w, h;
      sheep_icon->get_size(w, h);
      int x = (width - w)/2;
      int y = (height- h)/2;
      ctrl.ShowWindow(sheep_icon->get_handle(), x, y);
    }
  TRACE_EXIT();
}


void
TimerBox::update_time_bars(TransparentDamageControl &ctrl)
{
  TRACE_ENTER("TimerBox::update_time_bars");
  if (enabled)
    {
      int x = 0, y = 0;
      int bar_w, bar_h;
      int icon_width, icon_height;

      TRACE_MSG("1");
      slot_to_time_bar[0]->get_size(bar_w, bar_h);
      break_to_icon[0]->get_size(icon_width, icon_height);
      TRACE_MSG("2");
      int icon_bar_w = icon_width + 2 * PADDING_X + bar_w;
      int max_columns = __max(1, width / icon_bar_w);
      int max_rows = __max(1, (height + PADDING_Y) / (__max(icon_height, bar_h) + PADDING_Y));
      int rows = __max(1, __min(max_rows, (filled_slots + max_columns - 1) / max_columns));
      int columns = (filled_slots + rows -1) / rows;
      TRACE_MSG("3");
      
      int box_h = rows * __max(icon_height, bar_h) + (rows - 1) * PADDING_Y;
      y = __max(0, (height - box_h)/2);

      int icon_dy = 0;
      int bar_dy = 0;

      TRACE_MSG("4");
      if (bar_h > icon_height)
        {
          icon_dy = (bar_h - icon_height + 1) / 2;
        }
      else
        {
          bar_dy = (icon_height - bar_h + 1) / 2;
        }
         
      TRACE_MSG("5");
       
      int current_column = 0;
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          BreakId bid = slot_to_break[i];
          TRACE_MSG("6");

          if (bid != BREAK_ID_NONE)
            {
              TRACE_MSG("7");
              TimeBar *bar = get_time_bar(bid);

              ctrl.ShowWindow(break_to_icon[bid]->get_handle(), x, y + icon_dy);
              ctrl.ShowWindow(bar->get_handle(), x+icon_width+PADDING_X, y + bar_dy);
              bar->update();
              x += icon_bar_w + 2 * PADDING_X;
              current_column++;
              if (current_column >= columns)
                {
                  current_column = 0;
                  x = 0;
                  y += __max(icon_height, bar_h) + PADDING_Y;
                }
            }
        }
    }
  
  TRACE_MSG("8");
  for (int h = 0; h < BREAK_ID_SIZEOF; h++)
    {
      if ((! enabled) || (! break_visible[h]))
        {
          ctrl.HideWindow(break_to_icon[h]->get_handle());
        }
    }
  TRACE_MSG("9");
  for (int j = enabled ? filled_slots : 0; j < BREAK_ID_SIZEOF; j++)
    {
      ctrl.HideWindow(slot_to_time_bar[j]->get_handle());
    }
  TRACE_EXIT();
}

TimeBar *
TimerBox::get_time_bar(BreakId timer)
{
  TRACE_ENTER_MSG("TimerBox::get_time_bat", timer);
  TimeBar *ret = NULL;
  int slot = break_to_slot[timer];
  if (slot >= 0)
    {
      ret = slot_to_time_bar[slot];
    }
  TRACE_EXIT();
  return ret;
}


void
TimerBox::set_enabled(bool ena)
{
  TRACE_ENTER_MSG("TimerBox::set_enabled", ena);
  enabled = ena;
  TRACE_EXIT();
}
