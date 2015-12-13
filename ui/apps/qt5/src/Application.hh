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

#ifndef APPLICATION_HH
#define APPLICATION_HH

#include <list>
#include <vector>
#include <memory>

#include "commonui/Session.hh"
#include "commonui/Session.hh"
#include "commonui/SoundTheme.hh"
#include "config/Config.hh"
#include "core/IApp.hh"
#include "updater/Updater.hh"
#include "utils/ScopedConnections.hh"

#include "IApplication.hh"
#include "IBreakWindow.hh"
#include "IPreludeWindow.hh"
#include "IToolkit.hh"
#include "Menus.hh"

class Application :
  public IApplication,
  public workrave::IApp
{
public:
  typedef std::shared_ptr<Application> Ptr;

  Application(int argc, char **argv, IToolkit::Ptr toolkit);
  ~Application() override;

  SoundTheme::Ptr get_sound_theme() const;
  void main();

  // IApp methods
  void create_prelude_window(workrave::BreakId break_id) override;
  void create_break_window(workrave::BreakId break_id, workrave::BreakHint break_hint) override;
  void hide_break_window() override;
  void show_break_window() override;
  void refresh_break_window() override;
  void set_break_progress(int value, int max_value) override;
  void set_prelude_stage(PreludeStage stage) override;
  void set_prelude_progress_text(PreludeProgressText text) override;
  //virtual void terminate();

  // IApplication
  void restbreak_now() override;
  void terminate() override;

  // Internal public methods
  //void open_main_window();
  //void close_main_window();
  //void init_multihead();

  // Prefs
  // Misc
  //sigc::signal0<void> &signal_heartbeat();
  //HeadInfo &get_head(int head);
  //int get_number_of_heads() const;
  //int map_to_head(int &x, int &y);
  //void map_from_head(int &x, int &y, int head);
  //bool bound_head(int &x, int &y, int width, int height, int &head);
  //void interrupt_grab();

private:
  //std::string get_timers_tooltip();
  bool on_timer();
  //void init_platform();
  //void init_debug();
  //void init_nls();
  void init_core();
  void init_sound_player();
  //void init_multihead_mem(int new_num_heads);
  //void init_multihead_desktop();
  //void init_gui();
  void init_bus();
  void init_session();
  void init_startup_warnings();
  void init_updater();

  //void init_qt_multihead();

  //void process_visibility();

  void on_status_icon_balloon_activate(const std::string &id);
  void on_status_icon_activate();
  //void on_visibility_changed();
  //void on_main_window_closed();

  void on_break_event(workrave::BreakId break_id, workrave::BreakEvent event);
  void on_operation_mode_warning_timer();

private:
  typedef std::vector<IBreakWindow::Ptr> BreakWindows;
  typedef BreakWindows::iterator BreakWindowsIter;

  typedef std::vector<IPreludeWindow::Ptr> PreludeWindows;
  typedef PreludeWindows::iterator PreludeWindowsIter;

  //!
  IToolkit::Ptr toolkit;

  //! The Core controller
  workrave::ICore::Ptr core;

  //!
  Menus::Ptr menus;

  //!
  workrave::updater::Updater::Ptr updater;

  //! The number of command line arguments.
  int argc;

  //! The command line arguments.
  char **argv;

  //! The sound player
  SoundTheme::Ptr sound_theme;

  //! Interface to the break window.
  BreakWindows break_windows;

  //! Interface to the prelude windows.
  //! Interface to the break window.
  PreludeWindows prelude_windows;

  //! Number of active prelude windows;
  //int active_prelude_count;

  //! Current active break.
  workrave::BreakId active_break_id;

  //! Destroy break window on next heartbeat?
  //bool break_window_destroy;

  //! Destroy prelude window on next heartbeat?
  //bool prelude_window_destroy;

  //! Width of the screen.
  //int screen_width;

  //! Height of the screen.
  //int screen_height;

#ifdef PLATFORM_OS_UNIX
  //! Do we want a keyboard/pointer grab
  //bool grab_wanted;

  //! Connection to the grab retry timeout timer.
  //sigc::connection grab_retry_connection;
#endif

  //! Grab
  //WindowHints::Grab *grab_handle;

  //! The applet controller
  //AppletControl *applet_control;

  //!
  Session::Ptr session;

  //
  bool muted;

  //
  //bool closewarn_shown;

  scoped_connections connections;
};


//! Returns the sound player
inline SoundTheme::Ptr
Application::get_sound_theme() const
{
  return sound_theme;
}
#endif // APPLICATION_HH
