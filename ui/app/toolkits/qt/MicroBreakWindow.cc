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

  // if ((break_flags != BREAK_FLAGS_NONE) || restbreak->is_enabled())
  //   {
  //     Gtk::HBox *button_box = nullptr;
  //     if (break_flags != BREAK_FLAGS_NONE)
  //       {
  //         button_box = Gtk::manage(new Gtk::HBox(false, 6));

  //         Gtk::HBox *bbox = Gtk::manage(new Gtk::HBox(true, 6));

  //         if ((break_flags & BREAK_FLAGS_POSTPONABLE) != 0)
  //           {
  //             Gtk::Button *postpone_button = create_postpone_button();
  //             bbox->pack_end(*postpone_button, Gtk::PACK_EXPAND_WIDGET, 0);
  //           }

  //         if ((break_flags & BREAK_FLAGS_SKIPPABLE) != 0)
  //           {
  //             Gtk::Button *skip_button = create_skip_button();
  //             bbox->pack_end(*skip_button, Gtk::PACK_EXPAND_WIDGET, 0);
  //           }

  //         Gtk::Alignment *bboxa = Gtk::manage(new Gtk::Alignment(1.0, 0.0, 0.0, 0.0));
  //         bboxa->add(*bbox);

  //         if (restbreak->is_enabled())
  //           {
  //             button_box->pack_start(*Gtk::manage(create_restbreaknow_button(false)), Gtk::PACK_SHRINK, 0);
  //           }
  //         button_box->pack_end(*bboxa, Gtk::PACK_EXPAND_WIDGET, 0);
  //       }
  //     else
  //       {
  //         button_box = Gtk::manage(new Gtk::HBox(false, 6));
  //         button_box->pack_end(*Gtk::manage(create_restbreaknow_button(true)), Gtk::PACK_SHRINK, 0);
  //       }
  //     box->pack_start(*button_box, Gtk::PACK_EXPAND_WIDGET, 0);
  //   }

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

// Moved to BreakWindow?
// void
// MicroBreakWindow::update_time_bar()
// {
//   TRACE_ENTRY();
//   time_t time = progress_max_value - progress_value;
//   string s = _("Micro-break");
//   s += ' ';
//   s += Text::time_to_string(time);

//   time_bar->set_progress(progress_value, progress_max_value - 1);
//   time_bar->set_text(s);

//   auto core = app->get_core();
//   bool user_active = core->is_user_active();
//   if (frame != nullptr)
//     {
//       if (user_active && !is_flashing)
//         {
//           frame->set_frame_color(Gdk::Color("orange"));
//           frame->set_frame_visible(true);
//           frame->set_frame_flashing(500);
//           is_flashing = true;
//         }
//       else if (!user_active && is_flashing)
//         {
//           frame->set_frame_flashing(0);
//           frame->set_frame_visible(false);
//           is_flashing = false;
//         }
//     }
//   time_bar->update();
//   TRACE_VAR(progress_value, progress_max_value);
// }

void
MicroBreakWindow::update_label()
{
  TRACE_ENTRY();
  auto core = app->get_core();
  auto restbreak_timer = core->get_break(BREAK_ID_REST_BREAK);
  auto daily_timer = core->get_break(BREAK_ID_DAILY_LIMIT);

  BreakId show_next = BREAK_ID_NONE;

  int64_t rb = restbreak_timer->get_limit() - restbreak_timer->get_elapsed_time();
  int64_t dl = daily_timer->get_limit() - daily_timer->get_elapsed_time();

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
