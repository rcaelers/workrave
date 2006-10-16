// StatusWindow.cc
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

#include "StatusWindow.hh"

StatusWindow::StatusWindow()
  : QDialog(0, Qt::WindowTitleHint
            | Qt::WindowSystemMenuHint)
{
  timer_box_view = new TimerBoxView();
  layout = new QVBoxLayout();
  layout->addWidget(timer_box_view);
  setFixedSize(minimumSize());
  setLayout(layout);

  timer_box_control = new TimerBoxControl("applet", *timer_box_view);
}

StatusWindow::~StatusWindow()
{
  delete timer_box_control;
}

void StatusWindow::on_heartbeat()
{
  timer_box_control->update();
}
