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
#  include "config.h"
#endif

#include "RestBreakWindow.hh"

#include "debug.hh"

#include "commonui/Exercise.hh"

#include "ExercisesPanel.hh"
#include "TimeBar.hh"
#include "UiUtil.hh"
#include "qformat.hh"

using namespace workrave;
using namespace workrave::utils;

RestBreakWindow::RestBreakWindow(std::shared_ptr<IApplicationContext> app, QScreen *screen, BreakFlags break_flags)
  : BreakWindow(app, screen, BREAK_ID_REST_BREAK, break_flags)
  , app(app)
  , sound_theme(app->get_sound_theme())
{
  setWindowTitle(tr("Rest break"));
}

auto
RestBreakWindow::create_gui() -> QWidget *
{
  auto *box = new QVBoxLayout;

  pluggable_panel = new QHBoxLayout;
  box->addLayout(pluggable_panel);

  if (((get_break_flags() & BREAK_FLAGS_NO_EXERCISES) != 0) || get_exercise_count() == 0)
    {
      install_info_panel();
    }
  else
    {
      install_exercises_panel();
    }

  timebar = new TimeBar;
  box->addWidget(timebar);

  auto *widget = new QWidget;
  widget->setLayout(box);

  return widget;
}

void
RestBreakWindow::update_break_window()
{
  timebar->update();
}

void
RestBreakWindow::set_progress(int value, int max_value)
{
  time_t time = max_value - value;
  QString text = qstr(qformat(tr("Rest break for %s")) % UiUtil::time_to_string(time, true));

  timebar->set_progress(value, max_value);
  timebar->set_text(text);
}

auto
RestBreakWindow::get_exercise_count() -> int
{
  int ret = 0;

  if (app->get_exercises()->has_exercises())
    {
      ret = GUIConfig::break_exercises(BREAK_ID_REST_BREAK)();
    }
  return ret;
}

void
RestBreakWindow::install_exercises_panel()
{
  UiUtil::clear_layout(pluggable_panel);

  auto core = app->get_core();
  core->set_insist_policy(InsistPolicy::Ignore);

  auto *exercises_panel = new ExercisesPanel(app, false);

  pluggable_panel->addWidget(exercises_panel);

  exercises_panel->set_exercise_count(get_exercise_count());
  workrave::utils::connect(exercises_panel->signal_stop(), this, [this] {
    install_info_panel();
    center();
  });
}

void
RestBreakWindow::install_info_panel()
{
  UiUtil::clear_layout(pluggable_panel);

  auto core = app->get_core();
  core->set_insist_policy(InsistPolicy::Halt);

  QString text;
  if ((get_break_flags() & BREAK_FLAGS_NATURAL) != 0)
    {
      text = UiUtil::create_alert_text(tr("Natural rest break"), tr("This is your natural rest break."));
    }
  else
    {
      text = UiUtil::create_alert_text(tr("Rest break"),
                                       tr("This is your rest break. Make sure you stand up and\n"
                                          "walk away from your computer on a regular basis. Just\n"
                                          "walk around for a few minutes, stretch, and relax."));
    }

  auto *label = new QLabel(text);
  QLabel *image = UiUtil::create_image_label("rest-break.png");

  auto *restbreak_panel = new QHBoxLayout;
  restbreak_panel->addWidget(image);
  restbreak_panel->addWidget(label);

  pluggable_panel->addLayout(restbreak_panel);

  UiUtil::invalidate(pluggable_panel);

  // BlockMode block_mode = GUIConfig::cfg_block_mode();
  // if (block_mode == BlockMode::Off &&
  //     screen == 0)
  //   {
  //     Gtk::Requisition new_size;
  //     get_preferred_size(new_size, natural_size);

  //     int width_delta = (new_size.width - old_size.width) / 2;
  //     int height_delta = (new_size.height -  old_size.height) / 2;

  //     int x, y;
  //     get_position(x, y);
  //     move(x - width_delta, y - height_delta);
  //   }
  // else
  // {
  //   center();
  // }
}
