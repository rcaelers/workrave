// ITimer.hh --- The Break Timer Query Interface
//
// Copyright (C) 2001, 2002, 2003, 2005, 2006, 2007 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2007-08-30 16:03:12 robc>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef ITIMER_HH
#define ITIMER_HH

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif


//! The Timer interface.
class ITimer
{
public:
  enum TimerState
    {
      STATE_INVALID,
      STATE_RUNNING,
      STATE_STOPPED
    };

public:
  virtual ~ITimer() {}

  //! Returns the current time state.
  virtual TimerState get_state() const = 0;

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
};

#endif // ITIMER_HH
