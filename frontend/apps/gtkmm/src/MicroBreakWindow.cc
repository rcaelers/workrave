// MicroBreakWindow.cc --- window for the microbreak
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"

#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/image.h>

#include "nls.h"
#include "debug.hh"

#include "IBreak.hh"
#include "GUI.hh"
#include "CoreFactory.hh"
#include "MicroBreakWindow.hh"
#include "WindowHints.hh"
#include "TimeBar.hh"
#include "utils/AssetPath.hh"
#include "GtkUtil.hh"
#include "Text.hh"
#include "Hig.hh"
#include "Frame.hh"

using namespace workrave::utils;

//! Construct a new Microbreak window.
MicroBreakWindow::MicroBreakWindow(HeadInfo &head, BreakFlags break_flags, GUIConfig::BlockMode mode) :
  BreakWindow(BREAK_ID_MICRO_BREAK, head, break_flags, mode),
  progress_value(0),
  progress_max_value(0),
  is_flashing(false),
  fixed_size(false)
{
  set_title(_("Micro-break"));
}

Gtk::Widget *
MicroBreakWindow::create_gui()
{
  // Time bar
  time_bar = Gtk::manage(new TimeBar);
  time_bar->set_text("Microbreak 0:32");

  // Label
  label = Gtk::manage(new Gtk::Label());

  // Icon
  string icon = AssetPath::complete_directory("micro-break.png", AssetPath::SEARCH_PATH_IMAGES);
  Gtk::Image *img = Gtk::manage(new Gtk::Image(icon));
  img->set_alignment(0.0, 0.0);

  // HBox
  Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox(false, 12));
  hbox->pack_start(*img, false, false, 0);
  hbox->pack_start(*label, Gtk::PACK_EXPAND_PADDING, 0);

  // Overall vbox
  Gtk::VBox *box = new Gtk::VBox(false, 12);
  box->pack_start(*hbox, Gtk::PACK_EXPAND_WIDGET, 0);
  box->pack_start(*time_bar, Gtk::PACK_EXPAND_WIDGET, 0);

  // Button box at the bottom.
  ICore::Ptr core = CoreFactory::get_core();
  IBreak::Ptr restbreak =  core->get_break(BREAK_ID_REST_BREAK);
  if ((break_flags != BREAK_FLAGS_NONE) || restbreak->is_enabled())
    {
      Gtk::HBox *button_box;
      if (break_flags != BREAK_FLAGS_NONE)
        {
          button_box = Gtk::manage(new Gtk::HBox(false, 6));

          Gtk::HBox *bbox = Gtk::manage(new Gtk::HBox(true, 6));

          if ((break_flags & BREAK_FLAGS_POSTPONABLE) != 0)
            {
              Gtk::Button *postpone_button = create_postpone_button();
              bbox->pack_end(*postpone_button, Gtk::PACK_EXPAND_WIDGET, 0);
            }

          if ((break_flags & BREAK_FLAGS_SKIPPABLE) != 0)
            {
              Gtk::Button *skip_button = create_skip_button();
              bbox->pack_end(*skip_button, Gtk::PACK_EXPAND_WIDGET, 0);
            }

          Gtk::Alignment *bboxa =
            Gtk::manage(new Gtk::Alignment(1.0, 0.0, 0.0, 0.0));
          bboxa->add(*bbox);

          if (restbreak->is_enabled())
            {
              button_box->pack_start
                (*Gtk::manage(create_restbreaknow_button(false)),
                 Gtk::PACK_SHRINK, 0);
            }
          button_box->pack_end(*bboxa,
                                 Gtk::PACK_EXPAND_WIDGET, 0);
        }
      else
        {
          button_box = Gtk::manage(new Gtk::HBox(false, 6));
          button_box->pack_end(*Gtk::manage(create_restbreaknow_button(true)),
                               Gtk::PACK_SHRINK, 0);
        }
      box->pack_start(*button_box, Gtk::PACK_EXPAND_WIDGET, 0);
    }

  fixed_size = false;

  return box;
}


//! Destructor.
MicroBreakWindow::~MicroBreakWindow()
{
  TRACE_ENTER("MicroBreakWindow::~MicroBreakWindow");
  TRACE_EXIT();
}



Gtk::Button *
MicroBreakWindow::create_restbreaknow_button(bool label)
{
  Gtk::Button *ret;
  ret = Gtk::manage(GtkUtil::create_image_button(_("Rest break"),
                                            "timer-rest-break.png",
                                            label));
  ret->signal_clicked()
    .connect(sigc::mem_fun(*this,
                         &MicroBreakWindow::on_restbreaknow_button_clicked));
#ifdef HAVE_GTK3
  ret->set_can_focus(false);
#else
  ret->unset_flags(Gtk::CAN_FOCUS);
#endif

  return ret;
}

//! The restbreak button was clicked.
void
MicroBreakWindow::on_restbreaknow_button_clicked()
{
  IGUI *gui = GUI::get_instance();
  assert(gui != NULL);

  gui->restbreak_now();
}


void
MicroBreakWindow::update_time_bar()
{
  TRACE_ENTER("MicroBreakWindow::refresh_time_bar");

  time_t time = progress_max_value - progress_value;
  string s = _("Micro-break");
  s += ' ';
  s += Text::time_to_string(time);

  time_bar->set_progress(progress_value, progress_max_value - 1);
  time_bar->set_text(s);

  ICore::Ptr core = CoreFactory::get_core();
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
MicroBreakWindow::update_label()
{
  TRACE_ENTER("MicroBreakWindow::refresh_label");

  ICore::Ptr core = CoreFactory::get_core();

  IBreak::Ptr restbreak_timer =  core->get_break(BREAK_ID_REST_BREAK);
  IBreak::Ptr daily_timer =  core->get_break(BREAK_ID_DAILY_LIMIT);

  BreakId show_next = BREAK_ID_NONE;

  time_t rb = restbreak_timer->get_limit() - restbreak_timer->get_elapsed_time();
  time_t dl = daily_timer->get_limit() - daily_timer->get_elapsed_time();

  if (restbreak_timer->is_enabled())
    {
      show_next = BREAK_ID_REST_BREAK;
    }

  if (daily_timer->is_enabled())
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
MicroBreakWindow::update_break_window()
{
  update_time_bar();
  update_label();

  if (!fixed_size)
    {
      // Make sure the label doesn't resize anymore.
      // There has to be a better way to do this...
#ifdef HAVE_GTK3
      GtkRequisition min_size;
      GtkRequisition natural_size;
      label->get_preferred_size(min_size, natural_size);
#else
      Gtk::Requisition min_size = label->size_request();
#endif

      label->set_size_request(min_size.width, min_size.height);
      fixed_size = true;
      center();
    }
}


void
MicroBreakWindow::set_progress(int value, int max_value)
{
  progress_max_value = max_value;
  progress_value = value;
}
