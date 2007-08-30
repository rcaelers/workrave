// TimeBar.h --- Time bar
//
// Copyright (C) 2004, 2005, 2006 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef TIMEBAR_H
#define TIMEBAR_H

#include <windows.h>
#include <time.h>

#include "ITimeBar.hh"
#include "Applet.hh"

class CDeskBand;

class TimeBar
{
public:
  TimeBar(HWND hwnd, HINSTANCE hinst, CDeskBand *deskband);
  ~TimeBar();

  void set_progress(int value, int max_value);
  void set_secondary_progress(int value, int max_value);

  void set_text(const char *text);

  void update();
  void set_bar_color(ITimeBar::ColorId color);
  void set_secondary_bar_color(ITimeBar::ColorId color);

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
  ITimeBar::ColorId secondary_bar_color;
  ITimeBar::ColorId bar_color;
  char bar_text[APPLET_BAR_TEXT_MAX_LENGTH];

  static HFONT bar_font;
  static HBRUSH bar_colors[ITimeBar::COLOR_ID_SIZEOF];
  static void init(HINSTANCE hinst);
  static LRESULT CALLBACK wnd_proc(HWND hWnd, UINT uMessage, WPARAM wParam,
                                   LPARAM lParam);
  void compute_size(int &width, int &height);
  LRESULT on_paint(void);
  void time_to_string(time_t time, char *buf, int len);
};

#endif // TIMEBAR_H

