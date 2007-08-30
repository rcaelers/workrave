// TimerBoxView.cc --- Timers
//
// Copyright (C) 2006 Raymond Penners <raymond@dotsphinx.com>
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
// $Id: ITimerBoxView.hh 680 2006-10-01 20:49:47Z dotsphinx $
//

#include "TimerBoxView.hh"

TimerBoxView::TimerBoxView()
{
  layout = new QGridLayout();
  for (size_t i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      QString s = "brk";
      break_labels[i] = new QLabel(s);
      break_bars[i] = new TimeBar();
      layout->addWidget(break_labels[i]);
      layout->addWidget(break_bars[i]);
    }
  setLayout(layout);
}

TimerBoxView::~TimerBoxView()
{
}

void
TimerBoxView::set_slot(BreakId  id, int slot)
{
}

void
TimerBoxView::set_time_bar(BreakId id,
                           std::string text,
                           ITimeBar::ColorId primary_color,
                           int primary_value, int primary_max,
                           ITimeBar::ColorId secondary_color,
                           int secondary_value, int secondary_max)
{
  QString s = text.c_str();
  //QProgressBar *b = break_bars[id];
  //b->setMaximum(primary_max);
  //b->setMinimum(0);
  //b->setValue(primary_value);
}

void
TimerBoxView::set_tip(std::string tip)
{
}

void
TimerBoxView::set_icon(IconType icon)
{
}

void
TimerBoxView::update_view()
{
}

void
TimerBoxView::set_enabled(bool enabled)
{
}
