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

#include <memory>
#include <windows.h>
#include <time.h>
#include <map>

//#include "ui/UiTypes.hh"
#include "Applet.hh"

class CDeskBand;
class PaintHelper;

enum class TimerColorId
{
  Active = 0,
  Inactive,
  Overdue,
  InactiveOverActive,
  InactiveOverOverdue,
  Bg,
};

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

  void get_size(int &width, int &height) const;
  HWND get_handle() const
  {
    return hwnd;
  };

  void update_dpi();

private:
  void init_window(HWND parent, HINSTANCE hinst);
  void init_colors();
  void init_font();
  void update_size();

  static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
  LRESULT on_paint();
  void time_to_string(time_t time, char *buf, int len);

private:
  CDeskBand *deskband{nullptr};
  HWND hwnd{};
  int width{0}, height{0};
  int bar_max_value{100};
  int bar_value{0};
  int secondary_bar_max_value{0};
  int secondary_bar_value{100};
  TimerColorId secondary_bar_color{TimerColorId::Inactive};
  TimerColorId bar_color{TimerColorId::Active};
  char bar_text[APPLET_BAR_TEXT_MAX_LENGTH]{
    0,
  };
  std::shared_ptr<PaintHelper> paint_helper;

  HFONT bar_font;
  std::map<TimerColorId, HBRUSH> bar_colors;
  UINT dpi{96};
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
