// BreakInterface.hh --- Generic Interface for breaks
//
// Copyright (C) 2002, 2003 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2003-01-05 00:49:25 robc>
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

#ifndef BREAKINTERFACE_HH
#define BREAKINTERFACE_HH

//! Interface to a Break.
class BreakInterface
{
public:
  struct BreakStateData
  {
    bool forced_break;
    int prelude_count;

    int break_stage;
    bool final_prelude;
    int prelude_time;
  };
  
  enum BreakState { BREAK_ACTIVE, BREAK_INACTIVE, BREAK_SUSPENDED };

  BreakInterface() {}
  virtual ~BreakInterface() {}
  
  //! Starts the break.
  virtual void start_break() = 0;

  //! Forces the start of the break.
  virtual void force_start_break() = 0;

  //! Stops the break.
  virtual void stop_break() = 0;

  //! Is the break active ?
  virtual BreakState get_break_state() = 0;

  //! Returns \c true if the break requires a heartbeat.
  virtual bool need_heartbeat() = 0;
  
  //! Period update. 
  virtual void heartbeat() = 0;

  virtual void set_state_data(bool activate, const BreakStateData &data) = 0;
  virtual void get_state_data(BreakStateData &data) = 0;
};

#endif // BREAKINTERFACE_HH
