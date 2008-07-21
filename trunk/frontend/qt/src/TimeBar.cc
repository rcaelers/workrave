// TimeBar.cc
//
// Copyright (C) 2006, 2007 Raymond Penners <raymond@dotsphinx.com>
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

