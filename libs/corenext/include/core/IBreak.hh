// Copyright (C) 2001 - 2007, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_BACKEND_IBREAK_HH
#define WORKRAVE_BACKEND_IBREAK_HH

#include <cstring>

#include <boost/signals2.hpp>

#include "core/CoreTypes.hh"
#include "utils/Enum.hh"

namespace workrave
{
  enum class BreakEvent
  {
    ShowPrelude,
    ShowBreak,
    ShowBreakForced,
    BreakStart,
    BreakIdle,
    BreakStop,
    BreakIgnored,
    BreakPostponed,
    BreakSkipped,
    BreakTaken,
  };

  template<>
  struct workrave::utils::enum_traits<BreakEvent>
  {
    static constexpr std::array<std::pair<std::string_view, BreakEvent>, 10> names{
      {{"ShowPrelude", BreakEvent::ShowPrelude},
       {"ShowBreak", BreakEvent::ShowBreak},
       {"ShowBreakForced", BreakEvent::ShowBreakForced},
       {"BreakStart", BreakEvent::BreakStart},
       {"BreakIdle", BreakEvent::BreakIdle},
       {"BreakStop", BreakEvent::BreakStop},
       {"BreakIgnored", BreakEvent::BreakIgnored},
       {"BreakPostponed", BreakEvent::BreakPostponed},
       {"BreakSkipped", BreakEvent::BreakSkipped},
       {"BreakTaken", BreakEvent::BreakTaken}}};
  };

  //! Interface to retrieve information about a break.
  class IBreak
  {
  public:
    using Ptr = std::shared_ptr<IBreak>;

    virtual ~IBreak() = default;

    virtual boost::signals2::signal<void(workrave::BreakEvent)> &signal_break_event() = 0;

    //! Returns the name of the break.
    [[nodiscard]] virtual std::string get_name() const = 0;

    //! Is this break currently enabled?
    [[nodiscard]] virtual bool is_enabled() const = 0;

    //! Is the timer currently running?.
    [[nodiscard]] virtual bool is_running() const = 0;

    //! Is the user taking the break.
    [[nodiscard]] virtual bool is_taking() const = 0;

    //! Has the maximum number of prelude been reached
    [[nodiscard]] virtual bool is_max_preludes_reached() const = 0;

    //! Is the break currently active.
    [[nodiscard]] virtual bool is_active() const = 0;

    //! Returns the elasped active time.
    [[nodiscard]] virtual int64_t get_elapsed_time() const = 0;

    //! Returns the elasped idle time.
    [[nodiscard]] virtual int64_t get_elapsed_idle_time() const = 0;

    //! Returns the auto-reset interval (i.e. break duration)
    [[nodiscard]] virtual int64_t get_auto_reset() const = 0;

    //! Is the auto-reset enabled?
    [[nodiscard]] virtual bool is_auto_reset_enabled() const = 0;

    //! Returns the break limit (i.e. time before break)
    [[nodiscard]] virtual int64_t get_limit() const = 0;

    //! Is the limit enabled.
    [[nodiscard]] virtual bool is_limit_enabled() const = 0;

    //! Returns the total overdue time since the last daily limit reset.
    [[nodiscard]] virtual int64_t get_total_overdue_time() const = 0;

    //! Request to postpone the break.
    virtual void postpone_break() = 0;

    //! Request to skip the break.
    virtual void skip_break() = 0;
  };
} // namespace workrave

#endif // WORKRAVE_BACKEND_IBREAK_HH
