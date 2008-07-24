// GUI.hh --- The WorkRave GUI
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2007, 2008 Rob Caelers & Raymond Penners
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
#include <glib.h>

#include "ICoreEventListener.hh"
#include "ILinkEventListener.hh"
#include "IApp.hh"

// Generic GUI
class BreakControl;
class ISoundPlayer;
class IBreakWindow;
class PreludeWindow;
class MainWindow;

using namespace workrave;

namespace workrave
{
  // Core interfaces
  class IConfigurator;
  class LinkEvent;
}

class GUI :
  public IApp,
  public ICoreEventListener,
  public ILinkEventListener
{
public:
  GUI(int argc, char **argv);
  virtual ~GUI();

  static GUI *get_instance();

  void main();
  void restbreak_now();
  void terminate();

  // IGUIFactory methods
  virtual void set_break_response(IBreakResponse *rep);
  virtual void create_prelude_window(BreakId break_id);
  virtual void create_break_window(BreakId break_id, bool ignorable);
  virtual void hide_break_window();
  virtual void show_break_window();
  virtual void refresh_break_window();
  virtual void set_break_progress(int value, int max_value);
  virtual void set_prelude_stage(PreludeStage stage);
  virtual void set_prelude_progress_text(PreludeProgressText text);

  //
  void core_event_notify(CoreEvent event);
  void core_event_operation_mode_changed(const OperationMode m);

  //
  virtual void event_received(LinkEvent *event);

  ISoundPlayer *get_sound_player() const;

  static gboolean static_on_timer(gpointer data);

  enum BlockMode { BLOCK_MODE_NONE = 0, BLOCK_MODE_INPUT, BLOCK_MODE_ALL };

private:
  bool on_timer();
  void init_gui();
  void init_debug();
  void init_nls();
  void init_core();
  void init_sound_player();

  void collect_garbage();
  IBreakWindow *new_break_window(BreakId break_id, bool ignorable);

  // Prefs
  static const std::string CFG_KEY_GUI_BLOCK_MODE;
  BlockMode get_block_mode();
  void set_block_mode(BlockMode mode);

private:
  GMainLoop *main_loop;

  //! The one and only instance
  static GUI *instance;

  //! The Configurator.
  IConfigurator *configurator;

  //! The Core controller
  ICore *core;

  //! The sound player
  ISoundPlayer *sound_player;

  //! Interface to the break window.
  IBreakWindow *break_window;

  //! Interface to the prelude windows.
  PreludeWindow *prelude_window;

  //!
  MainWindow *main_window;

  //! Reponse interface for breaks
  IBreakResponse *response;

  //! Destroy break window on next heartbeat?
  bool break_window_destroy;

  //! Destroy prelude window on next heartbeat?
  bool prelude_window_destroy;

  //! Current active break.
  BreakId active_break_id;

  //! The number of command line arguments.
  int argc;

  //! The command line arguments.
  char **argv;

  //! Final prelude
  std::string progress_text;

  //! Progress values
  int progress_value;
  int progress_max_value;
};


//! Returns the only instance of GUI
inline GUI *
GUI::get_instance()
{
  return instance;
}

//! Returns the sound player
inline ISoundPlayer *
GUI::get_sound_player() const
{
  return sound_player;
}


#endif // GUI_HH
