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
#include "utils/Enum.hh"

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

  template<>
  struct workrave::utils::enum_traits<OperationMode>
  {
    static constexpr auto min = OperationMode::Normal;
    static constexpr auto max = OperationMode::Quiet;
    static constexpr auto linear = true;
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

  template<>
  struct workrave::utils::enum_traits<UsageMode>
  {
    static constexpr auto min = UsageMode::Normal;
    static constexpr auto max = UsageMode::Reading;
    static constexpr auto linear = true;
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

  enum BreakId
  {
    BREAK_ID_NONE = -1,
    BREAK_ID_MICRO_BREAK = 0,
    BREAK_ID_REST_BREAK = 1,
    BREAK_ID_DAILY_LIMIT = 2,
  };

  template<>
  struct workrave::utils::enum_traits<BreakId>
  {
    static constexpr auto min = BREAK_ID_MICRO_BREAK;
    static constexpr auto max = BREAK_ID_DAILY_LIMIT;
    static constexpr auto linear = true;
  };

  static constexpr auto BREAK_ID_SIZEOF = workrave::utils::enum_count<BreakId>();

  inline std::ostream &operator<<(std::ostream &stream, BreakId id)
  {
    switch (id)
      {
      case BREAK_ID_NONE:
        stream << "none";
        break;
      case BREAK_ID_MICRO_BREAK:
        stream << "microbreak";
        break;
      case BREAK_ID_REST_BREAK:
        stream << "restbreak";
        break;
      case BREAK_ID_DAILY_LIMIT:
        stream << "dailylimit";
        break;
      }
    return stream;
  }

  std::string operator%(const std::string &key, BreakId id);

  enum class BreakHint
  {
    Normal = 0,

    // Break was started on user request
    UserInitiated = 1,

    // Natural break.
    NaturalBreak = 2
  };

  template<>
  struct workrave::utils::enum_traits<BreakHint>
  {
    static constexpr auto flag = true;
    static constexpr auto bits = 2;
  };

  inline std::ostream &operator<<(std::ostream &stream, BreakHint hint)
  {
    switch (hint)
      {
      case BreakHint::Normal:
        stream << "none";
        break;
      case BreakHint::UserInitiated:
        stream << "microbreak";
        break;
      case BreakHint::NaturalBreak:
        stream << "restbreak";
        break;
      }
    return stream;
  }

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
