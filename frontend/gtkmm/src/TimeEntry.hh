// TimeEntry.hh --- Entry widget for time
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
//

#ifndef TIMEENTRY_HH
#define TIMEENTRY_HH

#include "preinclude.h"

#include <gtkmm/box.h>
#include <gtkmm/adjustment.h>

namespace Gtk
{
  class SpinButton;
}

class TimeEntry : public Gtk::HBox
{
public:  
  TimeEntry(bool millis=false);
  ~TimeEntry();

  time_t get_value();
  void set_value(time_t time);

  typedef SigC::Signal0<void> signal_value_changed_t;
  signal_value_changed_t &signal_value_changed();

protected:
  virtual void on_changed();
  virtual void on_value_changed();

  signal_value_changed_t sig_value_changed;

private:
  void update(Gtk::SpinButton *spin);
  
private:
  Gtk::SpinButton *hrs;
  Gtk::SpinButton *mins;
  Gtk::SpinButton *secs;

  Gtk::Adjustment hours_adjustment;
  Gtk::Adjustment mins_adjustment;
  Gtk::Adjustment secs_adjustment;

  bool millis;
};


inline TimeEntry::signal_value_changed_t&
TimeEntry::signal_value_changed()
{
  return sig_value_changed;
}

#endif // TIMEENTRY_HH
