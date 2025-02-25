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

#include <windows.h>
#include <winuser.h>

#include "TimerBox.h"
#include "TimeBar.h"
#include "Util.h"
#include "Icon.h"
#include "Debug.h"

using namespace workrave;

const int PADDING_X = 2;
const int PADDING_Y = 2;

TimerBox::TimerBox(HWND parent, HINSTANCE hinst, CDeskBand *deskband)
  : parent_window(parent)
  , hinstance(hinst)
  , deskband(deskband)
{

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      slot_to_time_bar[i] = new TimeBar(parent, hinst, deskband);
      break_to_icon[i] = nullptr;

      break_visible[i] = false;
      slot_to_break[i] = BREAK_ID_NONE;
      break_to_slot[i] = -1;
    }

  init_icons();
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
          break_to_slot[brk] = static_cast<short>(slot);
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
TimerBox::get_preferred_size(int &w, int &h) const
{
  TRACE_ENTER("TimerBox::get_preferred_size");
  w = preferred_width;
  h = preferred_height;
  TRACE_RETURN(w << " " << h);
}

void
TimerBox::get_minimum_size(int &w, int &h) const
{
  TRACE_ENTER("TimerBox::get_minimum_size");
  w = minimum_width;
  h = minimum_height;
  TRACE_RETURN(w << " " << h);
}

void
TimerBox::update(bool repaint)
{
  TRACE_ENTER_MSG("TimerBox::update", repaint);
  update_dimensions();
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
TimerBox::update_dpi()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      slot_to_time_bar[i]->update_dpi();
    }
  init_icons();
}

void
TimerBox::init_icons()
{
  TRACE_ENTER("TimerBox::init_icons");
  const char *icon_ids[] = {"micropause", "restbreak", "dailylimit"};

#if defined(_WIN64)
  UINT dpi = GetDpiForWindow(parent_window);
#else
  UINT dpi = 96;
#endif

  int scale_factor = dpi / 48;
  int size = 8 * scale_factor;

  TRACE_MSG("dpi " << dpi);
  TRACE_MSG("scale " << scale_factor << " size " << size);

  delete sheep_icon;

  sheep_icon = new Icon(parent_window, hinstance, "workrave", size, deskband);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (break_to_icon[i] != nullptr)
        {
          delete break_to_icon[i];
        }
      break_to_icon[i] = new Icon(parent_window, hinstance, icon_ids[i], size, deskband);
    }
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
      int w = 0;
      int h = 0;
      sheep_icon->get_size(w, h);
      int x = (width - w) / 2;
      int y = (height - h) / 2;
      ctrl.ShowWindow(sheep_icon->get_handle(), x, y);
    }
  TRACE_EXIT();
}

void
TimerBox::update_dimensions()
{
  TRACE_ENTER("TimerBox::update_dimensions");
  if (enabled && (filled_slots != 0))
    {
      int bar_w = 0;
      int bar_h = 0;
      int icon_width = 0;
      int icon_height = 0;

      slot_to_time_bar[0]->get_size(bar_w, bar_h);
      break_to_icon[0]->get_size(icon_width, icon_height);

      icon_bar_width = icon_width + 2 * PADDING_X + bar_w;

      int max_columns = __max(1, width / icon_bar_width);
      int max_rows = __max(1, (height + PADDING_Y) / (__max(icon_height, bar_h) + PADDING_Y));

      rows = __max(1, __min(max_rows, (filled_slots + max_columns - 1) / max_columns));
      columns = (filled_slots + rows - 1) / rows;
      int min_columns = (filled_slots + max_rows - 1) / max_rows;

      preferred_width = columns * icon_bar_width + (columns - 1) * PADDING_X;
      preferred_height = rows * __max(icon_height, bar_h) + (rows - 1) * PADDING_Y;

      minimum_width = min_columns * icon_bar_width + (min_columns - 1) * PADDING_X;
      minimum_height = preferred_height;

      TRACE_MSG("icon: " << icon_width << " " << icon_height);
      TRACE_MSG("bar: " << bar_w << " " << bar_h);
      TRACE_MSG("icon_bar_w " << icon_bar_width);
      TRACE_MSG("max r/c " << max_rows << " " << max_columns);
      TRACE_MSG("r/c " << rows << " " << columns);
      TRACE_MSG("pref" << preferred_width << " " << preferred_height);
      TRACE_MSG("min" << minimum_width << " " << minimum_height);
    }
  else
    {
      sheep_icon->get_size(preferred_width, preferred_height);
      minimum_width = preferred_width;
      minimum_height = preferred_height;
      TRACE_MSG("only sheep");
      TRACE_MSG("pref" << preferred_width << " " << preferred_height);
      TRACE_MSG("min" << minimum_width << " " << minimum_height);
    }
  TRACE_EXIT();
}

void
TimerBox::update_time_bars(TransparentDamageControl &ctrl)
{
  TRACE_ENTER("TimerBox::update_time_bars");
  if (enabled)
    {
      int x = 0;
      int y = 0;
      int bar_w = 0;
      int bar_h = 0;
      int icon_width = 0;
      int icon_height = 0;

      slot_to_time_bar[0]->get_size(bar_w, bar_h);
      break_to_icon[0]->get_size(icon_width, icon_height);

      y = __max(0, (height - preferred_height) / 2);

      int icon_dy = 0;
      int bar_dy = 0;

      if (bar_h > icon_height)
        {
          icon_dy = (bar_h - icon_height + 1) / 2;
        }
      else
        {
          bar_dy = (icon_height - bar_h + 1) / 2;
        }

      int current_column = 0;
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          BreakId bid = slot_to_break[i];

          if (bid != BREAK_ID_NONE)
            {
              TimeBar *bar = get_time_bar(bid);

              ctrl.ShowWindow(break_to_icon[bid]->get_handle(), x, y + icon_dy);
              ctrl.ShowWindow(bar->get_handle(), x + icon_width + PADDING_X, y + bar_dy);
              bar->update();
              x += icon_bar_width + 2 * PADDING_X;
              current_column++;
              if (current_column >= columns)
                {
                  current_column = 0;
                  x = 0;
                  y += __max(icon_height, bar_h) + PADDING_Y;
                }
            }
        }
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          BreakId bid = slot_to_break[i];
          break_to_icon[bid]->update();
        }
    }

  for (int h = 0; h < BREAK_ID_SIZEOF; h++)
    {
      if ((!enabled) || (!break_visible[h]))
        {
          ctrl.HideWindow(break_to_icon[h]->get_handle());
        }
    }
  for (int j = enabled ? filled_slots : 0; j < BREAK_ID_SIZEOF; j++)
    {
      ctrl.HideWindow(slot_to_time_bar[j]->get_handle());
    }
  TRACE_EXIT();
}

TimeBar *
TimerBox::get_time_bar(BreakId timer) const
{
  TRACE_ENTER_MSG("TimerBox::get_time_bat", timer);
  TimeBar *ret = nullptr;
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
