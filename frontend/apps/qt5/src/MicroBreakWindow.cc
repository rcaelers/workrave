// MicroBreakWindow.cc --- window for the microbreak
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

#include "MicroBreakWindow.hh"

#include "preinclude.h"

#include "nls.h"
#include "debug.hh"

#include "CoreFactory.hh"
#include "utils/AssetPath.hh"
#include "Text.hh"
#include "UiUtil.hh"
#include "Ui.hh"

using namespace workrave;
using namespace workrave::utils;
using namespace workrave::ui;

IBreakWindow::Ptr
MicroBreakWindow::create(int screen, BreakFlags break_flags, GUIConfig::BlockMode mode)
{
  return Ptr(new MicroBreakWindow(screen, break_flags, mode));
}


//! Construct a new Microbreak window.
MicroBreakWindow::MicroBreakWindow(int screen, BreakFlags break_flags, GUIConfig::BlockMode mode)
  : BreakWindow(screen, BREAK_ID_MICRO_BREAK, break_flags, mode),
    time_bar(NULL),
    label(NULL),
    progress_value(0),
    progress_max_value(0),
    fixed_size(false)
{
  setWindowTitle(QString::fromStdString(Ui::get_break_name(BREAK_ID_MICRO_BREAK)));
}

QWidget *
MicroBreakWindow::create_gui()
{
  // Time bar
  time_bar = new TimeBar;
  time_bar->set_text("Microbreak 0:32"); // FIXME:

  // Label
  label = new QLabel;

  // Icon
  QLabel *image = new QLabel;
  std::string file = AssetPath::complete_directory("micro-break.png", AssetPath::SEARCH_PATH_IMAGES);
  image->setPixmap(QPixmap(file.c_str()));

  // HBox
  QHBoxLayout *hbox = new QHBoxLayout;
  hbox->addWidget(image);
  hbox->addWidget(label);

  // Overall vbox
  QVBoxLayout *box = new QVBoxLayout;
  box->addLayout(hbox);
  box->addWidget(time_bar);

  // Button box at the bottom.
  ICore::Ptr core = CoreFactory::get_core();
  IBreak::Ptr restbreak =  core->get_break(BREAK_ID_REST_BREAK);
  if ((get_break_flags() != BREAK_FLAGS_NONE) || restbreak->is_enabled())
    {
      QHBoxLayout *button_box = new QHBoxLayout;
      if (get_break_flags() != BREAK_FLAGS_NONE)
        {
          if (restbreak->is_enabled())
            {
              button_box->addWidget(create_restbreaknow_button(false));
            }

          button_box->addStretch();

          if ((get_break_flags() & BREAK_FLAGS_SKIPPABLE) != 0)
            {
              button_box->addWidget(create_skip_button(), 0);
            }

          if ((get_break_flags() & BREAK_FLAGS_POSTPONABLE) != 0)
            {
              button_box->addWidget(create_postpone_button());
            }
        }
      else
        {
          button_box->addWidget(create_restbreaknow_button(true));
        }

      box->addLayout(button_box);
    }

  fixed_size = false;

  QWidget *widget = new QWidget;
  widget->setLayout(box);

  return widget;
}


//! Destructor.
MicroBreakWindow::~MicroBreakWindow()
{
  TRACE_ENTER("MicroBreakWindow::~MicroBreakWindow");
  TRACE_EXIT();
}



QAbstractButton *
MicroBreakWindow::create_restbreaknow_button(bool label)
{
  std::string file = AssetPath::complete_directory("timer-rest-break.png", AssetPath::SEARCH_PATH_IMAGES);
  QPixmap pixmap(file.c_str());
  QIcon icon(pixmap);

  QPushButton *button = new QPushButton(_("Rest break"));
  button->setIcon(icon);
  button->setIconSize(pixmap.rect().size());

  connect(button, &QPushButton::clicked, this, &MicroBreakWindow::on_restbreaknow_button_clicked);

  return button;
}


//! The restbreak button was clicked.
void
MicroBreakWindow::on_restbreaknow_button_clicked()
{
  // IGUI *gui = GUI::get_instance();
  //gui->restbreak_now();
}


void
MicroBreakWindow::update_time_bar()
{
  TRACE_ENTER("MicroBreakWindow::refresh_time_bar");

  time_t time = progress_max_value - progress_value;
  string s = Ui::get_break_name(BREAK_ID_MICRO_BREAK);
  s += ' ';
  s += Text::time_to_string(time);

  time_bar->set_progress(progress_value, progress_max_value - 1);
  time_bar->set_text(s);

  time_bar->update();
  TRACE_MSG(progress_value << " " << progress_max_value);
  TRACE_EXIT();
}

void
MicroBreakWindow::update_label()
{
  TRACE_ENTER("MicroBreakWindow::refresh_label");

  ICore::Ptr core = CoreFactory::get_core();

  IBreak::Ptr restbreak_timer =  core->get_break(BREAK_ID_REST_BREAK);
  IBreak::Ptr daily_timer =  core->get_break(BREAK_ID_DAILY_LIMIT);

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


  std::string txt(_("Please relax for a few seconds"));
  if (show_next == BREAK_ID_REST_BREAK)
    {
      char s[128];

      if (rb >= 0)
        {
          sprintf(s, _("Next rest break in %s"),
                  Text::time_to_string(rb, true).c_str());
        }
      else
        {
          sprintf(s, _("Rest break %s overdue"),
                  Text::time_to_string(-rb, true).c_str());
        }

      txt += "<br>";
      txt += s;
    }
  else if (show_next == BREAK_ID_DAILY_LIMIT)
    {
      char s[128];

      if (dl >= 0)
        {
          sprintf(s, _("Daily limit in %s"),
                  Text::time_to_string(dl, true).c_str());
        }
      else
        {
          sprintf(s, _("Daily limit %s overdue"),
                  Text::time_to_string(-dl, true).c_str());
        }

      txt += "<br>";
      txt += s;
    }

  label->setText(QString::fromStdString(UiUtil::create_alert_text(Ui::get_break_name(BREAK_ID_MICRO_BREAK), txt.c_str())));
  TRACE_EXIT();
}


//! Refresh window.
void
MicroBreakWindow::update_break_window()
{
  update_time_bar();
  update_label();
}


void
MicroBreakWindow::set_progress(int value, int max_value)
{
  progress_max_value = max_value;
  progress_value = value;
}
