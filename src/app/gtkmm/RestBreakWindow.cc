// RestBreakWindow.cc --- window for the micropause
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
// All rights reserved.
//
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

const int TIMEOUT = 1000;

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>

#include <gtkmm/button.h>
#include <gtkmm/image.h>

#include "debug.hh"
#include "nls.h"

#include "Hig.hh"
#include "RestBreakWindow.hh"
#include "Text.hh"
#include "TimeBar.hh"
#include "Util.hh"
#include "WindowHints.hh"

#include "TimerInterface.hh"
#include "BreakInterface.hh"
#include "BreakResponseInterface.hh"
#include "GtkUtil.hh"

#ifdef HAVE_EXERCISES
#include "CoreInterface.hh"
#include "CoreFactory.hh"
#include "Exercise.hh"
#include "ExercisesPanel.hh"
#endif

const int MARGINX = 8;
const int MARGINY = 8;


//! Constructor
/*!
 *  \param control The controller.
 */
RestBreakWindow::RestBreakWindow(HeadInfo &head, bool ignorable, bool insist) :
  window_width(0),
  window_height(0),
  timebar(NULL),
  progress_value(0),
  progress_max_value(0),
  insist_break(insist),
  head_info(head)
{
  TRACE_ENTER("RestBreakWindow::RestBreakWindow");

  // Initialize this window
  set_title(_("_Rest break"));
  
  // Add other widgets.
  Gtk::VBox *vbox = manage(new Gtk::VBox(false, 6));
  vbox->pack_start(
#ifdef HAVE_EXERCISES
                   pluggable_panel
#else
                   *create_info_panel()
#endif
                   , false, false, 0);

  // Timebar
  timebar = manage(new TimeBar);
  vbox->pack_start(*timebar, false, false, 6);
  
  // Button box at the bottom.
  if (ignorable)
    {
      button_box = manage(new Gtk::HButtonBox(Gtk::BUTTONBOX_END, 6));
      
      Gtk::Button *lockButton = create_lock_button();
      if (lockButton != NULL)
        button_box->pack_end(*manage(lockButton), Gtk::SHRINK, 0);

      Gtk::Button *skipButton = manage(create_skip_button());
      button_box->pack_end(*skipButton, Gtk::SHRINK, 0);

      Gtk::Button *postponeButton = manage(create_postpone_button());
      button_box->pack_end(*postponeButton, Gtk::SHRINK, 0);
      
      vbox->pack_end(*button_box, Gtk::SHRINK, 6);
  
      postponeButton->signal_clicked().connect(SigC::slot(*this, &RestBreakWindow::on_postpone_button_clicked));
      skipButton->signal_clicked().connect(SigC::slot(*this, &RestBreakWindow::on_skip_button_clicked));
    }
  
  add(*vbox);
  
  show_all_children();
  stick();
  
  // Set window hints.
  WindowHints::set_skip_winlist(Gtk::Widget::gobj(), true);
  WindowHints::set_always_on_top(Gtk::Widget::gobj(), true);
  
  add_events(Gdk::EXPOSURE_MASK);
  add_events(Gdk::FOCUS_CHANGE_MASK);

  set_screen(head);
  
  TRACE_EXIT();
}


//! Destructor.
RestBreakWindow::~RestBreakWindow()
{
  TRACE_ENTER("RestBreakWindow::~RestBreakWindow");
  ungrab();
  TRACE_EXIT();
}


//! Starts the restbreak.
void
RestBreakWindow::start()
{
  TRACE_ENTER("RestBreakWindow::start");
  refresh();
  set_ignore_activity(false);
  
#ifdef HAVE_EXERCISES
  if (get_exercise_count() > 0)
    {
      install_exercises_panel();
    }
  else
    {
      install_info_panel();
    }
#endif
  center();
  show_all();

  if (insist_break)
    {
      grab();
    }

#ifdef CAUSES_FVWM_FOCUS_PROBLEMS
  present(); // After grab() please (Windows)
#endif
  TRACE_EXIT();
}


//! Stops the restbreak.
void
RestBreakWindow::stop()
{
  TRACE_ENTER("RestBreakWindow::stop");
  ungrab();
  hide_all();
  TRACE_EXIT();
}


//! Self-Destruct
/*!
 *  This method MUST be used to destroy the objects through the
 *  BreakWindowInterface. it is NOT possible to do a delete on
 *  this interface...
 */
