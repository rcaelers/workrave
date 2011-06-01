// PreludeWindow.cc
//
// Copyright (C) 2001, 2002, 2003, 2004, 2006, 2007, 2008, 2009, 2011 Rob Caelers & Raymond Penners
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
#include <gtkmm/image.h>
#include <gtkmm/box.h>

#include "debug.hh"
#include "nls.h"

#include "Text.hh"
#include "Util.hh"

#include "CoreFactory.hh"
#include "ICore.hh"

#include "IBreakResponse.hh"
#include "PreludeWindow.hh"
#include "WindowHints.hh"
#include "Frame.hh"
#include "TimeBar.hh"
#include "Hig.hh"
#include "GtkUtil.hh"

#ifdef PLATFORM_OS_WIN32
#include <gdk/gdkwin32.h>
#endif


//! Construct a new Microbreak window.
PreludeWindow::PreludeWindow(HeadInfo &head, BreakId break_id)
  : Gtk::Window(Gtk::WINDOW_POPUP),
    SCREEN_MARGIN(20),
    did_avoid(false),
    break_id(break_id),
    time_bar(NULL),
    frame(NULL),
    window_frame(NULL),
    label(NULL),
    image_icon(NULL),
    progress_value(0),
    progress_max_value(0),
    flash_visible(false),
    prelude_response(NULL)
{
  TRACE_ENTER("PreludeWindow::PreludeWindow");
  Gtk::Window::set_border_width(0);

  init_avoid_pointer();
  realize();

  // Time bar
  time_bar = Gtk::manage(new TimeBar);

  // Label
  label = Gtk::manage(new Gtk::Label());

  // Box
  Gtk::VBox *vbox = Gtk::manage(new Gtk::VBox(false, 6));
  vbox->pack_start(*label, false, false, 0);
  vbox->pack_start(*time_bar, false, false, 0);

  // Icon
  image_icon = Gtk::manage(new Gtk::Image());

  // Box
  Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox(false, 6));
  hbox->pack_start(*image_icon, false, false, 0);
  hbox->pack_start(*vbox, false, false, 0);

  // Frame
  frame = Gtk::manage(new Frame);
  frame->set_frame_style(Frame::STYLE_SOLID);
  frame->set_frame_width(6);
  frame->set_border_width(6);
  frame->add(*hbox);
  frame->signal_flash().connect(sigc::mem_fun(*this, &PreludeWindow::on_frame_flash));
  flash_visible = true;
  color_warn = Gdk::Color("orange");
  color_alert = Gdk::Color("red");
  add(*frame);

  switch (break_id)
    {
    case BREAK_ID_MICRO_BREAK:
      label->set_markup(HigUtil::create_alert_text(_("Time for a micro-break?"), NULL));
      break;

    case BREAK_ID_REST_BREAK:
      label->set_markup(HigUtil::create_alert_text(_("You need a rest break..."), NULL));
      break;

    case BREAK_ID_DAILY_LIMIT:
      label->set_markup(HigUtil::create_alert_text(_("You should stop for today..."), NULL));
      break;

    default:
      break;
    }

#ifdef HAVE_GTK3
  set_can_focus(false);
#else  
  unset_flags(Gtk::CAN_FOCUS);
#endif
  
  show_all_children();
  stick();

  // trace window handles:
  // FIXME: debug, remove later
