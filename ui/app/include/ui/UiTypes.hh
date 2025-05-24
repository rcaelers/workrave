// Copyright (C) 2001 - 2021 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UI_UITYPES_HH
#define WORKRAVE_UI_UITYPES_HH

#include "utils/Enum.hh"

using BreakFlags = unsigned int;

const static unsigned int BREAK_FLAGS_NONE = 0;
const static unsigned int BREAK_FLAGS_POSTPONABLE = 1U << 0U;
const static unsigned int BREAK_FLAGS_SKIPPABLE = 1U << 1U;
const static unsigned int BREAK_FLAGS_NO_EXERCISES = 1U << 2U;
const static unsigned int BREAK_FLAGS_NATURAL = 1U << 3U;
const static unsigned int BREAK_FLAGS_USER_INITIATED = 1U << 4U;

enum class OperationModeIcon
{
  Normal,
  Quiet,
  Suspended
};

enum class TimerColorId
{
  Active = 0,
  Inactive,
  Overdue,
  ActiveDuringBreak1,
  ActiveDuringBreak2,
  InactiveOverActive,
  InactiveOverOverdue,
  Bg,
};

enum Orientation
{
  ORIENTATION_VERTICAL,
  ORIENTATION_HORIZONTAL,
};

template<>
struct workrave::utils::enum_traits<Orientation>
{
  static constexpr std::array<std::pair<std::string_view, Orientation>, 4> names{{
    {"vertical", ORIENTATION_VERTICAL},
    {"horizontal", ORIENTATION_HORIZONTAL},
  }};
};

#endif // WORKRAVE_UI_COMMON_UITYPES_HH
