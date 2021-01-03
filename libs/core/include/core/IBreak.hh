// IBreak.hh -- Interface of a break.
//
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

#include "core/CoreTypes.hh"

#include <boost/signals2.hpp>
#include <string.h>

namespace workrave {

  enum class BreakEvent {
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

  inline std::ostream& operator<<(std::ostream& stream, workrave::BreakEvent event)
  {
    switch(event)
      {
      case workrave::BreakEvent::ShowPrelude:
        stream << "ShowPrelude";
        break;

      case workrave::BreakEvent::ShowBreak:
        stream << "ShowBreak";
        break;

      case workrave::BreakEvent::ShowBreakForced:
        stream << "ShowBreakForced";
        break;

      case workrave::BreakEvent::BreakStart:
        stream << "BreakStart";
        break;

      case workrave::BreakEvent::BreakIdle:
        stream << "BreakIdle";
        break;

      case workrave::BreakEvent::BreakStop:
        stream << "BreakStop";
        break;

      case workrave::BreakEvent::BreakIgnored:
        stream << "BreakIgnored";
        break;

      case workrave::BreakEvent::BreakPostponed:
        stream << "BreakPostponed";
        break;

      case workrave::BreakEvent::BreakSkipped:
        stream << "BreakSkipped";
        break;

      case workrave::BreakEvent::BreakTaken:
        stream << "BreakTaken";
        break;
      }
    return stream;
  }

  //! Interface to retrieve information about a break.
  class IBreak
  {
  public:
    using Ptr = std::shared_ptr<IBreak>;

    virtual ~IBreak() = default;

    virtual boost::signals2::signal<void(workrave::BreakEvent)> &signal_break_event() = 0;

    //! Returns the name of the break.
    virtual std::string get_name() const = 0;

    //! Is this break currently enabled?
    virtual bool is_enabled() const = 0;

    //! Is the timer currently running?.
    virtual bool is_running() const = 0;

    //! Is the user taking the break.
    virtual bool is_taking() const = 0;

    //! Has the maximum number of prelude been reached
    virtual bool is_max_preludes_reached() const = 0;

    //! Is the break currently active.
    virtual bool is_active() const = 0;

    //! Returns the elasped active time.
    virtual int64_t get_elapsed_time() const = 0;

    //! Returns the elasped idle time.
    virtual int64_t get_elapsed_idle_time() const = 0;

    //! Returns the auto-reset interval (i.e. break duration)
    virtual int64_t get_auto_reset() const = 0;

    //! Is the auto-reset enabled?
    virtual bool is_auto_reset_enabled() const = 0;

    //! Returns the break limit (i.e. time before break)
    virtual int64_t get_limit() const = 0;

    //! Is the limit enabled.
    virtual bool is_limit_enabled() const = 0;

    //! Returns the total overdue time since the last daily limit reset.
    virtual int64_t get_total_overdue_time() const = 0;

    //! Request to postpone the break.
    virtual void postpone_break() = 0;

    //! Request to skip the break.
    virtual void skip_break() = 0;
  };
}

#endif // WORKRAVE_BACKEND_IBREAK_HH
