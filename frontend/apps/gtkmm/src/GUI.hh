// GUI.hh --- The WorkRave GUI
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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

#include "preinclude.h"

#include <sigc++/trackable.h>
#include <glibmm.h>

#include "config/Config.hh"

#include "HeadInfo.hh"
#include "IBreak.hh"
#include "IApp.hh"
#include "BreakWindow.hh"
#include "WindowHints.hh"
#include "SoundTheme.hh"

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

using namespace workrave;
using namespace workrave::config;

class IGUI
{
public:
  virtual ~IGUI() {}

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

class GUI :
  public IGUI,
  public IApp,
  public sigc::trackable
{
public:
  GUI(int argc, char **argv);
  virtual ~GUI();

  static IGUI *get_instance();

  AppletControl *get_applet_control() const;
  MainWindow *get_main_window() const;
  SoundTheme::Ptr get_sound_theme() const;
  Menus *get_menus() const;

  void main();

  // GUIFactoryInterface methods
  virtual void create_prelude_window(BreakId break_id);
  virtual void create_break_window(BreakId break_id, BreakHint break_hint);
  virtual void hide_break_window();
  virtual void show_break_window();
  virtual void refresh_break_window();
  virtual void set_break_progress(int value, int max_value);
  virtual void set_prelude_stage(PreludeStage stage);
  virtual void set_prelude_progress_text(PreludeProgressText text);
  virtual void terminate();

  //

  // Internal public methods
  void restbreak_now();
  void open_main_window();
  void close_main_window();
  void init_multihead();

  // Prefs
  // Misc
  sigc::signal0<void> &signal_heartbeat();
  HeadInfo &get_head(int head);
  int get_number_of_heads() const;
  int map_to_head(int &x, int &y);
  void map_from_head(int &x, int &y, int head);
  bool bound_head(int &x, int &y, int width, int height, int &head);
  void interrupt_grab();

private:
  std::string get_timers_tooltip();
  bool on_timer();
  void init_platform();
  void init_debug();
  void init_nls();
  void init_core();
  void init_sound_player();
  void init_multihead_mem(int new_num_heads);
  void init_multihead_desktop();
  void init_gui();
  void init_dbus();
  void init_session();
  void init_startup_warnings();

  void init_gtk_multihead();

  void cleanup_session();

  void collect_garbage();
  IBreakWindow *create_break_window(HeadInfo &head, BreakId break_id, BreakWindow::BreakFlags break_flags);

  bool grab();
  void ungrab();
  void process_visibility();

  void on_status_icon_balloon_activate(const std::string &id);
  void on_status_icon_activate();
  void on_visibility_changed();
  void on_main_window_closed();

  void on_break_event(BreakId break_id, BreakEvent event);
  void on_operation_mode_changed(const OperationMode m);
  void on_usage_mode_changed(const UsageMode m);
  
#if defined(PLATFORM_OS_UNIX)
  bool on_grab_retry_timer();
#endif
  bool on_operational_mode_warning_timer();

#if defined(PLATFORM_OS_WIN32)
  void win32_init_filter();
  static GdkFilterReturn win32_filter_func (void     *xevent,
                                            GdkEvent *event,
                                            gpointer  data);
#endif

private:
  //! The one and only instance
  static GUI *instance;

  //! The Core controller
  ICore::Ptr core;

  //! The sound player
  SoundTheme::Ptr sound_theme;

  //! Interface to the break window.
  IBreakWindow **break_windows;

  //! Interface to the prelude windows.
  PreludeWindow **prelude_windows;

  //! Number of active prelude windows;
  int active_break_count;

  //! Number of active prelude windows;
  int active_prelude_count;

  //! Current active break.
  BreakId active_break_id;

  //! The number of command line arguments.
  int argc;

  //! The command line arguments.
  char **argv;

  //! The main window, shows the timers.
  MainWindow *main_window;

  //! Menus
  Menus *menus;

  //! Heartbeat signal
  sigc::signal0<void> heartbeat_signal;

  //! Destroy break window on next heartbeat?
  bool break_window_destroy;

  //! Destroy prelude window on next heartbeat?
  bool prelude_window_destroy;

  //! Information on all heads.
  HeadInfo *heads;

  //! Number of heads
  int num_heads;

  //! Width of the screen.
  int screen_width;

  //! Height of the screen.
  int screen_height;

#ifdef PLATFORM_OS_UNIX
  //! Do we want a keyboard/pointer grab
  bool grab_wanted;

  //! Connection to the grab retry timeout timer.
  sigc::connection grab_retry_connection;
#endif

  //! Grab
  WindowHints::Grab *grab_handle;

  //! Status icon
  StatusIcon *status_icon;

  //! The applet controller
  AppletControl *applet_control;

  //!
  Session *session;

  //
  bool muted;

  //
  bool closewarn_shown;

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
