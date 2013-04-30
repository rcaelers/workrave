// DailyLimitWindow.cc --- window for the daily limit
//
// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#include "preinclude.h"
#include "nls.h"
#include "debug.hh"

#include "DailyLimitWindow.hh"
#include "UiUtil.hh"

#include "Util.hh"

using namespace workrave;

IBreakWindow::Ptr
DailyLimitWindow::create(int screen, BreakFlags break_flags, GUIConfig::BlockMode mode)
{
  return Ptr(new DailyLimitWindow(screen, break_flags, mode));
}


//! Construct a new Daily limit window.
DailyLimitWindow::DailyLimitWindow(int screen, BreakFlags break_flags, GUIConfig::BlockMode mode)
  : BreakWindow(screen, BREAK_ID_DAILY_LIMIT, break_flags, mode)
{
  setWindowTitle(_("Daily limit"));
}


QWidget *
DailyLimitWindow::create_gui()
{
  // label
  std::string txt = UiUtil::create_alert_text
    (_("Daily limit"),
     _("You have reached your daily limit. Please stop working\n"
       "behind the computer. If your working day is not over yet,\n"
       "find something else to do, such as reviewing a document."));

  QLabel *label = new QLabel;
  label->setText(txt.c_str());

  // Icon
  QLabel *image = new QLabel;
  std::string file = Util::complete_directory("daily-limit.png", Util::SEARCH_PATH_IMAGES);
  image->setPixmap(QPixmap(file.c_str()));
  
  // HBox
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget(image);
  hbox->addWidget(label);

  // Overall vbox
  QVBoxLayout *box = new QVBoxLayout;
  box->addLayout(hbox);

  // Button box at the bottom.
  QHBoxLayout *button_box = create_break_buttons(true, true);
  if (button_box)
    {
      box->addLayout(button_box);
    }

  QWidget *widget = new QWidget;
  widget->setLayout(box);
  
  return widget;
}


//! Destructor.
DailyLimitWindow::~DailyLimitWindow()
{
}


void
DailyLimitWindow::set_progress(int value, int max_value)
{
  (void) value;
  (void) max_value;
}
