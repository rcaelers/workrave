// MicroBreakWindow.cc --- window for the microbreak
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Rob Caelers & Raymond Penners
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

#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/image.h>

#include "nls.h"
#include "debug.hh"

#include "CoreFactory.hh"
#include "MicroBreakWindow.hh"
#include "WindowHints.hh"
#include "TimeBar.hh"
#include "ITimer.hh"
#include "IBreakResponse.hh"
#include "Util.hh"
#include "GtkUtil.hh"
#include "Text.hh"
#include "Hig.hh"
#include "Frame.hh"

//! Construct a new Microbreak window.
MicroBreakWindow::MicroBreakWindow(HeadInfo &head, bool ignorable, GUI::BlockMode mode) :
  BreakWindow(BREAK_ID_MICRO_BREAK, head, ignorable, mode),
  progress_value(0),
  progress_max_value(0),
  is_flashing(false)
{
  set_title(_("Micro-break"));
}

Gtk::Widget *
MicroBreakWindow::create_gui()
{
  // Time bar
  time_bar = manage(new TimeBar);
  time_bar->set_text("Microbreak 0:32"); // FIXME

  // Label
  label = manage(new Gtk::Label());

  // Icon
  string icon = Util::complete_directory("micro-break.png", Util::SEARCH_PATH_IMAGES);
  Gtk::Image *img = manage(new Gtk::Image(icon));
  img->set_alignment(0.0, 0.0);

  // HBox
  Gtk::HBox *hbox = manage(new Gtk::HBox(false, 12));
  hbox->pack_start(*img, false, false, 0);
  hbox->pack_start(*label, Gtk::PACK_EXPAND_PADDING, 0);

  // Overall vbox
  Gtk::VBox *box = new Gtk::VBox(false, 12);
  box->pack_start(*hbox, Gtk::PACK_EXPAND_WIDGET, 0);
  box->pack_start(*time_bar, Gtk::PACK_EXPAND_WIDGET, 0);

  // Button box at the bottom.
  ICore *core = CoreFactory::get_core();
  ITimer *restbreak_timer =  core->get_timer(BREAK_ID_REST_BREAK);
  bool has_rb = restbreak_timer->get_state() != ITimer::STATE_INVALID;
  if (ignorable_break || has_rb)
    {
      Gtk::HBox *button_box;
      if (ignorable_break)
        {
          button_box = manage(new Gtk::HBox(false, 6));

          Gtk::HBox *bbox = manage(new Gtk::HBox(true, 6));
          Gtk::Button *postpone_button = create_postpone_button();
          bbox->pack_end(*postpone_button, Gtk::PACK_EXPAND_WIDGET, 0);
          Gtk::Button *skip_button = create_skip_button();
          bbox->pack_end(*skip_button, Gtk::PACK_EXPAND_WIDGET, 0);

          Gtk::Alignment *bboxa =
            manage(new Gtk::Alignment(1.0, 0.0, 0.0, 0.0));
          bboxa->add(*bbox);

          if (has_rb)
            {
              button_box->pack_start
                (*manage(create_restbreaknow_button(false)),
                 Gtk::PACK_SHRINK, 0);
            }
          button_box->pack_end(*bboxa,
                                 Gtk::PACK_EXPAND_WIDGET, 0);
        }
      else
        {
          button_box = manage(new Gtk::HBox(false, 6));
          button_box->pack_end(*manage(create_restbreaknow_button(true)),
                               Gtk::PACK_SHRINK, 0);
        }
      box->pack_start(*button_box, Gtk::PACK_EXPAND_WIDGET, 0);
    }

  return box;
}


//! Destructor.
MicroBreakWindow::~MicroBreakWindow()
{
  TRACE_ENTER("MicroBreakWindow::~MicroBreakWindow");
  TRACE_EXIT();
}



//! Updates the main window.
void
MicroBreakWindow::heartbeat()
{
  refresh();
}


