// GUI.hh --- The WorkRave GUI
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Rob Caelers & Raymond Penners
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
#include <glib.h>


#include "Mutex.hh"
#include "CoreEventListener.hh"
#include "ActivityMonitorListener.hh"
#include "AppInterface.hh"

// Generic GUI
class BreakControl;
class SoundPlayerInterface;

// Core interfaces
class ConfiguratorInterface;

class GUI :
  public AppInterface,
  public CoreEventListener
{
public:
  GUI(int argc, char **argv);
  virtual ~GUI();
  
  static GUI *get_instance();

  void main();
  void restbreak_now();
  void terminate();

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
  
  SoundPlayerInterface *get_sound_player() const;

  static gboolean static_on_timer(gpointer data);
  
private:
  bool on_timer();
  void init_debug();
  void init_nls();
  void init_core();
  void init_sound_player();
 
private:
  //! The one and only instance
  static GUI *instance;

  //! The Configurator.
  ConfiguratorInterface *configurator;

  //! The Core controller
  CoreInterface *core;

  //! The sound player
  SoundPlayerInterface *sound_player;

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
};


//! Returns the only instance of GUI
inline GUI *
GUI::get_instance() 
{
  return instance;
}

//! Returns the sound player
inline SoundPlayerInterface *
GUI::get_sound_player() const
{
  return sound_player;
}


#endif // GUI_HH
