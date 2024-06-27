// Copyright (C) 2001 - 2017 Rob Caelers & Raymond Penners
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

#include "PreludeWindow.hh"

#include <gtkmm.h>

#include "commonui/nls.h"
#include "core/ICore.hh"
#include "debug.hh"
#include "utils/Platform.hh"
#include "commonui/Text.hh"

#include "Frame.hh"
#include "TimeBar.hh"
#include "Hig.hh"
#include "GtkUtil.hh"

#if defined(PLATFORM_OS_WINDOWS)
#  include <gdk/gdkwin32.h>
#endif

using namespace std;
using namespace workrave;
using namespace workrave::utils;

PreludeWindow::PreludeWindow(HeadInfo head, BreakId break_id)
  : Gtk::Window(Gtk::WINDOW_POPUP)
{
  TRACE_ENTRY();
#if defined(HAVE_WAYLAND)
  if (Platform::running_on_wayland())
    {
      auto wm = std::make_shared<WaylandWindowManager>();
      bool success = wm->init();
      if (success)
        {
          window_manager = wm;
        }
    }
#endif

  // On W32, must be *before* realize, otherwise a border is drawn.
  set_resizable(false);
  set_decorated(false);
  set_position(Gtk::WIN_POS_CENTER_ALWAYS);

  Gtk::Window::set_border_width(0);

  if (!Platform::can_position_windows())
    {
      set_app_paintable(true);
      signal_draw().connect(sigc::mem_fun(*this, &PreludeWindow::on_draw_event), false);
      signal_screen_changed().connect(sigc::mem_fun(*this, &PreludeWindow::on_screen_changed_event));
      on_screen_changed_event(get_screen());
      set_size_request(head.get_width(), head.get_height());
    }

  realize();

  time_bar = Gtk::manage(new TimeBar);
  label = Gtk::manage(new Gtk::Label());

  Gtk::VBox *vbox = Gtk::manage(new Gtk::VBox(false, 6));
  vbox->pack_start(*label, false, false, 0);
  vbox->pack_start(*time_bar, false, false, 0);

  image_icon = Gtk::manage(new Gtk::Image());

  Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox(false, 6));
  hbox->pack_start(*image_icon, false, false, 0);
  hbox->pack_start(*vbox, false, false, 0);

  frame = Gtk::manage(new Frame);
  frame->set_frame_style(Frame::STYLE_SOLID);
  frame->set_frame_width(6);
  frame->set_border_width(6);
  frame->add(*hbox);
  frame->signal_flash().connect(sigc::mem_fun(*this, &PreludeWindow::on_frame_flash_event));
  flash_visible = true;
  color_warn = Gdk::Color("orange");
  color_alert = Gdk::Color("red");

  add(*frame);

  switch (break_id)
    {
    case BREAK_ID_MICRO_BREAK:
      label->set_markup(HigUtil::create_alert_text(_("Time for a micro-break?"), nullptr));
      break;

    case BREAK_ID_REST_BREAK:
      label->set_markup(HigUtil::create_alert_text(_("You need a rest break..."), nullptr));
      break;

    case BREAK_ID_DAILY_LIMIT:
      label->set_markup(HigUtil::create_alert_text(_("You should stop for today..."), nullptr));
      break;

    default:
      break;
    }

  set_can_focus(false);
  set_accept_focus(false);
  set_focus_on_map(false);

  show_all_children();
  stick();

  this->head = head;
}

void
PreludeWindow::start()
{
  TRACE_ENTRY();
  // Need to realize window before it is shown
  // Otherwise, there is no gobj()...
  realize_if_needed();

#if defined(HAVE_WAYLAND)
  if (window_manager)
    {
      window_manager->init_surface(*this, head.get_monitor(), false);
    }
#endif

  // Set some window hints.
  set_skip_pager_hint(true);
  set_skip_taskbar_hint(true);

  GtkUtil::set_always_on_top(this, true);

  refresh();

  GtkUtil::center_window(*this, head);
  show_all();

  GtkUtil::set_always_on_top(this, true);

  time_bar->set_bar_color(TimerColorId::Overdue);
}

