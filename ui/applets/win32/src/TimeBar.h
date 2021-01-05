// TimeBar.h --- Time bar
//
// Copyright (C) 2004, 2005, 2006, 2007 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef TIMEBAR_H
#define TIMEBAR_H

#include <windows.h>
#include <time.h>
#include <map>

#include "commonui/ITimeBar.hh"
#include "Applet.hh"

class CDeskBand;
class PaintHelper;

class TimeBar
{
public:
  TimeBar(HWND hwnd, HINSTANCE hinst, CDeskBand *deskband);
  ~TimeBar();

  void set_progress(int value, int max_value);
  void set_secondary_progress(int value, int max_value);

  void set_text(const char *text);

  void update();
  void set_bar_color(TimerColorId color);
  void set_secondary_bar_color(TimerColorId color);

  void get_size(int &width, int &height);
  HWND get_handle() const { return hwnd; };

private:
  CDeskBand *deskband;
  HWND hwnd;
  int width, height;
  int bar_max_value;
  int bar_value;
  int secondary_bar_max_value;
  int secondary_bar_value;
  TimerColorId secondary_bar_color;
  TimerColorId bar_color;
  char bar_text[APPLET_BAR_TEXT_MAX_LENGTH];
  PaintHelper *paint_helper;

  static HFONT bar_font;
  static std::map<TimerColorId, HBRUSH> bar_colors;
  static void init(HINSTANCE hinst);
  static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
  void compute_size(int &width, int &height);
  LRESULT on_paint();
  void time_to_string(time_t time, char *buf, int len);
};

struct NONCLIENTMETRICS_PRE_VISTA_STRUCT
{
  UINT cbSize;
  int iBorderWidth;
  int iScrollWidth;
  int iScrollHeight;
  int iCaptionWidth;
  int iCaptionHeight;
  LOGFONT lfCaptionFont;
  int iSmCaptionWidth;
  int iSmCaptionHeight;
  LOGFONT lfSmCaptionFont;
  int iMenuWidth;
  int iMenuHeight;
  LOGFONT lfMenuFont;
  LOGFONT lfStatusFont;
  LOGFONT lfMessageFont;
  /*
  This is a pre-vista structure for compatibility across platforms.
  Normally, when Vista is the target build (WINVER 0x0600),
  NONCLIENTMETRICS structs contain an ifdef WINVER >= 0x0600:
  int     iPaddedBorderWidth;
  */
};

#endif // TIMEBAR_H
