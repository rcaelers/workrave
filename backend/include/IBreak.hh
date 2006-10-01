// IBreak.hh
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Rob Caelers <robc@krandor.org>
// All rights reserved.
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
// $Id$%
//

#ifndef IBREAK_HH
#define IBREAK_HH

#include "ICore.hh"

#include <string.h>
using namespace std;

class IBreak
{
public:
  virtual ~IBreak() {}
  
  virtual ITimer *get_timer() const = 0;
  virtual bool is_enabled() const = 0;
  virtual string get_name() const = 0;
  
  virtual int get_timer_limit() const = 0;
  virtual void set_timer_limit(int n) = 0;
  virtual int get_timer_auto_reset() const = 0;
  virtual void set_timer_auto_reset(int n) = 0;
  virtual string get_timer_reset_pred() const = 0;
  virtual void set_timer_reset_pred(string n) = 0;
  virtual int get_timer_snooze() const = 0;
  virtual void set_timer_snooze(int n) = 0;
  virtual string get_timer_monitor() const = 0;
  virtual void set_timer_monitor(string n) = 0;
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

#endif // TIMERDATA_HH
