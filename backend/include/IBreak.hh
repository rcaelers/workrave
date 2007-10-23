// IBreak.hh -- Interface of a break.
//
// Copyright (C) 2001 - 2007 Rob Caelers <robc@krandor.nl>
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
// $Id$%
//

#ifndef IBREAK_HH
#define IBREAK_HH

#include "ICore.hh"

#include <string.h>

namespace workrave {

  //! Interface to retrieve information about a break.
  class IBreak
  {
  public:
    virtual ~IBreak() {}

    //! Returns the name of the break.
    virtual std::string get_name() const = 0;

    //! Returns the ID of the break.
    virtual BreakId get_id() const = 0;

    //! Is this break currently enabled?
    virtual bool is_enabled() const = 0;

    //! Returns the current time state.
    virtual bool is_running() const = 0;

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
    
    // These functions will be removed in the next release

    virtual int get_timer_limit() const = 0;
    virtual void set_timer_limit(int n) = 0;
    virtual int get_timer_auto_reset() const = 0;
    virtual void set_timer_auto_reset(int n) = 0;
    virtual std::string get_timer_reset_pred() const = 0;
    virtual void set_timer_reset_pred(std::string n) = 0;
    virtual int get_timer_snooze() const = 0;
    virtual void set_timer_snooze(int n) = 0;
    virtual std::string get_timer_monitor() const = 0;
    virtual void set_timer_monitor(std::string n) = 0;
    virtual bool get_timer_activity_sensitive() const = 0;
    virtual void set_timer_activity_sensitive(bool b) = 0;
    virtual int get_break_max_preludes() const = 0;
    virtual void set_break_max_preludes(int n) = 0;
    virtual int get_break_max_postpone() const = 0;
    virtual void set_break_max_postpone(int n) = 0;
    virtual bool get_break_ignorable() const = 0;
    virtual void set_break_ignorable(bool b) = 0;
    virtual int get_break_exercises() const = 0;
    virtual void set_break_exercises(int n) = 0;
    virtual bool get_break_enabled() const = 0;
    virtual void set_break_enabled(bool b) = 0;
  };
}

#endif // IBREAK_HH
