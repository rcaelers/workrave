// TimeBar.cc
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
// $Id: IniConfigurator.hh 558 2006-02-23 19:42:12Z rcaelers $
//

#include <QStylePainter>
#include <QStyleOptionProgressBar>
#include "TimeBar.hh"
#include "debug.hh"

TimeBar::TimeBar(QWidget *parent)
  : QWidget(parent)
{
  resize(200, 30);
  setFixedSize(200, 30);
}

TimeBar::~TimeBar()
{
}

void TimeBar::paintEvent(QPaintEvent *event)
{
  QStylePainter paint(this);
  QStyleOptionProgressBar opt;
  opt.initFrom(this);
  opt.rect.setWidth(opt.rect.width()/2);
  paint.drawPrimitive(QStyle::PE_IndicatorProgressChunk, opt); 
}
   
