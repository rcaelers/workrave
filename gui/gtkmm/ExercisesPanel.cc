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

ExercisesPanel::ExercisesPanel()
{
  Exercise::parse_exercises(exercises);
  text_view.set_editable(false);
  progress_bar.set_orientation(Gtk::PROGRESS_TOP_TO_BOTTOM);
  
  back_button = manage
    (GtkUtil::create_stock_button_without_text(Gtk::Stock::GO_BACK));
  back_button->signal_clicked()
    .connect(SigC::slot(*this, &StatisticsDialog::on_go_back));

  forward_button = manage
    (GtkUtil::create_stock_button_without_text(Gtk::Stock::GO_FORWARD));
  forward_button->signal_clicked()
    .connect(SigC::slot(*this, &StatisticsDialog::on_go_forward));

  pause_button = manage
    (GtkUtil::create_stock_button_without_text(Gtk::Stock::STOP));
  pause_button->signal_clicked()
    .connect(SigC::slot(*this, &StatisticsDialog::on_pause));

  image_frame.add(image);

  button_box.pack_start(*back_button, false, false, 0);
  button_box.pack_start(*pause_button, false, false, 0);
  button_box.pack_start(*forward_button, false, false, 0);

  image_box.pack_start(image_frame, false, false, 0);
  image_box.pack_start(button_box, false, false, 0);
  
  pack_start(image_box, false, false, 0);
  pack_start(progress_bar, false, false, 0);
  pack_start(text_view, false, false, 0);
}

ExercisesPanel::~ExercisesPanel()
{
}

void
ExercisePanel::on_go_back()
{
}

void
ExercisePanel::on_go_forward()
{
}

void
ExercisePanel::on_pause()
{
}

