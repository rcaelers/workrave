// Exercise.hh --- Exercises
//
// Copyright (C) 2002, 2003, 2004, 2007, 2013 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef EXERCISE_HH
#define EXERCISE_HH

#include <list>
#include <string>
#include <utility>

struct Exercise
{
public:
  static bool has_exercises();

  struct Image
  {
    std::string image;
    int duration;
    bool mirror_x;
    Image(std::string img, int dur, bool mx)
      : image(std::move(img))
      , duration(dur)
      , mirror_x(mx)
    {
    }
  };

  std::string title;
  std::string description;
  int duration{0};
  std::list<Image> sequence;

public:
  static std::list<Exercise> get_exercises();

private:
  static std::string get_exercises_file_name();
  static void parse_exercises(const char *file_name, std::list<Exercise> &);
};

#endif // EXERCISE_HH
