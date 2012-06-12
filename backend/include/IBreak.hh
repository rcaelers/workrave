// IBreak.hh -- Interface of a break.
//
// Copyright (C) 2001 - 2007, 2011, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef IBREAK_HH
#define IBREAK_HH

#include "CoreTypes.hh"

#include <boost/signals2.hpp>
#include <string.h>

namespace workrave {

  //! Interface to retrieve information about a break.
  class IBreak
  {
  public:
    typedef boost::shared_ptr<IBreak> Ptr;

    virtual ~IBreak() {}

    enum BreakEvent {
      BREAK_EVENT_PRELUDE_STARTED,
      BREAK_EVENT_BREAK_IGNORED,
      BREAK_EVENT_BREAK_STARTED,
      BREAK_EVENT_BREAK_STARTED_FORCED,
      BREAK_EVENT_BREAK_ENDED,
      BREAK_EVENT_BREAK_IDLE,
    };
    
    virtual boost::signals2::signal<void(BreakEvent)> &signal_break_event() = 0;

    //! Returns the name of the break.
    virtual std::string get_name() const = 0;

    //! Is this break currently enabled?
    virtual bool is_enabled() const = 0;

    //! Is the timer currently runninf?.
    virtual bool is_running() const = 0;

    //! Is the user taking the break.
    virtual bool is_taking() const = 0;

    //! Is the break currently active.
    virtual bool is_active() const = 0;

    //! Returns the elasped active time.
    virtual time_t get_elapsed_time() const = 0;

    //! Returns the elasped idle time.
    virtual time_t get_elapsed_idle_time() const = 0;

    //! Returns the auto-reset interval (i.e. break duration)
    virtual time_t get_auto_reset() const = 0;

    //! Is the auto-reset enabled?
    virtual bool is_auto_reset_enabled() const = 0;

    //! Returns the break limit (i.e. time before break)
    virtual time_t get_limit() const = 0;

    //! Is the limit enabled.
    virtual bool is_limit_enabled() const = 0;

    //! Returns the total overdue time since the last daily limit reset.
    virtual time_t get_total_overdue_time() const = 0;
    
    //! Request to postpone the break.
    virtual void postpone_break() = 0;

    //! Request to skip the break.
    virtual void skip_break() = 0;
  };
}

#endif // IBREAK_HH
