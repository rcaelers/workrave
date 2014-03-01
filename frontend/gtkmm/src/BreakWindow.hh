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

#ifdef HAVE_CONFIG
#include "config.h"
#endif

#include <stdio.h>

#include "preinclude.h"

#include <gtkmm.h>

#include "ICore.hh"
#include "IBreakWindow.hh"
#include "HeadInfo.hh"
#include "WindowHints.hh"
#include "GUIConfig.hh"
#include "System.hh"

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
  class Box;
}

class Frame;

class BreakWindow :
  public Gtk::Window,
  public IBreakWindow
{
    friend class W32Compat;

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

  Gtk::Box *create_bottom_box(bool lockable, bool shutdownable);
  void resume_non_ignorable_break();
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
  Gtk::ComboBox *create_sysoper_combobox(bool shutdownable);

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
  bool force_focus_on_break_start;
  long parent;
#endif

  bool accel_added;
  Glib::RefPtr<Gtk::AccelGroup> accel_group;
  Gtk::Button *postpone_button;
  Gtk::Button *skip_button;

  class SysoperModelColumns : public Gtk::TreeModelColumnRecord
    {
    public:
      Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
      Gtk::TreeModelColumn<Glib::ustring> name;
      Gtk::TreeModelColumn<System::SystemOperation::SystemOperationType> id;
      bool has_button_images;

      SysoperModelColumns(bool has_button_images): has_button_images(has_button_images)
        {
          if (has_button_images)
            {
              add(icon);
            }
          add(name);
          add(id);
        }
    };

  //Supported system operations (like suspend, hibernate, shutdown)
  std::vector<System::SystemOperation> supported_system_operations;
  SysoperModelColumns *sysoper_model_columns;

  Gtk::ComboBox *sysoper_combobox;
  void get_operation_name_and_icon(
      System::SystemOperation::SystemOperationType type, const char **name, const char **icon_name);
  void append_row_to_sysoper_model(Glib::RefPtr<Gtk::ListStore> &model,
      System::SystemOperation::SystemOperationType type);
  void on_sysoper_combobox_changed();
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
