// MicroPauseWindow.cc --- window for the micropause
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id$";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/image.h>

#include "nls.h"
#include "debug.hh"

#include "MicroPauseWindow.hh"
#include "WindowHints.hh"
#include "TimeBar.hh"
#include "TimerInterface.hh"
#include "BreakResponseInterface.hh"
#include "Util.hh"
#include "Text.hh"
#include "Hig.hh"

//! Construct a new Micropause window.
MicroPauseWindow::MicroPauseWindow(HeadInfo &head, TimerInterface *timer, bool ignorable, bool insist) :
  BreakWindow(BREAK_ID_MICRO_PAUSE, head, ignorable, insist),
  restbreak_timer(timer),
  progress_value(0),
  progress_max_value(0)
{
  set_title(_("Micro-pause"));
}

Gtk::Widget *
MicroPauseWindow::create_gui()
{
  // Time bar
  time_bar = manage(new TimeBar);
  time_bar->set_text("Micropause 0:32"); // FIXME

  // Label
  label = manage(new Gtk::Label());

  // Icon
  string icon = Util::complete_directory("micropause.png", Util::SEARCH_PATH_IMAGES);
  Gtk::Image *img = manage(new Gtk::Image(icon));
  img->set_alignment(0.0, 0.0);
  
  // HBox
  Gtk::HBox *hbox = manage(new Gtk::HBox(false, 12));
  hbox->pack_start(*img, false, false, 0);
  hbox->pack_start(*label, Gtk::EXPAND | Gtk::FILL, 0);

  // Overall vbox
  Gtk::VBox *box = new Gtk::VBox(false, 12);
  box->pack_start(*hbox, Gtk::EXPAND | Gtk::FILL, 0);
  box->pack_start(*time_bar, Gtk::EXPAND | Gtk::FILL, 0);

  // Button box at the bottom.
  Gtk::HButtonBox *button_box = create_break_buttons(false, false);
  if (button_box != NULL)
    {
      box->pack_start(*manage(button_box), Gtk::EXPAND | Gtk::FILL, 0);
    }

  return box;
}


//! Destructor.
MicroPauseWindow::~MicroPauseWindow()
{
  TRACE_ENTER("MicroPauseWindow::~MicroPauseWindow");
  TRACE_EXIT();
}



//! Updates the main window.
void
MicroPauseWindow::heartbeat()
{
  refresh();
}


void
MicroPauseWindow::refresh_time_bar()
{
  TRACE_ENTER("MicroPauseWindow::refresh_time_bar");

  time_t time = progress_max_value - progress_value;
  string s = _("Micro-pause");
  s += ' ';
  s += Text::time_to_string(time);
  time_bar->set_progress(progress_value, progress_max_value - 1);
  time_bar->set_text(s);
  time_bar->update();
  TRACE_MSG(progress_value << " " << progress_max_value);
  TRACE_EXIT();
}


void
MicroPauseWindow::refresh_label()
{
  TRACE_ENTER("MicroPauseWindow::refresh_label");
  time_t limit = restbreak_timer->get_limit();
  time_t elapsed =  restbreak_timer->get_elapsed_time();
  char s[128];

  if (limit > elapsed)
    {
      time_t rb = limit - elapsed;
      sprintf(s, _("Next rest break in %s"),
              Text::time_to_string(rb, true).c_str());
    }
  else
    {
      time_t rb = elapsed - limit;
      sprintf(s, _("Rest break %s overdue"),
              Text::time_to_string(rb, true).c_str());
    }

  Glib::ustring txt(_("Please relax for a few seconds"));
  txt += "\n";
  txt += s;
  
  label->set_markup(HigUtil::create_alert_text(_("Micro-pause"), txt.c_str()));
  TRACE_EXIT();
}


//! Refresh window.
void
MicroPauseWindow::refresh()
{
  refresh_time_bar();
  refresh_label();
}


void
MicroPauseWindow::set_progress(int value, int max_value)
{
  progress_max_value = max_value;
  progress_value = value;
}


