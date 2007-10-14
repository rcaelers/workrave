// StatusWindow.hh
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

#ifndef STATUSWINDOW_HH
#define STATUSWINDOW_HH

#include <QDialog>
#include <QVBoxLayout>
#include "TimerBoxView.hh"
#include "TimerBoxControl.hh"

class StatusWindow : public QDialog
{
  Q_OBJECT

public:
  StatusWindow();
  ~StatusWindow();

  void on_heartbeat();

private:
  TimerBoxView *timer_box_view;
  TimerBoxControl *timer_box_control;
  QVBoxLayout *layout;
};

#endif // STATUSWINDOW_HH
