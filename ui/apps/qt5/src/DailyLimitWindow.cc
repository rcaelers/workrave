// Copyright (C) 2001 - 2015 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "DailyLimitWindow.hh"

#include "UiUtil.hh"

using namespace workrave;
using namespace workrave::utils;

DailyLimitWindow::DailyLimitWindow(QScreen *screen , BreakFlags break_flags, GUIConfig::BlockMode mode)
  : BreakWindow(screen, BREAK_ID_DAILY_LIMIT, break_flags, mode)
{
  setWindowTitle(tr("Daily limit"));
}

QWidget *
DailyLimitWindow::create_gui()
{
  QVBoxLayout *box = new QVBoxLayout;
  QWidget *widget = new QWidget;
  widget->setLayout(box);

  QString text = UiUtil::create_alert_text
    (tr("Daily limit"),
     tr("You have reached your daily limit. Please stop working\n"
       "behind the computer. If your working day is not over yet,\n"
       "find something else to do, such as reviewing a document."));
  
  QHBoxLayout *dailylimit_box = new QHBoxLayout;
  QLabel *label = new QLabel(text);
  QLabel *image = UiUtil::create_image_label("daily-limit.png");
  dailylimit_box->addWidget(image);
  dailylimit_box->addWidget(label);

  box->addLayout(dailylimit_box);

  QHBoxLayout *button_box = new QHBoxLayout;
  add_shutdown_button(button_box);
  add_lock_button(button_box);
  add_skip_button(button_box);
  add_postpone_button(button_box);

  box->addLayout(button_box);

  return widget;
}

void
DailyLimitWindow::set_progress(int value, int max_value)
{
  (void) value;
  (void) max_value;
}
