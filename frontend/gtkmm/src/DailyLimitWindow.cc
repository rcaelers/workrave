// DailyLimitWindow.cc --- window for the daily limit
//
// Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"
#include "nls.h"
#include "debug.hh"

#include "DailyLimitWindow.hh"

#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>

#include "IBreakResponse.hh"
#include "WindowHints.hh"
#include "Util.hh"
#include "Hig.hh"

//! Construct a new Daily limit window.
DailyLimitWindow::DailyLimitWindow(HeadInfo &head, bool ignorable,
                                   GUI::BlockMode mode) :
  BreakWindow(BREAK_ID_DAILY_LIMIT, head, ignorable, mode)
{
  set_title(_("Daily limit"));
}

Gtk::Widget *
DailyLimitWindow::create_gui()
{
  // label
  Glib::ustring txt = HigUtil::create_alert_text
    (_("Daily limit"),
     _("You have reached your daily limit. Please stop working\n"
       "behind the computer. If your working day is not over yet,\n"
       "find something else to do, such as reviewing a document."));

  Gtk::Label *label = manage(new Gtk::Label());
  label->set_markup(txt);

  // Icon
  string icon = Util::complete_directory("daily-limit.png", Util::SEARCH_PATH_IMAGES);
  Gtk::Image *img = manage(new Gtk::Image(icon));
  img->set_alignment(0.0, 0.0);

  // HBox
  Gtk::HBox *hbox = manage(new Gtk::HBox(false, 12));
  hbox->pack_start(*img, false, false, 0);
  hbox->pack_start(*label, Gtk::EXPAND | Gtk::FILL, 0);

  // Overall vbox
  Gtk::VBox *box = new Gtk::VBox(false, 12);
  box->pack_start(*hbox, Gtk::EXPAND | Gtk::FILL, 0);

  // Button box at the bottom.
  Gtk::HButtonBox *button_box = create_break_buttons(true, true);
  if (button_box)
    {
      box->pack_start(*manage(button_box), Gtk::EXPAND | Gtk::FILL, 0);
    }
  return box;
}


//! Destructor.
DailyLimitWindow::~DailyLimitWindow()
{
}

void
DailyLimitWindow::set_progress(int value, int max_value)
{
  (void) value;
  (void) max_value;
  // FIXME: show progress until timer restart time?
}


