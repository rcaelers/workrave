// ExercisesPanel.cc --- Exercises panel
//
// Copyright (C) 2002, 2003 Raymond Penners <raymond@dotsphinx.com>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#include "config.h"

#ifdef HAVE_EXERCISES

#include "ExercisesPanel.hh"
#include "GtkUtil.hh"
#include "GUI.hh"
#include "Util.hh"
#include "Hig.hh"
#include "nls.h"

ExercisesPanel::ExercisesPanel(Gtk::HButtonBox *dialog_action_area)
  : Gtk::HBox(false, 6)
{
  standalone = dialog_action_area != NULL;
  
  Exercise::parse_exercises(exercises);
  exercise_iterator = exercises.begin();

  progress_bar.set_orientation(Gtk::PROGRESS_BOTTOM_TO_TOP);


  description_label.set_size_request(250, 250);
  description_label.set_line_wrap(true);
  description_label.set_alignment(0.0, 0.0);
  image.set_size_request(250, 250);
  image_frame.add(image);

  image_box.pack_start(image_frame, false, false, 0);

  pause_button =  manage(new Gtk::Button());

  if (dialog_action_area != NULL)
    {
      back_button =  manage(new Gtk::Button(Gtk::Stock::GO_BACK));
      forward_button =  manage(new Gtk::Button(Gtk::Stock::GO_FORWARD));
      
      dialog_action_area->pack_start(*back_button, false, false, 0);
      dialog_action_area->pack_start(*pause_button, false, false, 0);
      dialog_action_area->pack_start(*forward_button, false, false, 0);
    }
  else
    {
      back_button =  GtkUtil::create_stock_button_without_text
        (Gtk::Stock::GO_BACK);
      forward_button =  GtkUtil::create_stock_button_without_text
        (Gtk::Stock::GO_FORWARD);
      
      Gtk::Button *stop_button =  GtkUtil::create_stock_button_without_text
        (Gtk::Stock::CLOSE);
      stop_button->signal_clicked()
        .connect(SigC::slot(*this, &ExercisesPanel::on_stop));

      Gtk::HBox *button_box = manage(new Gtk::HBox());
      Glib::RefPtr<Gtk::SizeGroup> button_size_group;
      button_size_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
      button_size_group->add_widget(*back_button);
      button_size_group->add_widget(*pause_button);
      button_size_group->add_widget(*forward_button);
      button_size_group->add_widget(*stop_button);
      button_box->pack_start(*back_button, false, false, 0);
      button_box->pack_start(*pause_button, false, false, 0);
      button_box->pack_start(*forward_button, false, false, 0);
      button_box->pack_start(*stop_button, false, false, 0);
      Gtk::Alignment *button_align
        = manage(new Gtk::Alignment(1.0, 0.0, 0.0, 0.0));
      button_align->add(*button_box);
      image_box.pack_start(*button_align, false, false, 0);
    }

  back_button->signal_clicked()
    .connect(SigC::slot(*this, &ExercisesPanel::on_go_back));

  forward_button->signal_clicked()
    .connect(SigC::slot(*this, &ExercisesPanel::on_go_forward));

  pause_button->signal_clicked()
    .connect(SigC::slot(*this, &ExercisesPanel::on_pause));

  
  pack_start(image_box, false, false, 0);
  pack_start(progress_bar, false, false, 0);
  pack_start(description_label, false, false, 0);

  paused = false;
  refresh_pause();
  start_exercise();
  
  heartbeat_signal = GUI::get_instance()->signal_heartbeat()
    .connect(SigC::slot(*this, &ExercisesPanel::heartbeat));

  exercise_count = exercise_num = 0;
}

ExercisesPanel::~ExercisesPanel()
{
  if (heartbeat_signal.connected())
    {
      heartbeat_signal.disconnect();
    }
}

void
ExercisesPanel::start_exercise()
{
  const Exercise &exercise = *exercise_iterator;
  description_label.set_markup(HigUtil::create_alert_text(exercise.title.c_str(), exercise.description.c_str()));
  exercise_time = 0;
  seq_time = 0;
  image_iterator = exercise.sequence.begin();
  refresh_progress();
  refresh_sequence();
}

void
ExercisesPanel::show_image()
{
  const Exercise::Image &img = (*image_iterator);
  seq_time += img.duration;
  string file = Util::complete_directory(img.image,
                                         Util::SEARCH_PATH_EXERCISES);
  if (! img.mirror_x)
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
  if (exercise_time >= seq_time)
    {
      image_iterator++;
      const Exercise &exercise = *exercise_iterator;
      if (image_iterator == exercise.sequence.end())
        {
          image_iterator = exercise.sequence.begin();
        }
      show_image();
    }
}

void
ExercisesPanel::refresh_progress()
{
  const Exercise &exercise = *exercise_iterator;
  progress_bar.set_fraction(1.0 - (double) exercise_time
                            / exercise.duration);
}

void
ExercisesPanel::on_stop()
{
  stop_signal();
}

void
ExercisesPanel::on_go_back()
{
  if (exercise_iterator == exercises.begin())
    {
      exercise_iterator = --(exercises.end());
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
  exercise_iterator++;
  if (exercise_iterator == exercises.end())
    {
      exercise_iterator = exercises.begin();;
    }
  start_exercise();
}

void
ExercisesPanel::refresh_pause()
{
  if (standalone)
    {
      pause_button->set_label(paused ? _("Resume") : _("Pause"));
    }
  else
    {
      Gtk::Image *img = new Gtk::Image
        (paused ? Gtk::Stock::EXECUTE : Gtk::Stock::STOP, 
         Gtk::ICON_SIZE_BUTTON);
      Gtk::manage(img);
      pause_button->remove();
      pause_button->add(*img);
      img->show_all();
    }
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
  if (paused)
    return;
  
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

#endif // HAVE_EXERCISES
