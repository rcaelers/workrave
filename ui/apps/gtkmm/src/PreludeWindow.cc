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

#include <gtkmm.h>
#include <gdkmm/devicemanager.h>

#include "debug.hh"
#include "commonui/nls.h"

#include "Text.hh"
#include "utils/AssetPath.hh"
#include "utils/Platform.hh"

#include "commonui/Backend.hh"
#include "core/ICore.hh"

#include "core/IBreakResponse.hh"
#include "PreludeWindow.hh"
#include "WindowHints.hh"
#include "Frame.hh"
#include "TimeBar.hh"
#include "Hig.hh"
#include "GtkUtil.hh"

#ifdef PLATFORM_OS_WINDOWS
#  include <gdk/gdkwin32.h>
#endif

using namespace std;
using namespace workrave;
using namespace workrave::utils;

//! Construct a new Microbreak window.
PreludeWindow::PreludeWindow(HeadInfo &head, BreakId break_id)
  : Gtk::Window(Gtk::WINDOW_POPUP)
{
  TRACE_ENTER("PreludeWindow::PreludeWindow");

  // On W32, must be *before* realize, otherwise a border is drawn.
  set_resizable(false);
  set_decorated(false);
  set_position(Gtk::WIN_POS_CENTER_ALWAYS);

  Gtk::Window::set_border_width(0);

  if (GtkUtil::running_on_wayland())
    {
      set_app_paintable(true);
      signal_draw().connect(sigc::mem_fun(*this, &PreludeWindow::on_draw_event));
      signal_screen_changed().connect(sigc::mem_fun(*this, &PreludeWindow::on_screen_changed_event));
      on_screen_changed_event(get_screen());
      set_size_request(head.get_width(), head.get_height());
    }

#ifdef PLATFORM_OS_WINDOWS
  init_avoid_pointer_polling();
#endif

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

  show_all_children();
  stick();

  this->head = head;
  Gtk::Window::set_screen(head.screen);
  TRACE_EXIT();
}

PreludeWindow::~PreludeWindow()
{
#ifdef PLATFORM_OS_WINDOWS
  if (avoid_signal.connected())
    {
      avoid_signal.disconnect();
    }
#endif
}

void
PreludeWindow::start()
{
  TRACE_ENTER("PreludeWindow::start");

  // Need to realize window before it is shown
  // Otherwise, there is not gobj()...
  realize_if_needed();

  // Set some window hints.
  set_skip_pager_hint(true);
  set_skip_taskbar_hint(true);

  WindowHints::set_always_on_top(this, true);

  refresh();

  GtkUtil::center_window(*this, head);
  show_all();

  WindowHints::set_always_on_top(this, true);

  time_bar->set_bar_color(TimerColorId::Overdue);

  TRACE_EXIT();
}

//! Adds a child to the window.
void
PreludeWindow::add(Gtk::Widget &widget)
{
  if (!window_frame)
    {
      window_frame = Gtk::manage(new Frame());
      window_frame->set_border_width(0);
      window_frame->set_frame_style(Frame::STYLE_BREAK_WINDOW);

      if (GtkUtil::running_on_wayland())
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

#ifndef PLATFORM_OS_WINDOWS
      window_frame->add_events(Gdk::ENTER_NOTIFY_MASK);
      window_frame->signal_enter_notify_event().connect(sigc::mem_fun(*this, &PreludeWindow::on_enter_notify_event));
#endif
    }

  window_frame->add(widget);
}

//! Stops the microbreak.
void
PreludeWindow::stop()
{
  TRACE_ENTER("PreludeWindow::stop");

  frame->set_frame_flashing(0);
  hide();

  TRACE_EXIT();
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
        tminus = 0;

      sprintf(s, progress_text.c_str(), Text::time_to_string(tminus).c_str());
    }
  time_bar->set_text(s);
  time_bar->update();

#if defined(PLATFORM_OS_WINDOWS)
  // Vista GTK phantom toplevel parent kludge:
  HWND hwnd = (HWND)GDK_WINDOW_HWND(gtk_widget_get_window(Gtk::Widget::gobj()));
  if (hwnd)
    {
      HWND hAncestor = GetAncestor(hwnd, GA_ROOT);
      HWND hDesktop = GetDesktopWindow();
      if (hAncestor && hDesktop && hAncestor != hDesktop)
        hwnd = hAncestor;
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
          int winx, winy;
          get_position(winx, winy);
          set_position(Gtk::WIN_POS_NONE);
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
  TRACE_ENTER("PreludeWindow::on_frame_flash_event");
  flash_visible = frame_visible;
  refresh();
  TRACE_EXIT();
}

