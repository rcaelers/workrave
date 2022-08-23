// Copyright (C) 2002 - 2013 Raymond Penners <raymond@dotsphinx.com>
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

#include "config.h"

#include <algorithm>
#include <random>

#include <cstring>
#include <gtkmm.h>

#include "ExercisesPanel.hh"
#include "GtkUtil.hh"
//#include "Application.hh"
#include "utils/AssetPath.hh"
#include "Hig.hh"
#include "commonui/nls.h"
#include "ui/SoundTheme.hh"
#include "debug.hh"

using namespace std;
using namespace workrave::utils;

static void
text_buffer_set_markup(GtkTextBuffer *buffer, const gchar *markup, gint len)
{
  GtkTextIter start, end;

  g_return_if_fail(GTK_IS_TEXT_BUFFER(buffer));
  g_return_if_fail(markup != nullptr);

  if (len < 0)
    len = (gint)strlen(markup);

  gtk_text_buffer_get_bounds(buffer, &start, &end);

  gtk_text_buffer_delete(buffer, &start, &end);

  if (len > 0)
    {
      gtk_text_buffer_get_iter_at_offset(buffer, &start, 0);
      gtk_text_buffer_insert_markup(buffer, &start, markup, len);
    }
}
// (end code to be removed)

int ExercisesPanel::exercises_pointer = 0;

ExercisesPanel::ExercisesPanel(SoundTheme::Ptr sound_theme, GtkCompat::Box *dialog_action_area)
  : GtkCompat::Box(Gtk::Orientation::HORIZONTAL, 6)
  , sound_theme(sound_theme)
  , exercises(Exercise::get_exercises())
{
  standalone = dialog_action_area != nullptr;

  copy(exercises.begin(), exercises.end(), back_inserter(shuffled_exercises));
  shuffle(shuffled_exercises.begin(), shuffled_exercises.end(), std::mt19937(std::random_device()()));

  progress_bar.set_orientation(Gtk::ORIENTATION_VERTICAL);

  description_scroll.add(description_text);
  description_scroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

  description_text.set_cursor_visible(false);
  description_text.set_wrap_mode(Gtk::WRAP_WORD);
  description_text.set_editable(false);
  image_frame.add(image);

  pause_button = Gtk::manage(new Gtk::Button());
  Gtk::Widget *description_widget;

  if (dialog_action_area != nullptr)
    {
      back_button = Gtk::manage(GtkUtil::create_custom_stock_button(_("Previous"), PREVIOUS_BUTTON_ID));
      forward_button = Gtk::manage(GtkUtil::create_custom_stock_button(_("Next"), NEXT_BUTTON_ID));
      stop_button = nullptr;

      dialog_action_area->pack_start(*back_button, false, false, 0);
      dialog_action_area->pack_start(*pause_button, false, false, 0);
      dialog_action_area->pack_start(*forward_button, false, false, 0);
      description_widget = &description_scroll;
    }
  else
    {
      back_button = Gtk::manage(GtkUtil::create_custom_stock_button(nullptr, PREVIOUS_BUTTON_ID));
      forward_button = Gtk::manage(GtkUtil::create_custom_stock_button(nullptr, NEXT_BUTTON_ID));

      stop_button = Gtk::manage(GtkUtil::create_custom_stock_button(nullptr, CLOSE_BUTTON_ID));
      stop_button->signal_clicked().connect(sigc::mem_fun(*this, &ExercisesPanel::on_stop));

      auto *button_box = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::HORIZONTAL));
      Gtk::Label *browse_label = Gtk::manage(new Gtk::Label());
      string browse_label_text = "<b>";
      browse_label_text += _("Exercises player");
      browse_label_text += ":</b>";
      browse_label->set_markup(browse_label_text);
      button_box->pack_start(*browse_label, false, false, 6);
      button_box->pack_start(*back_button, false, false, 0);
      button_box->pack_start(*pause_button, false, false, 0);
      button_box->pack_start(*forward_button, false, false, 0);
      button_box->pack_start(*stop_button, false, false, 0);
      Gtk::Alignment *button_align = Gtk::manage(new Gtk::Alignment(1.0, 0.0, 0.0, 0.0));
      button_align->add(*button_box);
      auto *description_box = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::VERTICAL));
      description_box->pack_start(description_scroll, true, true, 0);
      description_box->pack_start(*button_align, false, false, 0);
      description_widget = description_box;
    }

  // This is ugly, but I could not find a decent way to do this otherwise.
  //  size_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_BOTH);
  //  size_group->add_widget(image_frame);
  //  size_group->add_widget(*description_widget);
  image.set_size_request(250, 250);
  description_scroll.set_size_request(250, 200);
  // (end of ugly)

  back_button->signal_clicked().connect(sigc::mem_fun(*this, &ExercisesPanel::on_go_back));

  forward_button->signal_clicked().connect(sigc::mem_fun(*this, &ExercisesPanel::on_go_forward));

  pause_button->signal_clicked().connect(sigc::mem_fun(*this, &ExercisesPanel::on_pause));

  back_button->set_tooltip_text(_("Previous exercise"));
  forward_button->set_tooltip_text(_("Next exercise"));
  pause_button->set_tooltip_text(_("Pause exercises"));

  if (stop_button != nullptr)
    {
      stop_button->set_tooltip_text(_("End exercises"));
    }

  pack_start(image_frame, false, false, 0);
  pack_start(progress_bar, false, false, 0);
  pack_start(*description_widget, false, false, 0);

  heartbeat_signal = Glib::signal_timeout().connect(sigc::mem_fun(*this, &ExercisesPanel::heartbeat), 1000);

  exercise_count = 0;
  reset();
}