//! Adds a child to the window.
void
PreludeWindow::add(Gtk::Widget &widget)
{
  if (window_frame == nullptr)
    {
      window_frame = Gtk::manage(new Frame());
      window_frame->set_border_width(0);
      window_frame->set_frame_style(Frame::STYLE_BREAK_WINDOW);

      if (!Platform::can_position_windows())
        {
          align = Gtk::manage(new Gtk::Alignment(0.5, 0.5, 0.0, 0.0));
          align->add(*window_frame);
          Gtk::Window::add(*align);

          widget.signal_size_allocate().connect(sigc::mem_fun(*this, &PreludeWindow::on_size_allocate_event));
        }
      else
        {
          Gtk::Window::add(*window_frame);
        }

      window_frame->add_events(Gdk::ENTER_NOTIFY_MASK);
      window_frame->signal_enter_notify_event().connect(sigc::mem_fun(*this, &PreludeWindow::on_enter_notify_event), false);
    }

  window_frame->add(widget);
}

//! Stops the microbreak.
void
PreludeWindow::stop()
{
  TRACE_ENTRY();
  frame->set_frame_flashing(0);

#if defined(HAVE_WAYLAND)
  if (window_manager)
    {
      window_manager->clear_surfaces();
    }
#endif

  hide();
}

//! Refresh window.
void
PreludeWindow::refresh()
{
  char s[128] = "";

  time_bar->set_progress(progress_value, progress_max_value);

  int tminus = progress_max_value - progress_value;
  if (tminus >= 0 || (tminus < 0 && flash_visible))
    {
      if (tminus < 0)
        {
          tminus = 0;
        }

      sprintf(s, progress_text.c_str(), Text::time_to_string(tminus).c_str());
    }
  time_bar->set_text(static_cast<const char *>(s));
  time_bar->update();

#if defined(PLATFORM_OS_WINDOWS)
  // Vista GTK phantom toplevel parent kludge:
  HWND hwnd = (HWND)GDK_WINDOW_HWND(gtk_widget_get_window(Gtk::Widget::gobj()));
  if (hwnd)
    {
      HWND hAncestor = GetAncestor(hwnd, GA_ROOT);
      HWND hDesktop = GetDesktopWindow();
      if (hAncestor && hDesktop && hAncestor != hDesktop)
        {
          hwnd = hAncestor;
        }
      // Set toplevel window topmost!
      SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    }
#endif
}

void
PreludeWindow::set_progress(int value, int max_value)
{
  progress_value = value;
  progress_max_value = max_value;
  refresh();
}

void
PreludeWindow::set_progress_text(IApp::PreludeProgressText text)
{
  switch (text)
    {
    case IApp::PROGRESS_TEXT_BREAK_IN:
      progress_text = _("Break in %s");
      break;

    case IApp::PROGRESS_TEXT_DISAPPEARS_IN:
      progress_text = _("Disappears in %s");
      break;

    case IApp::PROGRESS_TEXT_SILENT_IN:
      progress_text = _("Silent in %s");
      break;
    }
}

void
PreludeWindow::set_stage(IApp::PreludeStage stage)
{
  const char *icon = nullptr;
  switch (stage)
    {
    case IApp::STAGE_INITIAL:
      frame->set_frame_flashing(0);
      frame->set_frame_visible(false);
      icon = "prelude-hint.png";
      break;

    case IApp::STAGE_WARN:
      frame->set_frame_visible(true);
      frame->set_frame_flashing(500);
      frame->set_frame_color(color_warn);
      icon = "prelude-hint-sad.png";
      break;

    case IApp::STAGE_ALERT:
      frame->set_frame_flashing(500);
      frame->set_frame_color(color_alert);
      icon = "prelude-hint-sad.png";
      break;

    case IApp::STAGE_MOVE_OUT:
      if (!did_avoid)
        {
          auto [winx, winy] = GtkUtil::get_centered_position(*this, head);
          move(winx, head.get_y() + SCREEN_MARGIN);
        }
      break;
    }
  if (icon != nullptr)
    {
      string file = GtkUtil::get_image_filename(icon);
      image_icon->set(file);
    }
}

