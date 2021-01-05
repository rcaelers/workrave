// Copyright (C) 2001 - 2009, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_BACKEND_CORETYPES_HH
#define WORKRAVE_BACKEND_CORETYPES_HH

#include <iostream>

namespace workrave
{
  /* Mode */
  enum class OperationMode
  {
    /* Breaks are reported to the user when due. */
    Normal,

    /* Monitoring is suspended. */
    Suspended,

    /* Breaks are not reported to the user when due. */
    Quiet,
  };

  inline std::ostream &operator<<(std::ostream &stream, OperationMode mode)
  {
    switch (mode)
      {
      case OperationMode::Normal:
        stream << "normal";
        break;
      case OperationMode::Suspended:
        stream << "suspended";
        break;
      case OperationMode::Quiet:
        stream << "quiet";
        break;
      }
    return stream;
  }

  enum class UsageMode
  {
    /* Normal 'average' PC usage. */
    Normal,

    /* User is reading. */
    Reading,
  };

  inline std::ostream &operator<<(std::ostream &stream, UsageMode mode)
  {
    switch (mode)
      {
      case UsageMode::Normal:
        stream << "normal";
        break;
      case UsageMode::Reading:
        stream << "reading";
        break;
      }
    return stream;
  }

  using BreakId = int;

  const int BREAK_ID_NONE        = -1;
  const int BREAK_ID_MICRO_BREAK = 0;
  const int BREAK_ID_REST_BREAK  = 1;
  const int BREAK_ID_DAILY_LIMIT = 2;
  const int BREAK_ID_SIZEOF      = 3;

  enum BreakHint
  {
    BREAK_HINT_NONE = 0,

    // Break was started on user request
    BREAK_HINT_USER_INITIATED = 1,

    // Natural break.
    BREAK_HINT_NATURAL_BREAK = 2
  };

  //! The way a break is insisted.
  enum class InsistPolicy
  {
    //! Uninitialized policy
    Invalid,

    //! Halts the timer on activity.
    Halt,

    //! Resets the timer on activity.
    Reset,

    //! Ignores all activity.
    Ignore
  };

  inline std::ostream &operator<<(std::ostream &stream, InsistPolicy policy)
  {
    switch (policy)
      {
      case InsistPolicy::Invalid:
        stream << "invalid";
        break;
      case InsistPolicy::Halt:
        stream << "halt";
        break;
      case InsistPolicy::Reset:
        stream << "reset";
        break;
      case InsistPolicy::Ignore:
        stream << "ignore";
        break;
      }
    return stream;
  }

} // namespace workrave

#endif // WORKRAVE_BACKEND_CORETYPES_HH