#ifdef PLATFORM_OS_WINDOWS
// Sets whether the window should run away for the mouse pointer.
void
PreludeWindow::init_avoid_pointer_polling()
{
  TRACE_ENTER("PreludeWindow::init_avoid_pointer_polling");
  if (!avoid_signal.connected())
    {
      int x, y;
      get_pointer_location(x, y);

      TRACE_MSG("d " << x << " " << y);

      POINT p;
      GetCursorPos(&p);
      TRACE_MSG("p " << p.x << " " << p.y);
      gdk_offset_x = p.x - x;
      gdk_offset_y = p.y - y;
      TRACE_MSG("offset " << gdk_offset_x << " " << gdk_offset_y);

      avoid_signal = Glib::signal_timeout().connect(sigc::mem_fun(*this, &PreludeWindow::on_avoid_pointer_timer_event), 150);
    }
  did_avoid = false;
  TRACE_EXIT();
}

bool
PreludeWindow::on_avoid_pointer_timer_event()
{
  TRACE_ENTER("PreludeWindow::on_avoid_pointer_timer");
  /*
    display->get_pointer reads low-level keyboard state, and that's a
    problem for anti-hook monitors. use GetCursorPos() instead.
  */
  POINT p;
  GetCursorPos(&p);

  int winx, winy;
  set_gravity(Gdk::GRAVITY_STATIC);
  get_position(winx, winy);

  int width, height;
  Gtk::Allocation a = frame->get_allocation();
  width = a.get_width();
  height = a.get_height();

  int px = p.x - gdk_offset_x;
  int py = p.y - gdk_offset_y;

  if (px >= winx && px < winx + width && py >= winy && py < winy + height)
    {
      avoid_pointer();
    }

  TRACE_EXIT();
  return true;
}

#endif // PLATFORM_OS_WINDOWS

#ifndef PLATFORM_OS_WINDOWS

//! GDK EventNotifyEvent notification.
bool
PreludeWindow::on_enter_notify_event(GdkEventCrossing *event)
{
  (void)event;

  avoid_pointer();
  return false;
}

#endif

//! Move window if pointer is neat specified location.
void
PreludeWindow::avoid_pointer()
{
  TRACE_ENTER("PreludeWindow::avoid_pointer");
  Glib::RefPtr<Gdk::Window> window = get_window();

  did_avoid = true;

  int winx, winy, width, height;
  if (GtkUtil::running_on_wayland())
    {
      Gtk::Allocation a = frame->get_allocation();
      winx = a.get_x();
      winy = a.get_y();
      width = a.get_width();
      height = a.get_height();
    }
  else
    {
      window->get_geometry(winx, winy, width, height);
    }

  TRACE_MSG("x:" << winx << " y:" << winy << " w:" << width << " h:" << height);

  int screen_height = head.get_height();
  int top_y = head.get_y() + SCREEN_MARGIN;
  int bottom_y = head.get_y() + screen_height - height - SCREEN_MARGIN;

  if (winy > screen_height / 2)
    {
      winy = top_y;
    }
  else
    {
      winy = bottom_y;
    }

  TRACE_MSG("new y:" << winy << " ty:" << top_y << " by:" << bottom_y << " h:" << screen_height);

  if (GtkUtil::running_on_wayland())
    {
      if (winy == bottom_y)
        {
          align->set(0.5, 0.9, 0.0, 0.0);
        }
      else
        {
          align->set(0.5, 0.1, 0.0, 0.0);
        }

      return;
    }

  set_position(Gtk::WIN_POS_NONE);
  move(winx, winy);
  TRACE_EXIT();
}

bool
PreludeWindow::on_draw_event(const Cairo::RefPtr<Cairo::Context> &cr)
{
  cr->save();
  cr->set_source_rgba(0.0, 0.0, 0.0, 0.0);
  cr->set_operator(Cairo::OPERATOR_SOURCE);
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
  if (GtkUtil::running_on_wayland())
    {
      Glib::RefPtr<Gdk::Window> window = get_window();

      if (window)
        {
          Cairo::RectangleInt rect = {allocation.get_x(), allocation.get_y(), allocation.get_width(), allocation.get_height()};

          window->input_shape_combine_region(Cairo::Region::create(rect), 0, 0);
        }
    }
}

void
PreludeWindow::get_pointer_location(int &x, int &y)
{
  Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
  Glib::RefPtr<Gdk::DeviceManager> device_manager = display->get_device_manager();
  Glib::RefPtr<Gdk::Device> device = device_manager->get_client_pointer();
  device->get_position(x, y);
}
