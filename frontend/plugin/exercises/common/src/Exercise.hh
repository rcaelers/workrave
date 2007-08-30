// Exercise.hh --- Exercises
//
// Copyright (C) 2002, 2003, 2004 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef EXERCISE_HH
#define EXERCISE_HH

#include "config.h"

#include <list>
#include <string>


struct Exercise
{
public:
  static bool has_exercises();

#ifdef HAVE_EXERCISES
  struct Image
  {
    std::string image;
    int duration;
    bool mirror_x;
    Image(const char *img, int dur, bool mx)
    {
      image = img;
      duration = dur;
      mirror_x = mx;
    }
  };

  std::string title;
  std::string description;
  int duration;
  std::list<Image> sequence;

public:
  static std::list<Exercise> get_exercises();

private:
  static std::string get_exercises_file_name();
  static void parse_exercises(const char *file_name, std::list<Exercise>&);
#endif // HAVE_EXERCISES
};




#endif // EXERCISE_HH
