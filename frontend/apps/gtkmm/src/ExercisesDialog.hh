// ExercisesDialog.hh --- Exercises Dialog
//
// Copyright (C) 2002, 2003, 2007, 2013 Raymond Penners <raymond@dotsphinx.com>
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

#ifdef HAVE_CONFIG
#include "config.h"
#endif

#include "preinclude.h"
#include "Hig.hh"
#include "ExercisesPanel.hh"

class ExercisesDialog : public HigDialog
{
public:
  ExercisesDialog();
  ~ExercisesDialog();

  int run();

private:
  ExercisesPanel exercises_panel;
};

#endif // EXERCISES_DIALOG_HH
