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
  // @rpc.enum(name="operation_mode")
  enum class OperationMode
  {
    /* Breaks are reported to the user when due. */
    // @rpc.enum.value(name="normal")
    Normal,

    /* Monitoring is suspended. */
    // @rpc.enum.value(name="suspended")
    Suspended,

    /* Breaks are not reported to the user when due. */
    // @rpc.enum.value(name="quiet")
    Quiet,
  };

  template<>
  struct workrave::utils::enum_traits<OperationMode>
  {
    static constexpr auto min = OperationMode::Normal;
    static constexpr auto max = OperationMode::Quiet;
    static constexpr auto linear = true;

    static constexpr std::array<std::pair<std::string_view, OperationMode>, 3> names{
      {{"normal", OperationMode::Normal}, {"suspended", OperationMode::Suspended}, {"quiet", OperationMode::Quiet}}};
  };

  // @rpc.enum(name="usage_mode")
  enum class UsageMode
  {
    /* Normal 'average' PC usage. */
    // @rpc.enum.value(name="normal")
    Normal,

    /* User is reading. */
    // @rpc.enum.value(name="reading")
    Reading,
  };

  template<>
  struct workrave::utils::enum_traits<UsageMode>
  {
    static constexpr auto min = UsageMode::Normal;
    static constexpr auto max = UsageMode::Reading;
    static constexpr auto linear = true;

    static constexpr std::array<std::pair<std::string_view, UsageMode>, 2> names{
      {{"normal", UsageMode::Normal}, {"reading", UsageMode::Reading}}};
  };

  // @rpc.enum(name="break_id")
  enum BreakId
  {
    // @rpc.enum.value(name="none")
    BREAK_ID_NONE = -1,
    // @rpc.enum.value(name="microbreak")
    BREAK_ID_MICRO_BREAK = 0,
    // @rpc.enum.value(name="restbreak")
    BREAK_ID_REST_BREAK = 1,
    // @rpc.enum.value(name="dailylimit")
    BREAK_ID_DAILY_LIMIT = 2,
  };

  template<>
  struct workrave::utils::enum_traits<BreakId>
  {
    static constexpr auto min = BREAK_ID_MICRO_BREAK;
    static constexpr auto max = BREAK_ID_DAILY_LIMIT;
    static constexpr auto linear = true;

    static constexpr std::array<std::pair<std::string_view, BreakId>, 4> names{{{"none", BREAK_ID_NONE},
                                                                                {"microbreak", BREAK_ID_MICRO_BREAK},
                                                                                {"restbreak", BREAK_ID_REST_BREAK},
                                                                                {"dailylimit", BREAK_ID_DAILY_LIMIT}}};
  };

  static constexpr auto BREAK_ID_SIZEOF = workrave::utils::enum_count<BreakId>();

  std::string operator%(const std::string &key, BreakId id);

  // @rpc.enum(name="break_hint")
  enum class BreakHint
  {
    // @rpc.enum.value(name="normal")
    Normal = 0,

    // Break was started on user request
    // @rpc.enum.value(name="userinitiated")
    UserInitiated = 1,

    // Natural break.
    // @rpc.enum.value(name="naturalbreak")
    NaturalBreak = 2
  };

  template<>
  struct workrave::utils::enum_traits<BreakHint>
  {
    static constexpr auto flag = true;
    static constexpr auto bits = 2;

    static constexpr std::array<std::pair<std::string_view, BreakHint>, 3> names{
      {{"normal", BreakHint::Normal}, {"userinitiated", BreakHint::UserInitiated}, {"naturalbreak", BreakHint::NaturalBreak}}};
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

  template<>
  struct workrave::utils::enum_traits<InsistPolicy>
  {
    static constexpr std::array<std::pair<std::string_view, InsistPolicy>, 4> names = {{{"invalid", InsistPolicy::Invalid},
                                                                                        {"halt", InsistPolicy::Halt},
                                                                                        {"reset", InsistPolicy::Reset},
                                                                                        {"ignore", InsistPolicy::Ignore}}};
  };

} // namespace workrave

#endif // WORKRAVE_BACKEND_CORETYPES_HH
