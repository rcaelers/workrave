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

#include "ExercisesPanel.hh"
#include "GtkUtil.hh"
#include "GUI.hh"

ExercisesPanel::ExercisesPanel(Gtk::HButtonBox *dialog_action_area)
  : back_button(Gtk::Stock::GO_BACK),
    pause_button(Gtk::Stock::STOP),
    forward_button(Gtk::Stock::GO_FORWARD)
{
  Exercise::parse_exercises(exercises);
  exercise_iterator = exercises.begin();

  progress_bar.set_orientation(Gtk::PROGRESS_TOP_TO_BOTTOM);


  back_button.signal_clicked()
    .connect(SigC::slot(*this, &ExercisesPanel::on_go_back));

  forward_button.signal_clicked()
    .connect(SigC::slot(*this, &ExercisesPanel::on_go_forward));

  pause_button.signal_clicked()
    .connect(SigC::slot(*this, &ExercisesPanel::on_pause));

  description_label.set_size_request(250, 250);
  description_label.set_line_wrap(true);
  
  image.set_size_request(250, 250);
  image_frame.add(image);

  image_box.pack_start(image_frame, false, false, 0);

  if (dialog_action_area != NULL)
    {
      dialog_action_area->pack_start(back_button, false, false, 0);
      dialog_action_area->pack_start(pause_button, false, false, 0);
      dialog_action_area->pack_start(forward_button, false, false, 0);
    }
  else
    {
      Gtk::HBox *button_box = manage(new Gtk::HBox());
      Glib::RefPtr<Gtk::SizeGroup> button_size_group;
      button_size_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
      button_size_group->add_widget(back_button);
      button_size_group->add_widget(forward_button);
      button_size_group->add_widget(pause_button);
      button_box->pack_start(back_button, false, false, 0);
      button_box->pack_start(pause_button, false, false, 0);
      button_box->pack_start(forward_button, false, false, 0);
      Gtk::Alignment *button_align
        = manage(new Gtk::Alignment(1.0, 0.0, 0.0, 0.0));
      button_align->add(*button_box);
      image_box.pack_start(*button_align, false, false, 0);
    }
  
  pack_start(image_box, false, false, 0);
  pack_start(progress_bar, false, false, 0);
  pack_start(description_label, false, false, 0);

  start_exercise();
  
  heartbeat_signal = GUI::get_instance()->signal_heartbeat()
    .connect(SigC::slot(*this, &ExercisesPanel::heartbeat));
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
  description_label.set_label(exercise.description);
  exercise_time = 0;
  progress_bar.set_fraction(0.);
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
ExercisesPanel::on_pause()
{
}

void
ExercisesPanel::heartbeat()
{
  const Exercise &exercise = *exercise_iterator;
  if (exercise_time >= exercise.duration)
    {
      on_go_forward();
    }
  else
    {
      exercise_time++;
      progress_bar.set_fraction((double) exercise_time / exercise.duration);
    }
}
