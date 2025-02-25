// Copyright (C) 2002 - 2011 Raymond Penners <raymond@dotsphinx.com>
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

#include "ui/SoundTheme.hh"
#include "commonui/Exercise.hh"

#include <gtkmm.h>
#include <sigc++/sigc++.h>

#define PREVIOUS_BUTTON_ID "media-skip-backward"
#define CLOSE_BUTTON_ID "window-close"
#define NEXT_BUTTON_ID "media-skip-forward"
#define EXECUTE_BUTTON_ID "media-playback-start"
#define STOP_BUTTON_ID "media-playback-pause"

class ExercisesPanel : public Gtk::HBox
{
public:
  using stop_signal_t = sigc::signal<void()>;

  ExercisesPanel(SoundTheme::Ptr sound_theme, ExerciseCollection::Ptr exercises, Gtk::ButtonBox *dialog_action_area);
  ~ExercisesPanel() override;

  void set_exercise_count(int num);
  stop_signal_t &signal_stop()
  {
    return stop_signal;
  }

protected:
  void on_realize() override;

private:
  void reset();
  void on_go_back();
  void on_go_forward();
  void on_pause();
  void on_stop();
  bool heartbeat();
  void start_exercise();
  void show_image();
  void refresh_progress();
  void refresh_sequence();
  void refresh_pause();
  int adjust_exercises_pointer(int inc)
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
  Gtk::Frame image_frame;
  Gtk::Image image;
  Gtk::ProgressBar progress_bar;
  Gtk::TextView description_text;
  Gtk::ScrolledWindow description_scroll;
  Gtk::Button *back_button{nullptr};
  Gtk::Button *pause_button{nullptr};
  Gtk::Button *forward_button{nullptr};
  Gtk::Button *stop_button{nullptr};
  Glib::RefPtr<Gtk::SizeGroup> size_group;
  ExerciseCollection::Ptr exercises;
  std::vector<Exercise> shuffled_exercises;
  std::vector<Exercise>::const_iterator exercise_iterator;
  std::list<Exercise::Image>::const_iterator image_iterator;
  sigc::connection heartbeat_signal;
  int exercise_time{};
  int seq_time{};
  bool paused{};
  bool stopped{};
  stop_signal_t stop_signal;
  bool standalone;
  int exercise_num{};
  int exercise_count;
  static int exercises_pointer;
};

#endif // EXERCISES_PANEL_HH
