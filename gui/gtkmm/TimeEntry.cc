// TimeEntry.cc --- Entry widget for time
//
// Copyright (C) 2002, 2003 Raymond Penners <raymond@dotsphinx.com>
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
// $Id$

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"

#include "debug.hh"

#include <unistd.h>
#include "TimeEntry.hh"

TimeEntry::TimeEntry(bool millis)
  : Gtk::HBox(false, 1),
    hours_adjustment(0, 0, 23),
    mins_adjustment(0, 0, 59),
    secs_adjustment(0, 0, 59)
{
  TRACE_ENTER("TimeEntry::TimeEntry");
  this->millis = millis;
  
  Gtk::SpinButton *secs = manage(new Gtk::SpinButton(secs_adjustment));
  secs->set_numeric(true);
  secs_adjustment.signal_value_changed().connect(SigC::slot(*this, &TimeEntry::on_value_changed));

  if (millis)
    {
      secs->set_width_chars(6);
      secs->set_wrap(false);

      secs_adjustment.set_upper(60000);
      secs_adjustment.set_step_increment(100);
      secs_adjustment.set_page_increment(1000);
      pack_start(*secs, 0, 0);
    }
  else
    {
      secs->set_width_chars(2);
      secs->set_wrap(true);

      Gtk::SpinButton *hrs = manage(new Gtk::SpinButton(hours_adjustment));
      hrs->set_numeric(true);
      hrs->set_wrap(true);
      hrs->set_width_chars(2);
      hours_adjustment.signal_value_changed().connect(SigC::slot(*this, &TimeEntry::on_value_changed));
	
      Gtk::SpinButton *mins = manage(new Gtk::SpinButton(mins_adjustment));
      mins->set_numeric(true);
      mins->set_wrap(true);
      mins->set_width_chars(2);
      mins_adjustment.signal_value_changed().connect(SigC::slot(*this, &TimeEntry::on_value_changed));
	
	
      Gtk::Label *semi1 = manage(new Gtk::Label(":"));
      Gtk::Label *semi2 = manage(new Gtk::Label(":"));
	
      pack_start(*hrs, 0, 0);
      pack_start(*semi1, 0, 0);
      pack_start(*mins, 0, 0);
      pack_start(*semi2, 0, 0);
      pack_start(*secs, 0, 0);
    }
  
  TRACE_EXIT();
}


//! Destructor.
TimeEntry::~TimeEntry()
{
  TRACE_ENTER("TimeEntry::~TimeEntry");
  // FIXME: disconnect signals?
  TRACE_EXIT();
}


//! Set time
void
TimeEntry::set_value(time_t t)
{
  if (! millis)
    {
      hours_adjustment.set_value(t / (60*60));
      mins_adjustment.set_value((t / 60) % 60);
      secs_adjustment.set_value(t % 60);
    }
  else
    {
      secs_adjustment.set_value(t);
    }
}

//! Get time
time_t
TimeEntry::get_value()
{
  int s = (int) secs_adjustment.get_value();
  if (! millis)
    {
      int h = (int) hours_adjustment.get_value();
      int m = (int) mins_adjustment.get_value();
      return h * 60 * 60 + m * 60 + s;
    }
  else
    {
      return s;
    }
}

void
TimeEntry::on_value_changed()
{
  sig_value_changed.emit();
}
