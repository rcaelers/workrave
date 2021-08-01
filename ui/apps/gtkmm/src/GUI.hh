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

#include "HeadInfo.hh"
#include "core/ICore.hh"
#include "core/IApp.hh"
#include "dbus/IDBusWatch.hh"
#include "utils/Signals.hh"
#include "commonui/SoundTheme.hh"

#include "BreakWindow.hh"
#include "WindowHints.hh"

namespace workrave
{
  class IBreakResponse;
} // namespace workrave

// GTKMM classes
class MainWindow;
class MicroBreakWindow;
class RestBreakWindow;
class PreludeWindow;
class Dispatcher;
class StatusIcon;
class AppletControl;
class Menus;

// Generic GUI
class BreakControl;
class IBreakWindow;
class Session;

class IGUI
{
public:
  virtual ~IGUI() = default;

  virtual sigc::signal0<void> &signal_heartbeat() = 0;

  virtual Menus *get_menus() const = 0;
  virtual MainWindow *get_main_window() const = 0;
  virtual SoundTheme::Ptr get_sound_theme() const = 0;

  virtual void open_main_window() = 0;
  virtual void restbreak_now() = 0;

  virtual void interrupt_grab() = 0;

  virtual int get_number_of_heads() const = 0;
  virtual HeadInfo &get_head(int head) = 0;
  virtual int map_to_head(int &x, int &y) = 0;
  virtual void map_from_head(int &x, int &y, int head) = 0;
  virtual bool bound_head(int &x, int &y, int width, int height, int &head) = 0;
  virtual void terminate() = 0;
};

class GUI
  : public IGUI
  , public workrave::IApp
  , public workrave::ICoreEventListener
  , public workrave::dbus::IDBusWatch
  , public workrave::utils::Trackable
  , public sigc::trackable
{
public:
  GUI(int argc, char **argv);
  ~GUI() override;

  static IGUI *get_instance();

  AppletControl *get_applet_control() const;
  MainWindow *get_main_window() const override;
  SoundTheme::Ptr get_sound_theme() const override;
  Menus *get_menus() const override;

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

  // Prefs
  // Misc
  sigc::signal0<void> &signal_heartbeat() override;
  HeadInfo &get_head(int head) override;
  int get_number_of_heads() const override;
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
  void init_multihead_mem(int new_num_heads);
  void init_multihead_desktop();
  void init_gui();
  void init_dbus();
  void init_session();
  void init_operation_mode_warning();

  void init_gtk_multihead();

#if defined(PLATFORM_OS_WINDOWS)
  static void session_quit_cb(EggSMClient *client, GUI *gui);
  static void session_save_state_cb(EggSMClient *client, GKeyFile *key_file, GUI *gui);
  void cleanup_session();
#endif
  void collect_garbage();
  IBreakWindow *create_break_window(HeadInfo &head, workrave::BreakId break_id, BreakWindow::BreakFlags break_flags);

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

  // !
  Glib::RefPtr<Gtk::Application> app;

  //! The Core controller
  workrave::ICore::Ptr core;

  //! The sound player
  SoundTheme::Ptr sound_theme;

  //! Interface to the break window.
  IBreakWindow **break_windows{nullptr};

  //! Interface to the prelude windows.
  PreludeWindow **prelude_windows{nullptr};

  //! Number of active prelude windows;
  int active_break_count{0};

  //! Number of active prelude windows;
  int active_prelude_count{0};

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

  //! Menus
  Menus *menus{nullptr};

  //! Heartbeat signal
  sigc::signal0<void> heartbeat_signal;

  //! Destroy break window on next heartbeat?
  bool break_window_destroy{false};

  //! Destroy prelude window on next heartbeat?
  bool prelude_window_destroy{false};

  //! Information on all heads.
  HeadInfo *heads{nullptr};

  //! Number of heads
  int num_heads{-1};

  //! Width of the screen.
  int screen_width{-1};

  //! Height of the screen.
  int screen_height{-1};

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
inline IGUI *
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

//! Returns the sound player
inline Menus *
GUI::get_menus() const
{
  return menus;
}

//! Returns the GUI Heartbeat signal.
inline sigc::signal0<void> &
GUI::signal_heartbeat()
{
  return heartbeat_signal;
}

//! Number of heads.
inline int
GUI::get_number_of_heads() const
{
  return num_heads;
}

#endif // GUI_HH