#ifdef PLATFORM_OS_WIN32
  HWND _hwnd = (HWND) GDK_WINDOW_HWND( Gtk::Widget::gobj()->window );
  HWND _scope = (HWND) GDK_WINDOW_HWND( GTK_WIDGET( this->gobj() )->window );
  HWND _hRoot = GetAncestor( _hwnd, GA_ROOT );
  HWND _hParent = GetAncestor( _hwnd, GA_PARENT );
  HWND _hDesktop = GetDesktopWindow();
  
  TRACE_MSG("PreludeWindow created" <<  hex << _hwnd  << dec);
  
  if (_hwnd != _scope)
    {
      TRACE_MSG( "!!!!!!!!!!!!!!!" <<  "Scope issue: " << hex << _scope  << dec);
    }
  
  if (_hwnd != _hRoot)
    {
      TRACE_MSG( "GetDesktopWindow()" <<  hex << _hDesktop  << dec);
      TRACE_MSG( "!!!!!!!!!!!!!!!" <<  "PreludeWindow GA_ROOT: " << hex << _hRoot  << dec);
    }
  
  if( _hParent != _hDesktop )
    {
      TRACE_MSG( "GetDesktopWindow()" <<  hex << _hDesktop  << dec);
      TRACE_MSG( "!!!!!!!!!!!!!!!" << "PreludeWindow GA_PARENT: " << hex << _hParent  << dec);
      
      HWND _hTemp;
      while( IsWindow( _hParent ) && _hParent != _hDesktop )
        {
          _hTemp = _hParent;
          _hParent = GetAncestor( _hTemp, GA_PARENT );
          HWND _hParent2 = (HWND)GetWindowLong( _hTemp, GWL_HWNDPARENT );
          if( _hParent == _hTemp )
            break;
          TRACE_MSG("!!!!!!!!!!!!!!!" << hex << _hTemp << " GA_PARENT: " << hex << _hParent  << dec);
          TRACE_MSG("!!!!!!!!!!!!!!!" << hex << _hTemp << " GWL_HWNDPARENT: " << hex << _hParent2  << dec);
        }
    }
#endif

  this->head = head;
  if (head.valid)
    {
      Gtk::Window::set_screen(head.screen);
    }
  TRACE_EXIT();
}


//! Destructor.
PreludeWindow::~PreludeWindow()
{
#ifdef PLATFORM_OS_WIN32
  if (avoid_signal.connected())
    {
      avoid_signal.disconnect();
    }
#endif
}



//! Starts the microbreak.
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
  
  time_bar->set_bar_color(TimeBar::COLOR_ID_OVERDUE);
  
  TRACE_EXIT();
}

//! Adds a child to the window.
void
PreludeWindow::add(Gtk::Widget& widget)
{
  if (! window_frame)
    {
      window_frame = Gtk::manage(new Frame());
      window_frame->set_border_width(0);
      window_frame->set_frame_style(Frame::STYLE_BREAK_WINDOW);
      Gtk::Window::add(*window_frame);
    }
  window_frame->add(widget);
}

//! Self-Destruct
/*!
 *  This method MUST be used to destroy the objects through the
 *  IPreludeWindow. it is NOT possible to do a delete on
 *  this interface...
 */
void
PreludeWindow::destroy()
{
  TRACE_ENTER("PreludeWindow::destroy");
  delete this;
  TRACE_EXIT();
}


//! Stops the microbreak.
void
PreludeWindow::stop()
{
  TRACE_ENTER("PreludeWindow::stop");

  frame->set_frame_flashing(0);
#ifdef HAVE_GTK3
  hide();
#else
  hide_all();
#endif

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
  
#if defined(PLATFORM_OS_WIN32)
// Vista GTK phantom toplevel parent kludge:
  HWND hwnd = (HWND) GDK_WINDOW_HWND( Gtk::Widget::gobj()->window );
  if( hwnd )
    {
      HWND hAncestor = GetAncestor( hwnd, GA_ROOT );
      HWND hDesktop = GetDesktopWindow();
      if( hAncestor && hDesktop && hAncestor != hDesktop )
          hwnd = hAncestor;
      // Set toplevel window topmost!
      SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, 0, 0, 
          SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
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
  const char *icon = NULL;
  switch(stage)
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
      if (! did_avoid)
        {
          int winx, winy;
          get_position(winx, winy);
          set_position(Gtk::WIN_POS_NONE);
          move (winx, head.get_y() + SCREEN_MARGIN);
        }
      break;
    }
  if (icon != NULL)
    {
      string file = Util::complete_directory(icon, Util::SEARCH_PATH_IMAGES);
      image_icon->set(file);
    }
}


void
PreludeWindow::on_frame_flash(bool frame_visible)
{
  TRACE_ENTER("PreludeWindow::on_frame_flash");
  flash_visible = frame_visible;
  refresh();
  TRACE_EXIT();
}


