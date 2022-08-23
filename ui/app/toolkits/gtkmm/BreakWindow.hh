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

#ifndef BREAKWINDOW_HH
#define BREAKWINDOW_HH

#include <cstdio>

#include <gtkmm.h>

#include "core/ICore.hh"
#include "ui/IBreakWindow.hh"
#include "HeadInfo.hh"
#include "GtkUtil.hh"
#include "ui/GUIConfig.hh"
#include "ui/UiTypes.hh"
#include "session/System.hh"
#include "ui/IToolkit.hh"

#if defined(PLATFORM_OS_WINDOWS)
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
} // namespace Gtk

class Frame;

class BreakWindow
  : public Gtk::Window
  , public IBreakWindow
{
public:
  BreakWindow(std::shared_ptr<IApplicationContext> app,
              workrave::BreakId break_id,
              HeadInfo &head,
              BreakFlags break_flags,
              BlockMode block_mode);
  ~BreakWindow() override;

  void init() override;
  void start() override;
  void stop() override;
  void refresh() override;

  virtual void update_break_window();

protected:
  virtual Gtk::Widget *create_gui() = 0;
  void init_gui();

  void center();

  GtkCompat::Box *create_bottom_box(bool lockable, bool shutdownable);
  void update_skip_postpone_lock();
  void check_skip_postpone_lock(bool &skip_locked, bool &postpone_locked, workrave::BreakId &break_id);
  void on_shutdown_button_clicked();
  void on_skip_button_clicked();
#if GTK_CHECK_VERSION(4, 0, 0)
  // TODO: use close request
#else
  bool on_delete_event(GdkEventAny *) override;
#endif
  void on_postpone_button_clicked();
  void on_lock_button_clicked();

#if defined(PLATFORM_OS_WINDOWS)
  void refresh_break_window();
#endif

  Gtk::Button *create_skip_button();
  Gtk::Button *create_postpone_button();
  Gtk::Button *create_lock_button();
  Gtk::ComboBox *create_sysoper_combobox();

  std::shared_ptr<IApplicationContext> app;

  //! Information about the (multi)head.
  HeadInfo head;

  //! Insist
  BlockMode block_mode;

  //! Ignorable
  BreakFlags break_flags;

  //! Flash frame
  Frame *frame{nullptr};

  //! Use fullscreen window to perform blocking
  bool fullscreen_grab;

private:
  class SysoperModelColumns : public Gtk::TreeModelColumnRecord
  {
  public:
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> icon;
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<System::SystemOperation::SystemOperationType> id;
    bool has_button_images;

    SysoperModelColumns(bool has_button_images)
      : has_button_images(has_button_images)
    {
      if (has_button_images)
        {
          add(icon);
        }
      add(name);
      add(id);
    }
  };

  //! Break ID
  workrave::BreakId break_id;

  //! Application
  Gtk::Widget *gui{nullptr};

  // Supported system operations (like suspend, hibernate, shutdown)
  std::vector<System::SystemOperation> supported_system_operations;
  SysoperModelColumns *sysoper_model_columns{nullptr};

  bool accel_added{false};
  Glib::RefPtr<Gtk::AccelGroup> accel_group;
  Gtk::Button *lock_button{nullptr};
  Gtk::Button *postpone_button{nullptr};
  Gtk::Button *skip_button{nullptr};
  Gtk::ComboBox *sysoper_combobox{nullptr};
  Gtk::ProgressBar *progress_bar{nullptr};
  Glib::RefPtr<Gtk::SizeGroup> box_size_group;
  Glib::RefPtr<Gtk::SizeGroup> button_size_group;

#if defined(PLATFORM_OS_WINDOWS)
  DesktopWindow *desktop_window{nullptr};
  bool force_focus_on_break_start{false};
  long parent{0};
#endif

  void get_operation_name_and_icon(System::SystemOperation::SystemOperationType type, const char **name, const char **icon_name);
  void append_row_to_sysoper_model(Glib::RefPtr<Gtk::ListStore> &model, System::SystemOperation::SystemOperationType type);
  void on_sysoper_combobox_changed();

  bool on_draw(const ::Cairo::RefPtr<::Cairo::Context> &cr) override;
  void on_screen_changed(const Glib::RefPtr<Gdk::Screen> &previous_screen) override;
};

#endif // BREAKWINDOW_HH
