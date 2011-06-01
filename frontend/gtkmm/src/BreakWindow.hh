// BreakWindow.hh --- base class for the break windows
//
// Copyright (C) 2001 - 2011 Rob Caelers & Raymond Penners
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

#ifndef BREAKWINDOW_HH
#define BREAKWINDOW_HH

#include <stdio.h>

#include "preinclude.h"

#include <gtkmm.h>

#include "ICore.hh"
#include "IBreakWindow.hh"
#include "HeadInfo.hh"
#include "WindowHints.hh"
#include "GUIConfig.hh"

#ifdef PLATFORM_OS_WIN32
class DesktopWindow;
#endif

namespace workrave
{
  class IBreakResponse;
}

namespace Gtk
{
  class Button;
  class HButtonBox;
}

class Frame;

class BreakWindow :
  public Gtk::Window,
  public IBreakWindow
{
public:
  enum BreakFlags
    {
      BREAK_FLAGS_NONE            = 0,
      BREAK_FLAGS_POSTPONABLE     = 1 << 0,
      BREAK_FLAGS_SKIPPABLE       = 1 << 1,
      BREAK_FLAGS_NO_EXERCISES    = 1 << 2,
      BREAK_FLAGS_NATURAL         = 1 << 3,
      BREAK_FLAGS_USER_INITIATED  = 1 << 4
    };
    
  BreakWindow(BreakId break_id,
              HeadInfo &head,
              BreakFlags break_flags,
              GUIConfig::BlockMode block_mode);
  virtual ~BreakWindow();

  void set_response(IBreakResponse *bri);

  virtual void init();
  virtual void start();
  virtual void stop();
  virtual void destroy();
  virtual void refresh();

  virtual void update_break_window();
  
  Glib::RefPtr<Gdk::Window> get_gdk_window();

protected:
  virtual Gtk::Widget *create_gui() = 0;
  void init_gui();

  void center();

  Gtk::HButtonBox *create_break_buttons(bool lockable, bool shutdownable);
  void resume_non_ignorable_break();
  void on_lock_button_clicked();
  void on_shutdown_button_clicked();
  void on_skip_button_clicked();
  bool on_delete_event(GdkEventAny *);
  void on_postpone_button_clicked();

  //! Information about the (multi)head.
  HeadInfo head;

  //! Insist
  GUIConfig::BlockMode block_mode;

  //! Ignorable
  BreakFlags break_flags;

  //! Flash frame
  Frame *frame;

protected:
  Gtk::Button *create_skip_button();
  Gtk::Button *create_postpone_button();
  Gtk::Button *create_lock_button();
  Gtk::Button *create_shutdown_button();

private:
  //! Send response to this interface.
  IBreakResponse *break_response;

  //! Break ID
  BreakId break_id;

  //! GUI
  Gtk::Widget *gui;

  //! Break windows visible?
  bool visible;
 
#ifdef PLATFORM_OS_WIN32
  DesktopWindow *desktop_window;

  long parent;
#endif

  bool accel_added;
  Gtk::Button *postpone_button;
  Gtk::Button *skip_button;
  Gtk::Button *lock_button;
  Gtk::Button *shutdown_button;
  Glib::RefPtr<Gtk::AccelGroup> accel_group;
};

inline BreakWindow::BreakFlags
operator|(BreakWindow::BreakFlags lhs, BreakWindow::BreakFlags rhs)
{
  return static_cast<BreakWindow::BreakFlags>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
}

inline BreakWindow::BreakFlags
operator&(BreakWindow::BreakFlags lhs, BreakWindow::BreakFlags rhs)
{
  return static_cast<BreakWindow::BreakFlags>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs));
}

inline BreakWindow::BreakFlags
operator^(BreakWindow::BreakFlags lhs, BreakWindow::BreakFlags rhs)
{
  return static_cast<BreakWindow::BreakFlags>(static_cast<unsigned>(lhs) ^ static_cast<unsigned>(rhs));
}

inline BreakWindow::BreakFlags
operator~(BreakWindow::BreakFlags flags)

{
  return static_cast<BreakWindow::BreakFlags>(~static_cast<unsigned>(flags));
}

inline BreakWindow::BreakFlags&
operator|=(BreakWindow::BreakFlags& lhs, BreakWindow::BreakFlags rhs)

{
  return (lhs = static_cast<BreakWindow::BreakFlags>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs)));
}

inline BreakWindow::BreakFlags&
operator&=(BreakWindow::BreakFlags& lhs, BreakWindow::BreakFlags rhs)

{
  return (lhs = static_cast<BreakWindow::BreakFlags>(static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs)));
}

inline BreakWindow::BreakFlags&
operator^=(BreakWindow::BreakFlags& lhs, BreakWindow::BreakFlags rhs)

{
  return (lhs = static_cast<BreakWindow::BreakFlags>(static_cast<unsigned>(lhs) ^ static_cast<unsigned>(rhs)));
}

#endif // BREAKWINDOW_HH
