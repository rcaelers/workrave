// ExercisesDialog.cc --- Exercises dialog
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_EXERCISES

#include "preinclude.h"

#include "nls.h"
#include "debug.hh"

#include <unistd.h>
#include <assert.h>

#include "ExercisesDialog.hh"
#include "ExercisesParser.hh"



ExercisesDialog::ExercisesDialog()
  : HigDialog(_("Exercises"), false, false)
{
  TRACE_ENTER("ExercisesDialog::ExercisesDialog");

  std::list<Exercise> exercises;
  ExercisesParser::parse_exercises(exercises);
  TRACE_EXIT();
}


//! Destructor.
ExercisesDialog::~ExercisesDialog()
{
  TRACE_ENTER("ExercisesDialog::~ExercisesDialog");

  TRACE_EXIT();
}

int
ExercisesDialog::run()
{
  show_all();
  return 0;
}


#endif // HAVE_EXERCISES