// Sets whether the window should run away for the mouse pointer.
void
PreludeWindow::init_avoid_pointer()
{
  TRACE_ENTER("PreludeWindow::init_avoid_pointer");
#ifdef PLATFORM_OS_WIN32
  if (! avoid_signal.connected())
    {
      POINT p;
      GetCursorPos(&p);
      
      Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
      int x, y;
      Gdk::ModifierType mod;
      display->get_pointer(x, y, mod);

      TRACE_MSG("p " << p.x << " " << p.y);
      TRACE_MSG("d " << x << " " << y);

      gdk_offset_x = p.x - x;
      gdk_offset_y = p.y - y;

      TRACE_MSG("offset " << gdk_offset_x << " " << gdk_offset_y); 
     
      avoid_signal = Glib::signal_timeout()
        .connect(sigc::mem_fun(*this, &PreludeWindow::on_avoid_pointer_timer),
                 150);
    }
#else
  if (
#ifdef HAVE_GTK3
      ! get_realized()
#else
      ! is_realized()
#endif
      )
    {
      Gdk::EventMask events;

      events = Gdk::ENTER_NOTIFY_MASK;
      add_events(events);
    }
#endif
  did_avoid = false;
  TRACE_EXIT();
}

#ifndef PLATFORM_OS_WIN32

//! GDK EventNotifyEvent notification.
bool
PreludeWindow::on_enter_notify_event(GdkEventCrossing *event)
{
  avoid_pointer((int)event->x, (int)event->y);
  return Gtk::Window::on_enter_notify_event(event);
}

#endif


//! Move window if pointer is neat specified location.
void
PreludeWindow::avoid_pointer(int px, int py)
{
  TRACE_ENTER_MSG("PreludeWindow::avoid_pointer", px << " " << py);
  Glib::RefPtr<Gdk::Window> window = get_window();

#ifdef HAVE_GTK3
  int winx, winy, width, height;
  window->get_geometry(winx, winy, width, height);
#else
  int winx, winy, width, height, wind;
  window->get_geometry(winx, winy, width, height, wind);
#endif
  
  TRACE_MSG("geom" << winx << " " << winy << " " << width << " " << height << " ");

#ifdef PLATFORM_OS_WIN32
  // This is only necessary for PLATFORM_OS_WIN32, since PLATFORM_OS_UNIX uses GdkEventCrossing.
  // Set gravitiy, otherwise, get_position() returns weird winy.
  set_gravity(Gdk::GRAVITY_STATIC);
  get_position(winx, winy);
  TRACE_MSG("pos " << winx << " " << winy);
  if (px < winx || px > winx+width || py < winy || py > winy+height)
    return;
#else
  px += winx;
  py += winy;
#endif

  TRACE_MSG("geom2" << winx << " " << winy << " " << width << " " << height << " ");

  int screen_height = head.get_height();
  int top_y = head.get_y() + SCREEN_MARGIN;
  int bottom_y = head.get_y() + screen_height - height - SCREEN_MARGIN;
  TRACE_MSG("geom3" << screen_height << " " << top_y << " " << bottom_y);
  
  if (winy < top_y + SCREEN_MARGIN)
    {
      winy = bottom_y;
    }
  else if (winy > bottom_y - SCREEN_MARGIN)
    {
      winy = top_y;
    }
  else
    {
      if (py > winy + height/2)
        {
          winy = top_y;
        }
      else
        {
          winy = bottom_y;
        }
    }

  set_position(Gtk::WIN_POS_NONE);
  move(winx, winy);
  did_avoid = true;
}

#ifdef PLATFORM_OS_WIN32
bool
PreludeWindow::on_avoid_pointer_timer()
{
  TRACE_ENTER("PreludeWindow::on_avoid_pointer_timer");
  /*
    display->get_pointer reads low-level keyboard state, and that's a
    problem for anti-hook monitors. use GetCursorPos() instead.
  */
  POINT p;
  if (GetCursorPos(&p))
    {
     avoid_pointer(p.x - gdk_offset_x, p.y - gdk_offset_y);
    }

  TRACE_EXIT();
  return true;
}
#endif