void
PreludeWindow::on_frame_flash_event(bool frame_visible)
{
  TRACE_ENTRY();
  flash_visible = frame_visible;
  refresh();
}

bool
PreludeWindow::on_enter_notify_event(GdkEventCrossing *event)
{
  (void)event;
  avoid_pointer();
  return false;
}

void
PreludeWindow::avoid_pointer()
{
  TRACE_ENTRY();

  did_avoid = true;

  int winx = 0;
  int winy = 0;
  int width = 0;
  int height = 0;
  if (!Platform::can_position_windows())
    {
      Gtk::Allocation a = frame->get_allocation();
      winx = a.get_x();
      winy = a.get_y();
      width = a.get_width();
      height = a.get_height();
    }
  else
    {
      Glib::RefPtr<Gdk::Window> window = get_window();
      window->get_geometry(winx, winy, width, height);
    }

  TRACE_MSG("x: {} y: {} w: {} h: {}", winx, winy, width, height);

  int screen_height = head.get_height();
  int top_y = head.get_y() + SCREEN_MARGIN;
  int bottom_y = head.get_y() + screen_height - height - SCREEN_MARGIN;

  if (winy > (head.get_y() + screen_height / 2))
    {
      winy = top_y;
    }
  else
    {
      winy = bottom_y;
    }

  TRACE_MSG("new y: {} ty: {} by:{} h:{}", winy, top_y, bottom_y, screen_height);

  if (!Platform::can_position_windows())
    {
      if (winy == bottom_y)
        {
          align->set(0.5F, 0.9F, 0.0F, 0.0F);
        }
      else
        {
          align->set(0.5F, 0.1F, 0.0F, 0.0F);
        }
    }
  else
    {
      set_position(Gtk::WIN_POS_NONE);
      move(winx, winy);
    }
}

bool
PreludeWindow::on_draw_event(const Cairo::RefPtr<Cairo::Context> &cr)
{
  cr->save();
  cr->set_source_rgba(0.0, 0.0, 0.0, 0.0);
#if CAIROMM_CHECK_VERSION(1, 15, 4)
  cr->set_operator(Cairo::Context::Operator::SOURCE);
#else
  cr->set_operator(Cairo::OPERATOR_SOURCE);
#endif
  cr->paint();
  cr->restore();

  return Gtk::Window::on_draw(cr);
}

void
PreludeWindow::on_screen_changed_event(const Glib::RefPtr<Gdk::Screen> &previous_screen)
{
  (void)previous_screen;

  const Glib::RefPtr<Gdk::Screen> screen = get_screen();
  const Glib::RefPtr<Gdk::Visual> visual = screen->get_rgba_visual();

  if (visual)
    {
      gtk_widget_set_visual(GTK_WIDGET(gobj()), visual->gobj());
    }
}

void
PreludeWindow::on_size_allocate_event(Gtk::Allocation &allocation)
{
  update_input_region(allocation);
}

void
PreludeWindow::update_input_region(Gtk::Allocation &allocation)
{
  if (!Platform::can_position_windows())
    {
      Glib::RefPtr<Gdk::Window> window = get_window();

      if (window)
        {
          Cairo::RectangleInt rect = {allocation.get_x(), allocation.get_y(), allocation.get_width(), allocation.get_height()};

          window->input_shape_combine_region(Cairo::Region::create(rect), 0, 0);
        }
    }
}
