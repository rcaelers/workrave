// GUI.hh --- The WorkRave GUI
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef GUI_HH
#define GUI_HH

#include "preinclude.h"

#include <sigc++/object.h>
#include <glibmm.h>

#include <gdk/gdkevents.h>
#include <gtkmm/tooltips.h>
#include <gdkmm/types.h>

#include "HeadInfo.hh"
#include "Mutex.hh"
#include "CoreEventListener.hh"
#include "ActivityMonitorListener.hh"
#include "AppInterface.hh"

#ifdef HAVE_GNOME
#include <libgnomeuimm.h>
#endif
#ifdef HAVE_GTK_MULTIHEAD
#include <gdkmm/display.h>
#include <gdkmm/screen.h>
#endif

#include "WindowHints.hh"

#ifdef WIN32
#include <windows.h>
#include <winuser.h>
typedef BOOL (CALLBACK *LUMONITORENUMPROC)(HMONITOR,HDC,LPRECT,LPARAM);
typedef BOOL (WINAPI *LUENUMDISPLAYMONITORS)(HDC,LPCRECT,LUMONITORENUMPROC,LPARAM);
#endif

// GTKMM classes
class MainWindow;
class MicroPauseWindow;
class RestBreakWindow;
class AppletWindow;
class PreludeWindow;
class BreakWindowInterface;
class BreakResponseInterface;
class Dispatcher;

// Generic GUI
class BreakControl;
class SoundPlayerInterface;

// Core interfaces
class ConfiguratorInterface;

class GUI :
  public AppInterface,
  public CoreEventListener,
  public ActivityMonitorListener,
  public SigC::Object
{
public:
  GUI(int argc, char **argv);
  virtual ~GUI();
  
  static GUI *get_instance();

  void main();

  // GUIFactoryInterface methods
  virtual void set_break_response(BreakResponseInterface *rep);
  virtual void start_prelude_window(BreakId break_id);
  virtual void start_break_window(BreakId break_id, bool ignorable);
  virtual void hide_break_window();
  virtual void refresh_break_window();
  virtual void set_break_progress(int value, int max_value);
  virtual bool delayed_hide_break_window();
  virtual void set_prelude_stage(PreludeStage stage);
  virtual void set_prelude_progress_text(PreludeProgressText text);

  //
  void core_event_notify(CoreEvent event);
  
  // Internal public methods
  void restbreak_now();
  void open_main_window();
  void close_main_window();
  void toggle_main_window();
  void terminate();
  void init_multihead();

  // Prefs
  static const std::string CFG_KEY_GUI_BLOCK_MODE;
  enum BlockMode { BLOCK_MODE_NONE = 0, BLOCK_MODE_INPUT, BLOCK_MODE_ALL };
  BlockMode get_block_mode();
  void set_block_mode(BlockMode mode);
  
  // Misc
  SigC::Signal0<void> &signal_heartbeat();
  
#ifdef HAVE_X  
  AppletWindow *get_applet_window() const;
#endif  
  MainWindow *get_main_window() const;
  Gtk::Tooltips *get_tooltips() const;
  SoundPlayerInterface *get_sound_player() const;

private:
  bool on_timer();
  void init_debug();
  void init_nls();
  void init_core();
  void init_sound_player();
  void init_multihead_mem(int new_num_heads);
  void init_multihead_desktop();
  void init_gui();
  void init_remote_control();

  bool action_notify();
  void on_activity();

#if defined(HAVE_GTK_MULTIHEAD)
  void init_gtk_multihead();
#elif defined(WIN32)
  void init_win32_multihead();
  void update_win32_multihead();
public:
  BOOL CALLBACK enum_monitor_callback(LPRECT rc);
private:
#endif
  
#ifdef HAVE_GNOME
  void init_gnome();
  void on_die();
  bool on_save_yourself(int phase, Gnome::UI::SaveStyle save_style, bool shutdown,
                        Gnome::UI::InteractStyle interact_style, bool fast);
#endif
  void collect_garbage();
  BreakWindowInterface *create_break_window(HeadInfo &head, BreakId break_id, bool ignorable);

  void locate_main_window(GdkEventConfigure *event);
  void relocate_main_window(int width, int height);
  bool on_mainwindow_configure_event(GdkEventConfigure *event);

  bool grab();
  void ungrab();

#if defined(HAVE_X)
  bool on_grab_retry_timer();
#endif
  
private:
  //! The one and only instance
  static GUI *instance;

  //! The Configurator.
  ConfiguratorInterface *configurator;

  //! The Core controller
  CoreInterface *core;

  //! The sound player
  SoundPlayerInterface *sound_player;

  //! Interface to the break window.
  BreakWindowInterface **break_windows;

  //! Interface to the prelude windows.
  PreludeWindow **prelude_windows;

  //! Number of active prelude windows;
  int active_break_count;
  
  //! Number of active prelude windows;
  int active_prelude_count;
  
  //! Reponse interface for breaks
  BreakResponseInterface *response;

  //! Current active break.
  BreakId active_break_id;
  
  //! The number of command line arguments.
  int argc;

  //! The command line arguments.
  char **argv;

#ifdef HAVE_X  
  //! The applet window.
  AppletWindow *applet_window;
#endif  

  //! The main window, shows the timers.
  MainWindow *main_window;

  //! Tooptip manager.
  Gtk::Tooltips *tooltips;

  //! Heartbeat signal
  SigC::Signal0<void> heartbeat_signal;

  //! Destroy break window on next heartbeat?
  bool break_window_destroy;

  //! Destroy prelude window on next heartbeat?
  bool prelude_window_destroy;

  SigC::Connection dispatch_connection;
  Dispatcher *dispatcher;

  //! Information on all heads.
  HeadInfo *heads;

  //! Number of heads
  int num_heads;

  //! Width of the screen.
  int screen_width;
  
  //! Height of the screen.
  int screen_height;

  //! Location of main window.
  Gdk::Point main_window_location;
  
  //! Location of main window relative to current head
  Gdk::Point main_window_head_location;

  //! Relocated location of main window
  Gdk::Point main_window_relocated_location;
  
#ifdef WIN32
  LUENUMDISPLAYMONITORS enum_monitors;
  HINSTANCE user_lib;
  int current_monitor;
#endif

#ifdef HAVE_X
  //! Do we want a keyboard/pointer grab
  bool grab_wanted;
#endif

  //! Grab
  WindowHints::Grab *grab_handle;
};


//! Returns the only instance of GUI
inline GUI *
GUI::get_instance() 
{
  return instance;
}


//! Returns tooltips
inline Gtk::Tooltips *
GUI::get_tooltips() const
{
  return tooltips;
}


#ifdef HAVE_X
//! Returns the applet window.
inline AppletWindow *
GUI::get_applet_window() const
{
  return applet_window;
}
#endif


//! Returns the main window.
inline MainWindow *
GUI::get_main_window() const
{
  return main_window;
}


//! Returns the sound player
inline SoundPlayerInterface *
GUI::get_sound_player() const
{
  return sound_player;
}

//! Returns the GUI Heartbeat signal.
inline SigC::Signal0<void> &
GUI::signal_heartbeat()
{
  return heartbeat_signal;
}

#endif // GUI_HH
