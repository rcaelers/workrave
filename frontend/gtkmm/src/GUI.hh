// GUI.hh --- The WorkRave GUI
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Rob Caelers & Raymond Penners
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
#include "ICoreEventListener.hh"
#include "IApp.hh"

#ifdef HAVE_GNOMEMM
#include <libgnomeuimm.h>
#endif
#include <gdkmm/display.h>
#include <gdkmm/screen.h>

#include "WindowHints.hh"

namespace workrave {
  class IBreakResponse;
  class IConfigurator;
}

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
class ISoundPlayer;
class IBreakWindow;

using namespace workrave;

class GUI :
  public IApp,
  public ICoreEventListener,
  public SigC::Object
{
public:
  GUI(int argc, char **argv);
  virtual ~GUI();

  static GUI *get_instance();

  void main();

  // GUIFactoryInterface methods
  virtual void set_break_response(IBreakResponse *rep);
  virtual void start_prelude_window(BreakId break_id);
  virtual void start_break_window(BreakId break_id, bool ignorable);
  virtual void hide_break_window();
  virtual void refresh_break_window();
  virtual void set_break_progress(int value, int max_value);
  virtual void set_prelude_stage(PreludeStage stage);
  virtual void set_prelude_progress_text(PreludeProgressText text);
  virtual void terminate();

  //
  void core_event_notify(CoreEvent event);
  void core_event_operation_mode_changed(const OperationMode m);

  // Internal public methods
  void restbreak_now();
  void open_main_window();
  void close_main_window();
  void toggle_main_window();
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

  AppletControl *get_applet_control() const;
  MainWindow *get_main_window() const;
  Gtk::Tooltips *get_tooltips() const;
  ISoundPlayer *get_sound_player() const;

private:
  std::string get_timers_tooltip();
  bool on_timer();
  void init_debug();
  void init_nls();
  void init_core();
  void init_sound_player();
  void init_multihead_mem(int new_num_heads);
  void init_multihead_desktop();
  void init_gui();
  void init_remote_control();

  void init_gtk_multihead();

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
  IBreakWindow *create_break_window(HeadInfo &head, BreakId break_id, bool ignorable);

  bool grab();
  void ungrab();

#if defined(PLATFORM_OS_UNIX)
  bool on_grab_retry_timer();
#endif

#if defined(PLATFORM_OS_WIN32)
  void win32_init_filter();
  static GdkFilterReturn win32_filter_func (void     *xevent,
                                            GdkEvent *event,
                                            gpointer  data);
#endif

private:
  //! The one and only instance
  static GUI *instance;

  //! The Configurator.
  IConfigurator *configurator;

  //! The Core controller
  ICore *core;

  //! The sound player
  ISoundPlayer *sound_player;

  //! Interface to the break window.
  IBreakWindow **break_windows;

  //! Interface to the prelude windows.
  PreludeWindow **prelude_windows;

  //! Number of active prelude windows;
  int active_break_count;

  //! Number of active prelude windows;
  int active_prelude_count;

  //! Reponse interface for breaks
  IBreakResponse *response;

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

#ifdef PLATFORM_OS_WIN32
  int current_monitor;
#endif

#ifdef PLATFORM_OS_UNIX
  //! Do we want a keyboard/pointer grab
  bool grab_wanted;

  //! Connection to the grab retry timeout timer.
  SigC::Connection grab_retry_connection;
#endif

  //! Grab
  WindowHints::Grab *grab_handle;

  //! Status icon
  StatusIcon *status_icon;

  //! The applet controller
  AppletControl *applet_control;
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
inline ISoundPlayer *
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