void
RestBreakWindow::destroy()
{
  delete this;
}

//! The postpone button was clicked.
void
RestBreakWindow::on_postpone_button_clicked()
{
  if (break_response != NULL)
    {
      break_response->postpone_break(BREAK_ID_REST_BREAK);
    }
}



//! The skip button was clicked.
void
RestBreakWindow::on_skip_button_clicked()
{
  if (break_response != NULL)
    {
      break_response->skip_break(BREAK_ID_REST_BREAK);
    }
}



//! RestBreak window is exposed.
bool
RestBreakWindow::on_expose_event(GdkEventExpose* e)
{
  // Send event to parent.
  Gtk::Window::on_expose_event(e);

  draw_time_bar();
  return true;
}


//! Period timer callback.
void
RestBreakWindow::refresh()
{
  draw_time_bar();
}


void
RestBreakWindow::set_progress(int value, int max_value)
{
  progress_max_value = max_value;
  progress_value = value;
}


//! Draws the timer bar.
void
RestBreakWindow::draw_time_bar()
{
  timebar->set_text_color(Gdk::Color("black"));
  timebar->set_progress(progress_value, progress_max_value);

  time_t time = progress_max_value - progress_value;
  char s[128];
  sprintf(s, _("Rest break for %s"), Text::time_to_string(time, true).c_str());
  
  timebar->set_text(s);
  timebar->update();
}


Gtk::Widget *
RestBreakWindow::create_info_panel()
{
  Gtk::HBox *info_box = manage(new Gtk::HBox(false, 12));
  
  string icon = Util::complete_directory("restbreak.png", Util::SEARCH_PATH_IMAGES);
  Gtk::Image *info_img = manage(new Gtk::Image(icon));
  info_img->set_alignment(0.0, 0.0);
  Gtk::Label *info_lab =
    manage(new Gtk::Label());
  Glib::ustring txt = HigUtil::create_alert_text
    (_("Rest break"),
     _("This is your rest break. Make sure you stand up and\n"
       "walk away from your computer on a regular basis. Just\n"
       "walk around for a few minutes, stretch, and relax."));
  info_lab->set_markup(txt);
  info_box->pack_start(*info_img, false, false, 0);
  info_box->pack_start(*info_lab, false, true, 0);
  return info_box;
}

#ifdef HAVE_EXERCISES
void
RestBreakWindow::clear_pluggable_panel()
{
  Glib::ListHandle<Gtk::Widget *> children = pluggable_panel.get_children();
  if (children.size() > 0)
    {
      pluggable_panel.remove(*(*(children.begin())));
    }
}

int
RestBreakWindow::get_exercise_count()
{
  int ret = 0;
  
  if (Exercise::has_exercises())
    {
      CoreInterface *core = CoreFactory::get_core();
      assert(core != NULL);
      
      ret = core->get_break(BREAK_ID_REST_BREAK)->get_break_exercises();
    }
  return ret;
}

void
RestBreakWindow::install_exercises_panel()
{
  if (head_info.count != 0)
    {
      install_info_panel();
    }
  else
    {
      set_ignore_activity(true);
      clear_pluggable_panel();
      ExercisesPanel *exercises_panel = manage(new ExercisesPanel(NULL));
      pluggable_panel.pack_start(*exercises_panel, false, false, 0);
      exercises_panel->set_exercise_count(get_exercise_count());
      exercises_panel->signal_stop().connect
        (SigC::slot(*this, &RestBreakWindow::install_info_panel));
      pluggable_panel.show_all();
      pluggable_panel.queue_resize();
      center();
    }
}

void
RestBreakWindow::install_info_panel()
{
  set_ignore_activity(false);
  clear_pluggable_panel();
  pluggable_panel.pack_start(*(create_info_panel()), false, false, 0);
  pluggable_panel.show_all();
  pluggable_panel.queue_resize();
  center();
}

void
RestBreakWindow::set_ignore_activity(bool i)
{
  CoreInterface *core = CoreFactory::get_core();
  assert(core != NULL);
  BreakInterface *bi = core->get_break(BREAK_ID_REST_BREAK);
  assert(bi != NULL);

  bi->set_insist_policy(i ? BreakInterface::INSIST_POLICY_IGNORE : BreakInterface::INSIST_POLICY_HALT);
}
#endif
