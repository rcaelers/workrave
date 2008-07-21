// TimerBox.cpp --- Timer box
//
// Copyright (C) 2004, 2005 Raymond Penners <raymond@dotsphinx.com>
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
TimerBox::update(bool repaint)
{
  TransparentDamageControl ctrl;
  ctrl.BeginPaint(repaint);
  update_time_bars(ctrl);
  update_sheep(ctrl);
  ctrl.EndPaint();
}


void
TimerBox::update_sheep(TransparentDamageControl &ctrl)
{
  if (enabled && (filled_slots != 0))
    {
      ctrl.HideWindow(sheep_icon->get_handle());
    }
  else
    {
      int w, h;
      sheep_icon->get_size(w, h);
      int x = (width - w)/2;
      int y = (height- h)/2;
      ctrl.ShowWindow(sheep_icon->get_handle(), x, y);
    }
}


void
TimerBox::update_time_bars(TransparentDamageControl &ctrl)
{
  if (enabled)
    {
      int x = 0, y = 0;
      int bar_w, bar_h;
      int icon_width, icon_height;

      slot_to_time_bar[0]->get_size(bar_w, bar_h);
      break_to_icon[0]->get_size(icon_width, icon_height);
      int icon_bar_w = icon_width + PADDING_X + bar_w;
      int columns = __max(1, width / icon_bar_w);
      int rows = (filled_slots + columns - 1) / columns;
      int box_h = rows * __max(icon_height, bar_h) + (rows -1) * PADDING_Y;
      y = __max(0, (height - box_h)/2);

      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          BreakId bid = slot_to_break[i];

          if (bid != BREAK_ID_NONE)
            {
              TimeBar *bar = get_time_bar(bid);

              ctrl.ShowWindow(break_to_icon[bid]->get_handle(), x, y);
              ctrl.ShowWindow(bar->get_handle(), x+icon_width+PADDING_X, y);
              bar->update();
              x += icon_bar_w;
              if (x + icon_bar_w > width)
                {
                  x = 0;
                  y += __max(icon_height, bar_h) + PADDING_Y;
                }

            }
        }
    }

  for (int h = 0; h < BREAK_ID_SIZEOF; h++)
    {
      if ((! enabled) || (! break_visible[h]))
        {
          ctrl.HideWindow(break_to_icon[h]->get_handle());
        }
    }
  for (int j = enabled ? filled_slots : 0; j < BREAK_ID_SIZEOF; j++)
    {
      ctrl.HideWindow(slot_to_time_bar[j]->get_handle());
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