Gtk::Button *
MicroBreakWindow::create_restbreaknow_button(bool label)
{
  Gtk::Button *ret;
  ret = manage(GtkUtil::create_image_button(_("Rest break"),
                                            "timer-rest-break.png",
                                            label));
  ret->signal_clicked()
    .connect(sigc::mem_fun(*this,
                         &MicroBreakWindow::on_restbreaknow_button_clicked));
  GTK_WIDGET_UNSET_FLAGS(ret->gobj(), GTK_CAN_FOCUS);
  return ret;
}

//! The restbreak button was clicked.
void
MicroBreakWindow::on_restbreaknow_button_clicked()
{
  GUI *gui = GUI::get_instance();
  assert(gui != NULL);

  gui->restbreak_now();
}


void
MicroBreakWindow::refresh_time_bar()
{
  TRACE_ENTER("MicroBreakWindow::refresh_time_bar");

  time_t time = progress_max_value - progress_value;
  string s = _("Micro-break");
  s += ' ';
  s += Text::time_to_string(time);

  time_bar->set_progress(progress_value, progress_max_value - 1);
  time_bar->set_text(s);

  ICore *core = CoreFactory::get_core();
  bool user_active = core->is_user_active();
  if (frame != NULL)
    {
      if (user_active && !is_flashing)
        {
          frame->set_frame_color(Gdk::Color("orange"));
          frame->set_frame_visible(true);
          frame->set_frame_flashing(500);
          is_flashing = true;
        }
      else if (!user_active && is_flashing)
        {
          frame->set_frame_flashing(0);
          frame->set_frame_visible(false);
          is_flashing = false;
        }
    }
  time_bar->update();
  TRACE_MSG(progress_value << " " << progress_max_value);
  TRACE_EXIT();
}


void
MicroBreakWindow::refresh_label()
{
  TRACE_ENTER("MicroBreakWindow::refresh_label");

  ICore *core = CoreFactory::get_core();

  ITimer *restbreak_timer =  core->get_timer(BREAK_ID_REST_BREAK);
  ITimer *daily_timer =  core->get_timer(BREAK_ID_DAILY_LIMIT);

  BreakId show_next = BREAK_ID_NONE;

  time_t rb = restbreak_timer->get_limit() - restbreak_timer->get_elapsed_time();
  time_t dl = daily_timer->get_limit() - daily_timer->get_elapsed_time();

  if (restbreak_timer->get_state() != ITimer::STATE_INVALID)
    {
      show_next = BREAK_ID_REST_BREAK;
    }

  if (daily_timer->get_state() != ITimer::STATE_INVALID)
    {
      if (show_next == BREAK_ID_NONE || dl < rb)
        {
          show_next = BREAK_ID_DAILY_LIMIT;
        }
    }


  Glib::ustring txt(_("Please relax for a few seconds"));
  if (show_next == BREAK_ID_REST_BREAK)
    {
      char s[128];

      if (rb >= 0)
        {
          sprintf(s, _("Next rest break in %s"),
                  Text::time_to_string(rb, true).c_str());
        }
      else
        {
          sprintf(s, _("Rest break %s overdue"),
                  Text::time_to_string(-rb, true).c_str());
        }

      txt += "\n";
      txt += s;
    }
  else if (show_next == BREAK_ID_DAILY_LIMIT)
    {
      char s[128];

      if (dl >= 0)
        {
          sprintf(s, _("Daily limit in %s"),
                  Text::time_to_string(dl, true).c_str());
        }
      else
        {
          sprintf(s, _("Daily limit %s overdue"),
                  Text::time_to_string(-dl, true).c_str());
        }

      txt += "\n";
      txt += s;
    }

  label->set_markup(HigUtil::create_alert_text(_("Micro-break"), txt.c_str()));
  TRACE_EXIT();
}


//! Refresh window.
void
MicroBreakWindow::refresh()
{
  BreakWindow::refresh();

  refresh_time_bar();
  refresh_label();
}


void
MicroBreakWindow::set_progress(int value, int max_value)
{
  progress_max_value = max_value;
  progress_value = value;
}
