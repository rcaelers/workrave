// ExercisesPanel.cc --- Exercises panel
//
// Copyright (C) 2002 - 2012 Raymond Penners <raymond@dotsphinx.com>
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
#include "config.h"
#endif

#include "preinclude.h"

#include "ExercisesPanel.hh"
#include "Util.hh"
#include "UiUtil.hh"
#include "nls.h"
// #include "SoundPlayer.hh"
#include "debug.hh"

int ExercisesPanel::exercises_pointer = 0;

ExercisesPanel::ExercisesPanel(QHBoxLayout *dialog_action_area)
  : exercises(Exercise::get_exercises())
{
  standalone = dialog_action_area != NULL;

  copy(exercises.begin(), exercises.end(), back_inserter(shuffled_exercises));
  random_shuffle(shuffled_exercises.begin(), shuffled_exercises.end());

  QHBoxLayout *box = new QHBoxLayout;

  image = new QLabel;
  image->setFrameShape(QFrame::Panel);
  box->addWidget(image);
  
  progress_bar = new QProgressBar;
  progress_bar->setOrientation(Qt::Vertical);
  progress_bar->setTextVisible(false);
  
  box->addWidget(progress_bar);

  description_text = new QTextEdit;
  description_text->setWordWrapMode(QTextOption::WordWrap);
  description_text->setReadOnly(true);

  description_scroll = new QScrollArea;
  description_scroll->setWidget(description_text);
  description_scroll->setWidgetResizable(true);
  
  pause_button = new QPushButton;
  
  if (dialog_action_area != NULL)
    {
      back_button =  new QPushButton;
      back_button->setIcon(QIcon::fromTheme("go-previous"));

      forward_button =  new QPushButton;
      forward_button->setIcon(QIcon::fromTheme("go-next"));
      stop_button = NULL;

      dialog_action_area->addWidget(back_button);
      dialog_action_area->addWidget(pause_button);
      dialog_action_area->addWidget(forward_button);

      box->addWidget(description_scroll);
    }
  else
    {
      back_button = new QPushButton;
      back_button->setIcon(QIcon::fromTheme("go-previous"));

      forward_button = new QPushButton;
      forward_button->setIcon(QIcon::fromTheme("go-next"));

      stop_button = new QPushButton;
      stop_button->setIcon(QIcon::fromTheme("window-close"));

      connect(stop_button, &QPushButton::clicked, this, &ExercisesPanel::on_stop);

      QHBoxLayout *button_box = new QHBoxLayout;
      QLabel *browse_label = new QLabel;
      
      std::string browse_label_text = "<b>";
      browse_label_text += _("Exercises player");
      browse_label_text += ":</b>";
      browse_label->setText(browse_label_text.c_str());
      
      button_box->addWidget(browse_label);
      button_box->addWidget(back_button);
      button_box->addWidget(pause_button);
      button_box->addWidget(forward_button);
      button_box->addWidget(stop_button);

      QVBoxLayout *description_box = new QVBoxLayout;
      description_box->addWidget(description_scroll);
      description_box->addLayout(button_box);

      box->addLayout(description_box);
    }

  connect(back_button, &QPushButton::clicked, this, &ExercisesPanel::on_go_back);
  connect(forward_button, &QPushButton::clicked, this, &ExercisesPanel::on_go_forward);
  connect(pause_button, &QPushButton::clicked, this, &ExercisesPanel::on_pause);
  
  back_button->setToolTip(_("Previous exercise"));
  forward_button->setToolTip(_("Next exercise"));
  pause_button->setToolTip(_("Pause exercises"));

  if (stop_button != NULL)
    {
      stop_button->setToolTip(_("End exercises"));
    }

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(heartbeat()));
  timer->start(1000);
  
  exercise_count = 0;
  reset();

  setLayout(box);
}

ExercisesPanel::~ExercisesPanel()
{
  TRACE_ENTER("ExercisesPanel::~ExercisesPanel");
  timer->stop();
  delete timer;
  TRACE_EXIT();
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
  if (shuffled_exercises.size() > 0)
    {
      const Exercise &exercise = *exercise_iterator;

      std::string txt = UiUtil::create_alert_text(exercise.title.c_str(),
                                                  exercise.description.c_str());

      description_text->setText(txt.c_str());
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
  TRACE_ENTER("ExercisesPanel::show_image");

  const Exercise::Image &img = (*image_iterator);
  seq_time += img.duration;
  TRACE_MSG("image=" << img.image);
  std::string file = Util::complete_directory(img.image,
                                              Util::SEARCH_PATH_EXERCISES);
  if (! img.mirror_x)
    {
      image->setPixmap(QPixmap(file.c_str()));
    }
  else
    {
      QPixmap pixmap(file.c_str());
      image->setPixmap(pixmap.transformed(QTransform::fromScale(-1, 1)));
    }

  TRACE_EXIT();
}

void
ExercisesPanel::refresh_sequence()
{
  TRACE_ENTER("ExercisesPanel::refresh_sequence");
  const Exercise &exercise = *exercise_iterator;
  if (exercise_time >= seq_time && exercise.sequence.size() > 0)
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
          // SoundPlayer *snd = GUI::get_instance()->get_sound_player();
          // snd->play_sound(SOUND_EXERCISE_STEP);
        }
    }

  TRACE_EXIT();
}

void
ExercisesPanel::refresh_progress()
{
  const Exercise &exercise = *exercise_iterator;
  progress_bar->setMinimum(0);
  progress_bar->setMaximum(exercise.duration);
  progress_bar->setValue(exercise.duration - exercise_time);
}


void
ExercisesPanel::on_stop()
{
  if (! stopped)
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
      pause_button->setIcon(QIcon::fromTheme("media-playback-pause"));
      //pause_button->setText(_("Resume"));
    }
  else
    {
      pause_button->setIcon(QIcon::fromTheme("media-playback-start"));
      //pause_button->setText(_("Pause"));
    }
  
  if (paused)
    pause_button->setToolTip(_("Resume exercises"));
  else
    pause_button->setToolTip(_("Pause exercises"));
}


void
ExercisesPanel::on_pause()
{
  paused = ! paused;
  refresh_pause();
}


void
ExercisesPanel::heartbeat()
{
  if (paused || stopped)
    return;

  if (shuffled_exercises.size() == 0)
    return;

  const Exercise &exercise = *exercise_iterator;
  exercise_time++;
  if (exercise_time >= exercise.duration)
    {
      on_go_forward();
      //SoundPlayer *snd = GUI::get_instance()->get_sound_player();
      exercise_num++;
      if (exercise_num == exercise_count)
        {
          on_stop();
        }
      // snd->play_sound(stopped
      //                 ? SOUND_EXERCISES_ENDED
      //                 : SOUND_EXERCISE_ENDED);
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