void
ExercisesPanel::on_realize()
{
  GtkCompat::Box::on_realize();
  Glib::RefPtr<Gtk::StyleContext> style_context = get_style_context();

  style_context->context_save();
  style_context->set_state((Gtk::StateFlags)0);
  style_context->add_class(GTK_STYLE_CLASS_BACKGROUND);
  description_text.override_background_color(get_style_context()->get_background_color());
  style_context->context_restore();
}

ExercisesPanel::~ExercisesPanel()
{
  TRACE_ENTRY();
  if (heartbeat_signal.connected())
    {
      heartbeat_signal.disconnect();
    }
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

      Glib::RefPtr<Gtk::TextBuffer> buf = description_text.get_buffer();
      std::string txt = HigUtil::create_alert_text(exercise.title.c_str(), exercise.description.c_str());
      text_buffer_set_markup(buf->gobj(), txt.c_str(), static_cast<gint>(txt.length()));
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
  string file = AssetPath::complete_directory(img.image, AssetPath::SEARCH_PATH_EXERCISES);
  if (!img.mirror_x)
    {
      image.set(file);
    }
  else
    {
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(file);
      Glib::RefPtr<Gdk::Pixbuf> flip = GtkUtil::flip_pixbuf(pixbuf, true, false);
      image.set(flip);
    }
}

void
ExercisesPanel::refresh_sequence()
{
  TRACE_ENTRY();
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
          sound_theme->play_sound(SoundEvent::ExerciseStep);
        }
    }
}

void
ExercisesPanel::refresh_progress()
{
  const Exercise &exercise = *exercise_iterator;
  progress_bar.set_fraction(1.0 - (double)exercise_time / exercise.duration);
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
  const char *icon = paused ? EXECUTE_BUTTON_ID : STOP_BUTTON_ID;
  const char *label = paused ? _("Resume") : _("Pause");
  GtkUtil::update_custom_stock_button(pause_button, standalone ? label : nullptr, icon);
  if (paused)
    pause_button->set_tooltip_text(_("Resume exercises"));
  else
    pause_button->set_tooltip_text(_("Pause exercises"));
}

void
ExercisesPanel::on_pause()
{
  paused = !paused;
  refresh_pause();
}

bool
ExercisesPanel::heartbeat()
{
  if (paused || stopped)
    return false;

  if (shuffled_exercises.size() == 0)
    return false;

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
  return true;
}

void
ExercisesPanel::set_exercise_count(int num)
{
  exercise_count = num;
}
