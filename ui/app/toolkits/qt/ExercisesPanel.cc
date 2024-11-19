// Copyright (C) 2002 - 2013 Raymond Penners <raymond@dotsphinx.com>
// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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


#include "ExercisesPanel.hh"

#include <random>
#include <memory>

#include "debug.hh"

#include "utils/AssetPath.hh"
#include "ui/IApplicationContext.hh"

#include "UiUtil.hh"

using namespace workrave::utils;

int ExercisesPanel::exercises_pointer = 0;

ExercisesPanel::ExercisesPanel(std::shared_ptr<IApplicationContext> app, bool standalone)
  : sound_theme(app->get_sound_theme())
  , exercises(app->get_exercises())
{
  auto exercise_list = exercises->get_exercises();

  copy(exercise_list.begin(), exercise_list.end(), back_inserter(shuffled_exercises));

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(shuffled_exercises.begin(), shuffled_exercises.end(), g);

  auto *box = new QGridLayout;

  image = new QLabel;
  image->setFrameShape(QFrame::Panel);
  box->addWidget(image, 0, 0);

  progress_bar = new QProgressBar;
  progress_bar->setOrientation(Qt::Vertical);
  progress_bar->setTextVisible(false);

  box->addWidget(progress_bar, 0, 1);

  description_text = new QTextEdit;
  description_text->setWordWrapMode(QTextOption::WordWrap);
  description_text->setReadOnly(true);

  description_scroll = new QScrollArea;
  description_scroll->setWidget(description_text);
  description_scroll->setWidgetResizable(true);

  pause_button = new QPushButton;

  auto *back_button = new QPushButton;
  back_button->setIcon(QIcon::fromTheme("go-previous", UiUtil::create_icon("go-previous-symbolic.svg")));

  auto *forward_button = new QPushButton;
  forward_button->setIcon(QIcon::fromTheme("go-next", UiUtil::create_icon("go-next-symbolic.svg")));

  auto *stop_button = new QPushButton;
  stop_button->setIcon(QIcon::fromTheme("window-close", UiUtil::create_icon("window-close-symbolic.svg")));

  if (standalone)
    {
      auto *button_box = new QHBoxLayout();

      button_box->addWidget(stop_button);
      button_box->addWidget(back_button);
      button_box->addWidget(pause_button);
      button_box->addWidget(forward_button);

      box->addWidget(description_scroll, 0, 2);
      box->addLayout(button_box, 1, 0, 1, 3);
    }
  else
    {
      auto *button_box = new QHBoxLayout;
      auto *browse_label = new QLabel;

      QString browse_label_text = "<b>";
      browse_label_text += tr("Exercises player");
      browse_label_text += ":</b>";
      browse_label->setText(browse_label_text);

      button_box->addWidget(browse_label);
      button_box->addWidget(stop_button);
      button_box->addWidget(back_button);
      button_box->addWidget(pause_button);
      button_box->addWidget(forward_button);

      auto *description_box = new QVBoxLayout;
      description_box->addWidget(description_scroll);
      description_box->addLayout(button_box);

      box->addLayout(description_box, 0, 2);
    }

  connect(back_button, &QPushButton::clicked, this, &ExercisesPanel::on_go_back);
  connect(forward_button, &QPushButton::clicked, this, &ExercisesPanel::on_go_forward);
  connect(pause_button, &QPushButton::clicked, this, &ExercisesPanel::on_pause);
  connect(stop_button, &QPushButton::clicked, this, &ExercisesPanel::on_stop);

  back_button->setToolTip(tr("Previous exercise"));
  forward_button->setToolTip(tr("Next exercise"));
  pause_button->setToolTip(tr("Pause exercises"));
  stop_button->setToolTip(tr("End exercises"));

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(heartbeat()));
  timer->start(1000);

  reset();

  setLayout(box);
}

void
ExercisesPanel::reset()
{
  int i = adjust_exercises_pointer(1);
  exercise_iterator = shuffled_exercises.begin();
  while (i > 0)
    {
      exercise_iterator++;
      i--;
    }
  exercise_num = 0;
  paused = false;
  stopped = false;
  refresh_pause();
  start_exercise();
}

