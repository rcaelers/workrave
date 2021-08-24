// Copyright (C) 2001 - 2020 Rob Caelers & Raymond Penners
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

#ifndef GUI_HH
#define GUI_HH

#include <sigc++/trackable.h>
#include <glibmm.h>

#if defined(PLATFORM_OS_WINDOWS)
#  include "eggsmclient.h"
#endif

#include "core/ICore.hh"
#include "core/IApp.hh"
#include "dbus/IDBusWatch.hh"
#include "utils/Signals.hh"
#include "commonui/UiTypes.hh"

#include "HeadInfo.hh"
#include "Menus.hh"
#include "IBreakWindow.hh"
#include "IPreludeWindow.hh"

#include "IApplication.hh"
#include "IToolkit.hh"
#include "WindowHints.hh"

#ifdef HAVE_INDICATOR
#  include "IndicatorAppletMenu.hh"
#endif

namespace workrave
{
  class IBreakResponse;
} // namespace workrave

// GTKMM classes
class MainWindow;
class StatusIcon;
class AppletControl;
class BreakControl;
class Session;

class GUI
  : public IApplication
  , public workrave::IApp
  , public workrave::ICoreEventListener
  , public workrave::dbus::IDBusWatch
  , public workrave::utils::Trackable
  , public sigc::trackable
{
public:
  GUI(int argc, char **argv, std::shared_ptr<IToolkit> toolkit);
  ~GUI() override;

  static IApplication *get_instance();

  AppletControl *get_applet_control() const;
  MainWindow *get_main_window() const override;
  SoundTheme::Ptr get_sound_theme() const override;

  void main();

  // GUIFactoryInterface methods
  void set_break_response(workrave::IBreakResponse *rep) override;
  void create_prelude_window(workrave::BreakId break_id) override;
  void create_break_window(workrave::BreakId break_id, workrave::utils::Flags<workrave::BreakHint> break_hint) override;
  void hide_break_window() override;
  void show_break_window() override;
  void refresh_break_window() override;
  void set_break_progress(int value, int max_value) override;
  void set_prelude_stage(PreludeStage stage) override;
  void set_prelude_progress_text(PreludeProgressText text) override;
  void terminate() override;

  //
  void core_event_notify(const workrave::CoreEvent event) override;
  void core_event_operation_mode_changed(const workrave::OperationMode m) override;
  void core_event_usage_mode_changed(const workrave::UsageMode m) override;

  void bus_name_presence(const std::string &name, bool present) override;

  // Internal public methods
  void restbreak_now() override;
  void open_main_window() override;
  void close_main_window();
  void init_multihead();

  // Misc
  HeadInfo get_head_info(int screen_index) const override
  {
    return toolkit->get_head_info(screen_index);
  };
  int get_head_count() const override
  {
    return toolkit->get_head_count();
  };

  sigc::signal0<void> &signal_heartbeat() override;
  int map_to_head(int &x, int &y) override;
  void map_from_head(int &x, int &y, int head) override;
  bool bound_head(int &x, int &y, int width, int height, int &head) override;
  void interrupt_grab() override;

private:
  std::string get_timers_tooltip();
  bool on_timer();
  void init_platform();
  void init_debug();
  void init_nls();
  void init_core();
  void init_sound_player();
  void update_multihead();
  void init_gui();
  void init_dbus();
  void init_session();
  void init_operation_mode_warning();

#if defined(PLATFORM_OS_WINDOWS)
  static void session_quit_cb(EggSMClient *client, GUI *gui);
  static void session_save_state_cb(EggSMClient *client, GKeyFile *key_file, GUI *gui);
  void cleanup_session();
#endif
  void collect_garbage();

  void grab();
  void ungrab();
  void process_visibility();

  void on_status_icon_balloon_activate(const std::string &id);
  void on_status_icon_activate();
  void on_visibility_changed();
  void on_main_window_closed();

  bool on_operational_mode_warning_timer();

#if defined(PLATFORM_OS_WINDOWS)
  void win32_init_filter();
  static GdkFilterReturn win32_filter_func(void *xevent, GdkEvent *event, gpointer data);
#endif

private:
  //! The one and only instance
  static GUI *instance;

  std::shared_ptr<IToolkit> toolkit;

  // !
  Glib::RefPtr<Gtk::Application> app;

  //! The Core controller
  workrave::ICore::Ptr core;

  //! The sound player
  SoundTheme::Ptr sound_theme;

  std::shared_ptr<Menus> menus;
  std::shared_ptr<MenuModel> menu_model;
  // std::shared_ptr<workrave::updater::Updater> updater;

#ifdef HAVE_INDICATOR
  std::shared_ptr<IndicatorAppletMenu> indicator_menu;
#endif

  std::vector<IBreakWindow::Ptr> break_windows;
  std::vector<IPreludeWindow::Ptr> prelude_windows;

  //! Response interface for breaks
  workrave::IBreakResponse *response{nullptr};

  //! Current active break.
  workrave::BreakId active_break_id{workrave::BREAK_ID_NONE};

  //! The number of command line arguments.
  int argc{0};

  //! The command line arguments.
  char **argv{nullptr};

  //! The main window, shows the timers.
  MainWindow *main_window{nullptr};

  //! Heartbeat signal
  sigc::signal0<void> heartbeat_signal;

  //! Status icon
  StatusIcon *status_icon{nullptr};

  //! The applet controller
  AppletControl *applet_control{nullptr};

  //!
  Session *session{nullptr};

  //
  bool muted{false};

  //
  bool closewarn_shown{false};

  // UI Event connections
  std::list<sigc::connection> event_connections;
};

//! Returns the only instance of GUI
inline IApplication *
GUI::get_instance()
{
  return instance;
}

//! Returns the applet window.
inline AppletControl *
GUI::get_applet_control() const
{
  return applet_control;
}

//! Returns the main window.
inline MainWindow *
GUI::get_main_window() const
{
  return main_window;
}

//! Returns the sound player
inline SoundTheme::Ptr
GUI::get_sound_theme() const
{
  return sound_theme;
}

//! Returns the GUI Heartbeat signal.
inline sigc::signal0<void> &
GUI::signal_heartbeat()
{
  return heartbeat_signal;
}
#endif // GUI_HH
