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

#include <memory>
#include <boost/signals2.hpp>

#include <QtGui>
#include <QtWidgets>

#include "ui/IApplicationContext.hh"
#include "commonui/Exercise.hh"

class ExercisesPanel : public QWidget
{
  Q_OBJECT

public:
  explicit ExercisesPanel(std::shared_ptr<IApplicationContext> app, bool standalone);
  ~ExercisesPanel() override = default;

  void set_exercise_count(int num);
  auto signal_stop() -> boost::signals2::signal<void()> &;

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
  auto adjust_exercises_pointer(int inc) -> int
  {
    int ret = exercises_pointer;
    exercises_pointer += inc;
    if (!shuffled_exercises.empty())
      {
        exercises_pointer %= shuffled_exercises.size();
      }
    return ret;
  }

  SoundTheme::Ptr sound_theme;
  QTimer *timer{nullptr};
  QLabel *image{nullptr};
  QProgressBar *progress_bar{nullptr};
  QTextEdit *description_text{nullptr};
  QScrollArea *description_scroll{nullptr};
  QPushButton *pause_button{nullptr};

  std::shared_ptr<ExerciseCollection> exercises;
  std::vector<Exercise> shuffled_exercises;
  std::vector<Exercise>::const_iterator exercise_iterator;
  std::list<Exercise::Image>::const_iterator image_iterator;
  boost::signals2::signal<void()> stop_signal;

  int exercise_time{0};
  int seq_time{0};
  bool paused{false};
  bool stopped{false};
  int exercise_num{0};
  int exercise_count{0};
  static int exercises_pointer;

};

#endif // EXERCISES_PANEL_HH