void
ExercisesPanel::start_exercise()
{
  if (!shuffled_exercises.empty())
    {
      const Exercise &exercise = *exercise_iterator;

      QString txt = UiUtil::create_alert_text(exercise.title.c_str(), exercise.description.c_str());

      description_text->setText(txt);
      exercise_time = 0;
      seq_time = 0;
      image_iterator = exercise.sequence.end();
      refresh_progress();
      refresh_sequence();
    }
}

void
ExercisesPanel::show_image()
{
  TRACE_ENTRY();
  const Exercise::Image &img = (*image_iterator);
  seq_time += img.duration;
  TRACE_MSG("image= {}", img.image);
  std::string file = AssetPath::complete_directory(img.image, SearchPathId::Exercises);
  if (!img.mirror_x)
    {
      image->setPixmap(QPixmap(file.c_str()));
    }
  else
    {
      QPixmap pixmap(file.c_str());
      image->setPixmap(pixmap.transformed(QTransform::fromScale(-1, 1)));
    }
}

void
ExercisesPanel::refresh_sequence()
{
  TRACE_ENTRY();
  const Exercise &exercise = *exercise_iterator;
  if (exercise_time >= seq_time && !exercise.sequence.empty())
    {
      if (image_iterator == exercise.sequence.end())
        {
          image_iterator = exercise.sequence.begin();
        }
      else
        {
          image_iterator++;
          if (image_iterator == exercise.sequence.end())
            {
              image_iterator = exercise.sequence.begin();
            }
        }

      show_image();
      if (exercise_time != 0)
        {
          sound_theme->play_sound(SoundEvent::ExerciseStep);
        }
    }
}

void
ExercisesPanel::refresh_progress()
{
  const Exercise &exercise = *exercise_iterator;
  progress_bar->setRange(0, exercise.duration);
  progress_bar->setValue(exercise.duration - exercise_time);
  progress_bar->update();
}

void
ExercisesPanel::on_stop()
{
  if (!stopped)
    {
      stopped = true;
      stop_signal();
    }
}

void
ExercisesPanel::on_go_back()
{
  adjust_exercises_pointer(-1);
  if (exercise_iterator == shuffled_exercises.begin())
    {
      exercise_iterator = --(shuffled_exercises.end());
    }
  else
    {
      exercise_iterator--;
    }
  start_exercise();
}

void
ExercisesPanel::on_go_forward()
{
  adjust_exercises_pointer(1);
  exercise_iterator++;
  if (exercise_iterator == shuffled_exercises.end())
    {
      exercise_iterator = shuffled_exercises.begin();
    }
  start_exercise();
}

void
ExercisesPanel::refresh_pause()
{
  if (paused)
    {
      pause_button->setIcon(QIcon::fromTheme("media-playback-pause", UiUtil::create_icon("media-playback-pause-symbolic.svg")));
    }
  else
    {
      pause_button->setIcon(QIcon::fromTheme("media-playback-start", UiUtil::create_icon("media-playback-start-symbolic.svg")));
    }

  if (paused)
    {
      pause_button->setToolTip(tr("Resume exercises"));
    }
  else
    {
      pause_button->setToolTip(tr("Pause exercises"));
    }
}

void
ExercisesPanel::on_pause()
{
  paused = !paused;
  refresh_pause();
}

void
ExercisesPanel::heartbeat()
{
  if (paused || stopped)
    {
      return;
    }

  if (shuffled_exercises.empty())
    {
      return;
    }

  const Exercise &exercise = *exercise_iterator;
  exercise_time++;
  if (exercise_time >= exercise.duration)
    {
      on_go_forward();
      exercise_num++;
      if (exercise_num == exercise_count)
        {
          on_stop();
        }
      sound_theme->play_sound(stopped ? SoundEvent::ExercisesEnded : SoundEvent::ExerciseEnded);
    }
  else
    {
      refresh_sequence();
      refresh_progress();
    }
}

void
ExercisesPanel::set_exercise_count(int num)
{
  exercise_count = num;
}

boost::signals2::signal<void()> &
ExercisesPanel::signal_stop()
{
  return stop_signal;
}
