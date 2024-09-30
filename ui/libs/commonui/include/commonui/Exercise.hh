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

#ifndef WORKRAVE_UI_EXERCISE_HH
#define WORKRAVE_UI_EXERCISE_HH

#include <list>
#include <string>
#include <utility>
#include <memory>

struct Exercise
{
public:
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
};

class ExerciseCollection
{
public:
  using Ptr = std::shared_ptr<ExerciseCollection>;

  ExerciseCollection();
  ~ExerciseCollection() = default;

  std::list<Exercise> get_exercises();
  bool has_exercises();
  void load();

private:
  void parse_exercises(const std::string &file_name);

private:
  std::list<Exercise> exercises;
};

#endif // WORKRAVE_UI_EXERCISE_HH
