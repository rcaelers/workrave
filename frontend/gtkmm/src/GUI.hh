// GUI.hh --- The WorkRave GUI
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Rob Caelers & Raymond Penners
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

#ifdef HAVE_GNOMEMM
#include <libgnomeuimm.h>
#endif
#ifdef HAVE_GTK_MULTIHEAD
#include <gdkmm/display.h>
#include <gdkmm/screen.h>
#endif

#include "WindowHints.hh"

// GTKMM classes
class MainWindow;
class MicroBreakWindow;
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
  HeadInfo &get_head(int head);
  int get_number_of_heads() const;
  int map_to_head(int &x, int &y);
  void map_from_head(int &x, int &y, int head);
  bool bound_head(int &x, int &y, int width, int height, int head);
  void interrupt_grab();
  
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

#if defined(HAVE_GTK_MULTIHEAD)
  void init_gtk_multihead();
#elif defined(WIN32)
  void init_win32_multihead();
  void update_win32_multihead();
public:
  BOOL CALLBACK enum_monitor_callback(HMONITOR mon, HDC hdc, LPRECT rc);
private:
#endif
  
#ifdef HAVE_GNOME
  void init_gnome();
#ifdef HAVE_GNOMEMM
  void on_die();
  bool on_save_yourself(int phase, Gnome::UI::SaveStyle save_style, bool shutdown,
                        Gnome::UI::InteractStyle interact_style, bool fast);
#endif
#endif
#ifdef HAVE_KDE
  void init_kde();
#endif
  void collect_garbage();
  BreakWindowInterface *create_break_window(HeadInfo &head, BreakId break_id, bool ignorable);

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

  //! Information on all heads.
  HeadInfo *heads;

  //! Number of heads
  int num_heads;

  //! Width of the screen.
  int screen_width;
  
  //! Height of the screen.
  int screen_height;

#ifdef WIN32
  int current_monitor;
#endif

#ifdef HAVE_X
  //! Do we want a keyboard/pointer grab
  bool grab_wanted;

  //! Connection to the grab retry timeout timer.
  SigC::Connection grab_retry_connection;
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

//! Number of heads.
inline int
GUI::get_number_of_heads() const
{
  return num_heads;
}


#endif // GUI_HH
