// Copyright (C) 2001 - 2012 Rob Caelers & Raymond Penners
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
#  include "config.h"
#endif

#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/image.h>

#include "commonui/nls.h"
#include "debug.hh"

#include "core/IBreak.hh"
//#include "Application.hh"
#include "MicroBreakWindow.hh"
#include "TimeBar.hh"
#include "GtkUtil.hh"
#include "commonui/Text.hh"
#include "Hig.hh"
#include "Frame.hh"
#include "ui/IApplicationContext.hh"

using namespace std;
using namespace workrave;

//! Construct a new Microbreak window.
MicroBreakWindow::MicroBreakWindow(std::shared_ptr<IApplicationContext> app,
                                   HeadInfo head,
                                   BreakFlags break_flags,
                                   BlockMode mode)
  : BreakWindow(app, BREAK_ID_MICRO_BREAK, head, break_flags, mode)
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
  Gtk::Image *img = GtkUtil::create_image("micro-break.png");
  img->set_alignment(0.0, 0.0);

  // Box
  auto *hbox = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::HORIZONTAL, 12));
  hbox->pack_start(*img, false, false, 0);
  hbox->pack_start(*label, Gtk::PACK_EXPAND_PADDING, 0);

  // Overall vbox
  auto *box = new GtkCompat::Box(Gtk::Orientation::VERTICAL, 12);
  box->pack_start(*hbox, Gtk::PACK_EXPAND_WIDGET, 0);
  box->pack_start(*time_bar, Gtk::PACK_EXPAND_WIDGET, 0);

  // Button box at the bottom.
  auto core = app->get_core();
  auto restbreak = core->get_break(BREAK_ID_REST_BREAK);

  if ((break_flags != BREAK_FLAGS_NONE) || restbreak->is_enabled())
    {
      GtkCompat::Box *button_box;
      if (break_flags != BREAK_FLAGS_NONE)
        {
          button_box = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::HORIZONTAL, 6));

          auto *bbox = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::HORIZONTAL, 6));

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

          Gtk::Alignment *bboxa = Gtk::manage(new Gtk::Alignment(1.0, 0.0, 0.0, 0.0));
          bboxa->add(*bbox);

          if (restbreak->is_enabled())
            {
              button_box->pack_start(*Gtk::manage(create_restbreaknow_button(false)), Gtk::PACK_SHRINK, 0);
            }
          button_box->pack_end(*bboxa, Gtk::PACK_EXPAND_WIDGET, 0);
        }
      else
        {
          button_box = Gtk::manage(new GtkCompat::Box(Gtk::Orientation::HORIZONTAL, 6));
          button_box->pack_end(*Gtk::manage(create_restbreaknow_button(true)), Gtk::PACK_SHRINK, 0);
        }
      box->pack_start(*button_box, Gtk::PACK_EXPAND_WIDGET, 0);
    }

  fixed_size = false;

  return box;
}

Gtk::Button *
MicroBreakWindow::create_restbreaknow_button(bool label)
{
  Gtk::Button *ret;
  ret = Gtk::manage(GtkUtil::create_image_button(_("Rest break"), "timer-rest-break.png", label));
  ret->signal_clicked().connect(sigc::mem_fun(*this, &MicroBreakWindow::on_restbreaknow_button_clicked));
  ret->set_can_focus(false);

  return ret;
}

void
MicroBreakWindow::on_restbreaknow_button_clicked()
{
  auto core = app->get_core();
  core->force_break(BREAK_ID_REST_BREAK, BreakHint::Normal);
}

void
MicroBreakWindow::update_time_bar()
{
  TRACE_ENTRY();
  time_t time = progress_max_value - progress_value;
  string s = _("Micro-break");
  s += ' ';
  s += Text::time_to_string(time);

  time_bar->set_progress(progress_value, progress_max_value - 1);
  time_bar->set_text(s);

  auto core = app->get_core();
  bool user_active = core->is_user_active();
  if (frame != nullptr)
    {
      if (user_active && !is_flashing)
        {
          frame->set_frame_color(Gdk::RGBA("orange"));
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
  TRACE_VAR(progress_value, progress_max_value);
}

void
MicroBreakWindow::update_label()
{
  TRACE_ENTRY();
  auto core = app->get_core();
  auto restbreak_timer = core->get_break(BREAK_ID_REST_BREAK);
  auto daily_timer = core->get_break(BREAK_ID_DAILY_LIMIT);

  BreakId show_next = BREAK_ID_NONE;

  int64_t rb = restbreak_timer->get_limit() - restbreak_timer->get_elapsed_time();
  int64_t dl = daily_timer->get_limit() - daily_timer->get_elapsed_time();

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
          sprintf(s, _("Next rest break in %s"), Text::time_to_string(rb, true).c_str());
        }
      else
        {
          sprintf(s, _("Rest break %s overdue"), Text::time_to_string(-rb, true).c_str());
        }

      txt += "\n";
      txt += s;
    }
  else if (show_next == BREAK_ID_DAILY_LIMIT)
    {
      char s[128];

      if (dl >= 0)
        {
          sprintf(s, _("Daily limit in %s"), Text::time_to_string(dl, true).c_str());
        }
      else
        {
          sprintf(s, _("Daily limit %s overdue"), Text::time_to_string(-dl, true).c_str());
        }

      txt += "\n";
      txt += s;
    }

  label->set_markup(HigUtil::create_alert_text(_("Micro-break"), txt.c_str()));
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
      GtkRequisition min_size;
      GtkRequisition natural_size;
      label->get_preferred_size(min_size, natural_size);

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
