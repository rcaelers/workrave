// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#if defined(PLATFORM_OS_WINDOWS)
#  include "ui/windows/WindowsForceFocus.hh"
#  undef ERROR
#  undef IN
#  undef OUT
#  undef WINDING
#endif

#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>

#include "debug.hh"
#include "commonui/nls.h"

#include "Hig.hh"
#include "RestBreakWindow.hh"
#include "commonui/Text.hh"
#include "TimeBar.hh"

#include "core/IBreak.hh"
#include "GtkUtil.hh"
#include "Frame.hh"

#include "core/ICore.hh"
#include "config/IConfigurator.hh"

#include "commonui/Exercise.hh"
#include "ExercisesPanel.hh"

using namespace std;
using namespace workrave;
using namespace workrave::utils;

RestBreakWindow::RestBreakWindow(std::shared_ptr<IApplicationContext> app, HeadInfo head, BreakFlags break_flags, BlockMode mode)
  : BreakWindow(app, BREAK_ID_REST_BREAK, head, break_flags, mode)
{
  TRACE_ENTRY();
  set_title(_("Rest break"));
}

Gtk::Widget *
RestBreakWindow::create_gui()
{
  // Add other widgets.
  auto *vbox = new Gtk::VBox(false, 6);

  pluggable_panel = Gtk::manage(new Gtk::HBox);

  vbox->pack_start(*pluggable_panel, false, false, 0);

  // Timebar
  timebar = Gtk::manage(new TimeBar);
  vbox->pack_start(*timebar, false, false, 6);

  Gtk::Box *bottom_box = create_bottom_box(true, GUIConfig::break_enable_shutdown(BREAK_ID_REST_BREAK)());
  if (bottom_box != nullptr)
    {
      vbox->pack_end(*Gtk::manage(bottom_box), Gtk::PACK_SHRINK, 6);
    }

  return vbox;
}

RestBreakWindow::~RestBreakWindow()
{
  TRACE_ENTRY();
}

void
RestBreakWindow::start()
{
  TRACE_ENTRY();
  init_gui();
  if (get_exercise_count() > 0)
    {
      install_exercises_panel();
    }
  else
    {
      install_info_panel();
    }

  update_break_window();

  BreakWindow::start();
}

void
RestBreakWindow::update_break_window()
{
  draw_time_bar();
}

void
RestBreakWindow::set_progress(int value, int max_value)
{
  progress_max_value = max_value;
  progress_value = value;
}

void
RestBreakWindow::draw_time_bar()
{
  timebar->set_progress(progress_value, progress_max_value);

  time_t time = progress_max_value - progress_value;
  char s[128];
  sprintf(s, _("Rest break for %s"), Text::time_to_string(time, true).c_str());

  timebar->set_text(s);

  auto core = app->get_core();
  bool user_active = core->is_user_active();
  if (frame != nullptr)
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

  timebar->update();
}

Gtk::Widget *
RestBreakWindow::create_info_panel()
{
  Gtk::HBox *info_box = Gtk::manage(new Gtk::HBox(false, 12));

  Gtk::Image *info_img = GtkUtil::create_image("rest-break.png");
  info_img->set_alignment(0.0, 0.0);
  Gtk::Label *info_lab = Gtk::manage(new Gtk::Label());
  Glib::ustring txt;

  if ((break_flags & BREAK_FLAGS_NATURAL) != 0)
    {
      txt = HigUtil::create_alert_text(_("Natural rest break"), _("This is your natural rest break."));
    }
  else
    {
      txt = HigUtil::create_alert_text(_("Rest break"),
                                       _("This is your rest break. Make sure you stand up and\n"
                                         "walk away from your computer on a regular basis. Just\n"
                                         "walk around for a few minutes, stretch, and relax."));
    }

  GtkUtil::set_theme_fg_color(info_lab);

  info_lab->set_markup(txt);
  info_box->pack_start(*info_img, false, false, 0);
  info_box->pack_start(*info_lab, false, true, 0);
  return info_box;
}

void
RestBreakWindow::clear_pluggable_panel()
{
  TRACE_ENTRY();
  auto children = pluggable_panel->get_children();
  if (!children.empty())
    {
      TRACE_MSG("Clearing");
      pluggable_panel->remove(*(*(children.begin())));
    }
}

int
RestBreakWindow::get_exercise_count()
{
  int ret = 0;

  if (app->get_exercises()->has_exercises())
    {
      ret = GUIConfig::break_exercises(BREAK_ID_REST_BREAK)();
    }
  return ret;
}

void
RestBreakWindow::install_exercises_panel()
{
  if (!head.is_primary() || ((break_flags & BREAK_FLAGS_NO_EXERCISES) != 0))
    {
      install_info_panel();
    }
  else
    {
      set_ignore_activity(true);
      clear_pluggable_panel();
      ExercisesPanel *exercises_panel = Gtk::manage(new ExercisesPanel(app->get_sound_theme(), app->get_exercises(), nullptr));
      pluggable_panel->pack_start(*exercises_panel, false, false, 0);
      exercises_panel->set_exercise_count(get_exercise_count());
      exercises_panel->signal_stop().connect(sigc::mem_fun(*this, &RestBreakWindow::install_info_panel));
      pluggable_panel->show_all();
      pluggable_panel->queue_resize();
    }
  center();
}

void
RestBreakWindow::install_info_panel()
{
  Gtk::Requisition old_size;
  Gtk::Requisition natural_size;
  get_preferred_size(old_size, natural_size);

  set_ignore_activity(false);
  clear_pluggable_panel();
  pluggable_panel->pack_start(*(create_info_panel()), false, false, 0);
  pluggable_panel->show_all();
  pluggable_panel->queue_resize();

  BlockMode block_mode = GUIConfig::block_mode()();
  if (block_mode == BlockMode::Off && head.is_primary())
    {
      Gtk::Requisition new_size;
      get_preferred_size(new_size, natural_size);

      int width_delta = (new_size.width - old_size.width) / 2;
      int height_delta = (new_size.height - old_size.height) / 2;

      int x = 0;
      int y = 0;
      get_position(x, y);
      move(x - width_delta, y - height_delta);
    }
  else
    {
      center();
    }
}

void
RestBreakWindow::set_ignore_activity(bool i)
{
  auto core = app->get_core();

#if defined(PLATFORM_OS_WINDOWS)
  if (WindowsForceFocus::GetForceFocusValue())
    {
      i = true;
    }
#endif

  core->set_insist_policy(i ? InsistPolicy::Ignore : InsistPolicy::Halt);
}
