// Copyright (C) 2002, 2003, 2007 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef EXERCISES_DIALOG_HH
#define EXERCISES_DIALOG_HH

#include "Hig.hh"
#include "ExercisesPanel.hh"

class ExercisesDialog : public HigDialog
{
public:
  ExercisesDialog(SoundTheme::Ptr sound_theme, ExerciseCollection::Ptr exercises);

  int run();

private:
#if GTK_CHECK_VERSION(4, 0, 0)
  // GTK4 dialogs have no get_action_area(); build our own box of action
  // widgets and add it via add_action_widget() instead.
  GtkCompat::Box action_box;
#endif
  ExercisesPanel exercises_panel;
};

#endif // EXERCISES_DIALOG_HH
