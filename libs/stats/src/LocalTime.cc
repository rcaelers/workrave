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

#include "stats/LocalTime.hh"

#include "utils/TimeSource.hh"

namespace workrave::stats
{
  //! The current local wall clock time.
  LocalTime local_now()
  {
    // Goes through TimeSource, like the rest of the codebase, so that tests can
    // simulate the passage of time instead of mutating statistics directly.
    const std::time_t now = std::chrono::system_clock::to_time_t(workrave::utils::TimeSource::get_real_time());

    std::tm local{};
#if defined(_WIN32)
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif

    return to_local_time(local);
  }
} // namespace workrave::stats
