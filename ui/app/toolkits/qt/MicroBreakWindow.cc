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
#  include "config.h"
#endif

#include "MicroBreakWindow.hh"

#include "utils/AssetPath.hh"
#include "debug.hh"

#include "Ui.hh"
#include "UiUtil.hh"
#include "qformat.hh"

using namespace workrave;
using namespace workrave::utils;

MicroBreakWindow::MicroBreakWindow(std::shared_ptr<IApplicationContext> app, QScreen *screen, BreakFlags break_flags)
  : BreakWindow(app, screen, BREAK_ID_MICRO_BREAK, break_flags)
  , app(app)
{
  setWindowTitle(Ui::get_break_name(BREAK_ID_MICRO_BREAK));
}

auto
MicroBreakWindow::create_gui() -> QWidget *
{
  time_bar = new TimeBar;
  time_bar->set_text("Microbreak 0:32"); // FIXME:

  label = new QLabel;
  QLabel *image = UiUtil::create_image_label("micro-break.png");

  auto *hbox = new QHBoxLayout;
  hbox->addWidget(image);
  hbox->addWidget(label);

  auto *box = new QVBoxLayout;
  box->addLayout(hbox);
  box->addWidget(time_bar);

  auto core = app->get_core();
  auto restbreak = core->get_break(BREAK_ID_REST_BREAK);

  // QHBoxLayout *button_box = new QHBoxLayout;
  // if (restbreak->is_enabled())
  //   {
  //     QPushButton *button = UiUtil::create_image_text_button("timer-rest-break.png", tr("Rest break"));
  //     connect(button, &QPushButton::clicked, this, &MicroBreakWindow::on_restbreaknow_button_clicked);
  //     button_box->addWidget(button);
  //     button_box->addStretch();
  //   }

  // box->addLayout(button_box);

  auto *widget = new QWidget;
  widget->setLayout(box);

  return widget;
}

void
MicroBreakWindow::on_restbreaknow_button_clicked()
{
  // IGUI *gui = Application::get_instance();
  // gui->restbreak_now();
}

void
MicroBreakWindow::update_label()
{
  auto core = app->get_core();

  IBreak::Ptr restbreak_timer = core->get_break(BREAK_ID_REST_BREAK);
  IBreak::Ptr daily_timer = core->get_break(BREAK_ID_DAILY_LIMIT);

  BreakId show_next = BREAK_ID_NONE;

  time_t rb = restbreak_timer->get_limit() - restbreak_timer->get_elapsed_time();
  time_t dl = daily_timer->get_limit() - daily_timer->get_elapsed_time();

  if (restbreak_timer->is_enabled())
    {
      show_next = BREAK_ID_REST_BREAK;
    }

  if (daily_timer->is_enabled())
    {
      if (show_next == BREAK_ID_NONE || dl < rb)
        {
          show_next = BREAK_ID_DAILY_LIMIT;
        }
    }

  // TODO: fix ugly std::string/QString conversionses
  QString txt(tr("Please relax for a few seconds"));
  if (show_next == BREAK_ID_REST_BREAK)
    {
      txt += "<br>";
      if (rb >= 0)
        {
          txt += qstr(qformat(tr("Next rest break in %s")) % UiUtil::time_to_string(rb, true));
        }
      else
        {
          txt += qstr(qformat(tr("Rest break %s overdue")) % UiUtil::time_to_string(-rb, true));
        }
    }
  else if (show_next == BREAK_ID_DAILY_LIMIT)
    {
      txt += "<br>";
      if (dl >= 0)
        {
          txt += qstr(qformat(tr("Daily limit in %s")) % UiUtil::time_to_string(dl, true));
        }
      else
        {
          txt += qstr(qformat(tr("Daily limit %s overdue")) % UiUtil::time_to_string(-dl, true));
        }
    }

  label->setText(UiUtil::create_alert_text(Ui::get_break_name(BREAK_ID_MICRO_BREAK), txt));
}

void
MicroBreakWindow::update_break_window()
{
  update_label();
  time_bar->update();
}

void
MicroBreakWindow::set_progress(int value, int max_value)
{
  time_t time = max_value - value;
  QString s = Ui::get_break_name(BREAK_ID_MICRO_BREAK);
  s += ' ';
  s += UiUtil::time_to_string(time);

  time_bar->set_progress(value, max_value - 1);
  time_bar->set_text(s);
}
