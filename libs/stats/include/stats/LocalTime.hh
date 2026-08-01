// Copyright (C) 2026 Rob Caelers
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

#ifndef WORKRAVE_LIBS_STATS_LOCALTIME_HH
#define WORKRAVE_LIBS_STATS_LOCALTIME_HH

#include <chrono>
#include <ctime>

namespace workrave::stats
{
  //! Wall clock time as the user reads it off a clock, to the second.
  using LocalTime = std::chrono::local_seconds;

  //! The calendar date of a local time.
  [[nodiscard]] inline std::chrono::year_month_day
  date_of(LocalTime time)
  {
    return std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(time)};
  }

  //! The time of day of a local time, counted from midnight.
  [[nodiscard]] inline std::chrono::seconds
  time_of_day(LocalTime time)
  {
    return time - std::chrono::floor<std::chrono::days>(time);
  }

  //! Combines a date and a time of day into a local time.
  [[nodiscard]] inline LocalTime
  to_local_time(std::chrono::year_month_day date, std::chrono::seconds time_of_day)
  {
    return std::chrono::local_days{date} + time_of_day;
  }

  //! Converts a std::tm, as the C library reports it, to a local time.
  [[nodiscard]] inline LocalTime
  to_local_time(const std::tm &time)
  {
    const std::chrono::year_month_day date{std::chrono::year{time.tm_year + 1900} / (time.tm_mon + 1) / time.tm_mday};

    return to_local_time(date,
                         std::chrono::hours{time.tm_hour} + std::chrono::minutes{time.tm_min}
                           + std::chrono::seconds{time.tm_sec});
  }

  //! Converts a local time back to a std::tm, for locale aware formatting. // TODO/STATS: avoid std::tm
  [[nodiscard]] inline std::tm
  to_tm(LocalTime time)
  {
    const std::chrono::year_month_day date = date_of(time);
    const std::chrono::hh_mm_ss clock{time_of_day(time)};

    std::tm result{};
    result.tm_year = static_cast<int>(date.year()) - 1900;
    result.tm_mon = static_cast<int>(static_cast<unsigned>(date.month())) - 1;
    result.tm_mday = static_cast<int>(static_cast<unsigned>(date.day()));
    result.tm_hour = static_cast<int>(clock.hours().count());
    result.tm_min = static_cast<int>(clock.minutes().count());
    result.tm_sec = static_cast<int>(clock.seconds().count());

    // Some locales spell out the weekday, so these cannot be left at zero.
    result.tm_wday = static_cast<int>(std::chrono::weekday{std::chrono::sys_days{date}}.c_encoding());
    result.tm_yday = static_cast<int>(
      (std::chrono::sys_days{date} - std::chrono::sys_days{date.year() / std::chrono::January / 1}).count());
    result.tm_isdst = -1;

    return result;
  }

  //! The current local wall clock time.
  [[nodiscard]] LocalTime local_now();
} // namespace workrave::stats

#endif // WORKRAVE_LIBS_STATS_LOCALTIME_HH
