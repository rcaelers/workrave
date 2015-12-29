// Copyright (C) 2002 - 2011, 2013 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef EXERCISES_PANEL_HH
#define EXERCISES_PANEL_HH

#ifdef HAVE_CONFIG
#include "config.h"
#endif

#include <memory>
#include <boost/signals2.hpp>

#include <QtGui>
#include <QtWidgets>

#include "commonui/SoundTheme.hh"
#include "commonui/Exercise.hh"

class ExercisesPanel : public QWidget
{
  Q_OBJECT

public:
  explicit ExercisesPanel(SoundTheme::Ptr sound_theme, bool standalone);

  void set_exercise_count(int num);
  boost::signals2::signal<void()> &signal_stop();

public Q_SLOTS:
  void heartbeat();

private:
  void reset();
  void on_go_back();
  void on_go_forward();
  void on_pause();
  void on_stop();
  void start_exercise();
  void show_image();
  void refresh_progress();
  void refresh_sequence();
  void refresh_pause();
  int adjust_exercises_pointer(int inc)
  {
    int ret = exercises_pointer;
    exercises_pointer += inc;
    if (shuffled_exercises.size() != 0)
      {
        exercises_pointer %= shuffled_exercises.size();
      }
    return ret;
  }

  SoundTheme::Ptr sound_theme;
  QTimer *timer;
  QLabel *image;
  QProgressBar *progress_bar;
  QTextEdit *description_text;
  QScrollArea *description_scroll;
  QPushButton *pause_button;

  const std::list<Exercise> exercises;
  std::vector<Exercise> shuffled_exercises;
  std::vector<Exercise>::const_iterator exercise_iterator;
  std::list<Exercise::Image>::const_iterator image_iterator;

  int exercise_time;
  int seq_time;
  bool paused;
  bool stopped;
  int exercise_num;
  int exercise_count;
  static int exercises_pointer;

  boost::signals2::signal<void()> stop_signal;
};

#endif // EXERCISES_PANEL_HH
