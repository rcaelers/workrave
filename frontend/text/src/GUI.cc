
// TextGUI.cc --- The WorkRave GUI
//
// Copyright (C) 2001 - 2009 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"

#include "nls.h"
#include "debug.hh"

#include <sstream>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>

#include "GUI.hh"
#include "PreludeWindow.hh"
#include "BreakWindow.hh"
#include "IBreakWindow.hh"
#include "MainWindow.hh"

#include "CoreFactory.hh"
#include "ICore.hh"
#include "IConfigurator.hh"

#include "System.hh"
#include "IBreakResponse.hh"
#include "SoundPlayer.hh"

#include "Util.hh"

#ifdef PLATFORM_OS_WIN32
#include "crashlog.h"
#endif

#include <glib-object.h>

GUI *GUI::instance = NULL;

const string GUI::CFG_KEY_GUI_BLOCK_MODE =  "gui/breaks/block_mode";


//! GUI Constructor.
/*!
 *  \param argc number of command line parameters.
 *  \param argv all command line parameters.
 */
GUI::GUI(int argc, char **argv)  :
  configurator(NULL),
  core(NULL),
  sound_player(NULL),
  break_window(NULL),
  prelude_window(NULL),
  main_window(NULL),
  response(NULL),
  break_window_destroy(false),
  prelude_window_destroy(false),
  active_break_id(BREAK_ID_NONE)
{
  TRACE_ENTER("GUI:GUI");

  assert(! instance);
  instance = this;

  this->argc = argc;
  this->argv = argv;

  TRACE_EXIT();
}


//! Destructor.
GUI::~GUI()
{
  TRACE_ENTER("GUI:~GUI");

  assert(instance);
  instance = NULL;

  delete main_window;

  TRACE_EXIT();
}


//! Forces a restbreak.
void
GUI::restbreak_now()
{
  core->force_break(BREAK_ID_REST_BREAK, true);
}


gboolean
GUI::static_on_timer(gpointer data)
{
  GUI *gui = (GUI*) data;
  gui->on_timer();
  return true;
}


//! The main entry point.
void
GUI::main()
{
  TRACE_ENTER("GUI::main");

#ifdef PLATFORM_OS_WIN32
  // Enable Windows structural exception handling.
  __try1(exception_handler);
#endif

#ifdef PLATFORM_OS_WIN32
  System::init();
#else
  System::init(NULL);
#endif

  g_type_init();

  init_debug();
  init_core();
  init_sound_player();

  // The main status window.
  main_window = new MainWindow();

  main_loop = g_main_loop_new(NULL, FALSE);

  const char *env = getenv("WORKRAVE_TEST");
  if (env == NULL)
    {
      g_timeout_add(1000, static_on_timer, this);
    }

  g_main_loop_run(main_loop);
  g_main_loop_unref(main_loop);

  delete main_window;
  main_window = NULL;

#ifdef PLATFORM_OS_WIN32
  // Disable Windows structural exception handling.
  __except1;
#endif

  TRACE_EXIT();
}


//! Terminates the GUI.
void
GUI::terminate()
{
  TRACE_ENTER("GUI::terminate");

  CoreFactory::get_configurator()->save();

  collect_garbage();

  g_main_loop_quit(main_loop);

  TRACE_EXIT();
}


//! Periodic heartbeat.
bool
GUI::on_timer()
{
  if (core != NULL)
    {
      core->heartbeat();
    }

  if (main_window != NULL)
    {
      main_window->update();
    }

  collect_garbage();

  return true;
}

#ifdef NDEBUG
static void my_log_handler(const gchar *log_domain, GLogLevelFlags log_level,
                           const gchar *message, gpointer user_data)
{
}
#endif


//! Initializes the GUI
void
GUI::init_gui()
{
  // The main status window.
  main_window = new MainWindow();
}

//! Initializes messages hooks.
void
GUI::init_debug()
{
#ifdef NDEBUG
  char *domains[] = { NULL, "Gtk", "GLib", "Gdk", "gtkmm", "GLib-GObject" };
  for (int i = 0; i < sizeof(domains)/sizeof(char *); i++)
    {
      g_log_set_handler(domains[i],
                        (GLogLevelFlags) (G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION),
                        my_log_handler, NULL);
    }

#endif
}


//! Initializes i18n.
void
GUI::init_nls()
{
#ifdef ENABLE_NLS
  const char *locale_dir;
#ifdef PLATFORM_OS_WIN32
  string dir = Util::get_application_directory() + "\\lib\\locale";
  locale_dir = dir.c_str();
#else
  locale_dir = "FIXME:";
#endif
  bindtextdomain(PACKAGE, locale_dir);
  bind_textdomain_codeset(PACKAGE, "UTF-8");
  textdomain(PACKAGE);
#endif
}


//! Initializes the core.
void
GUI::init_core()
{
  string display_name = "";

  core = CoreFactory::get_core();
  core->init(argc, argv, this, display_name);
  core->set_core_events_listener(this);
}


//! Initializes the sound player.
void
GUI::init_sound_player()
{
  sound_player = new SoundPlayer();
}


void
GUI::core_event_notify(CoreEvent event)
{
  TRACE_ENTER_MSG("GUI::core_event_notify", event)
  // FIXME: HACK
  SoundPlayer::SoundEvent snd = (SoundPlayer::SoundEvent) event;
  if (sound_player != NULL)
    {
      TRACE_MSG("play");
      sound_player->play_sound(snd);
    }
  TRACE_EXIT();
}


void
GUI::core_event_operation_mode_changed(const OperationMode m)
{
  (void) m;
}

//! Returns a break window for the specified break.
IBreakWindow *
GUI::new_break_window(BreakId break_id, bool user_initiated)
{
  IBreakWindow *ret = NULL;
  BlockMode block_mode = get_block_mode();
  bool ignorable = true;

  (void) user_initiated;
  
  if (break_id == BREAK_ID_MICRO_BREAK)
    {
      ret = new BreakWindow(break_id, ignorable, block_mode);
    }
  else if (break_id == BREAK_ID_REST_BREAK)
    {
      ret = new BreakWindow(break_id, ignorable, block_mode);
    }
  else if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      ret = new BreakWindow(break_id, ignorable, block_mode);
    }

  return ret;
}


void
GUI::set_break_response(IBreakResponse *rep)
{
  response = rep;
}


void
GUI::create_prelude_window(BreakId break_id)
{
  hide_break_window();
  collect_garbage();

  active_break_id = break_id;

  prelude_window = new PreludeWindow(break_id);
  prelude_window->set_response(response);
}


void
GUI::create_break_window(BreakId break_id, bool ignorable)
{
  TRACE_ENTER("GUI::start_break_window");

  hide_break_window();
  collect_garbage();

  active_break_id = break_id;

  break_window = new_break_window(break_id, ignorable);
  break_window->set_response(response);

  if (get_block_mode() != GUI::BLOCK_MODE_NONE)
    {
      /// XXX: grab keyboard and mouse
    }

  TRACE_EXIT();
}

void
GUI::hide_break_window()
{
  TRACE_ENTER("GUI::hide_break_window");
  active_break_id = BREAK_ID_NONE;

  if (prelude_window != NULL)
    {
      prelude_window->stop();
      prelude_window_destroy = true;
    }

  if (break_window != NULL)
    {
      break_window->stop();
      break_window_destroy = true;
    }

  // XXX: release the mouse/keyboard grab

  TRACE_EXIT();
}

void
GUI::show_break_window()
{
  TRACE_ENTER("GUI::hide_break_window");
  active_break_id = BREAK_ID_NONE;

  if (prelude_window != NULL)
    {
      prelude_window->start();
    }

  if (break_window != NULL)
    {
      break_window->start();
    }

  TRACE_EXIT();
}


void
GUI::refresh_break_window()
{
  if (prelude_window != NULL)
    {
      prelude_window->refresh();
    }

  if (break_window != NULL)
    {
      break_window->refresh();
    }
}


void
GUI::set_break_progress(int value, int max_value)
{
  if (prelude_window != NULL)
    {
      prelude_window->set_progress(value, max_value);
    }

  if (break_window != NULL)
    {
      break_window->set_progress(value, max_value);
    }
}


void
GUI::set_prelude_stage(PreludeStage stage)
{
  if (prelude_window != NULL)
    {
      prelude_window->set_stage(stage);
    }
}


void
GUI::set_prelude_progress_text(PreludeProgressText text)
{
  if (prelude_window != NULL)
    {
      prelude_window->set_progress_text(text);
    }
}

//! Destroys the break/prelude windows, if requested.
void
GUI::collect_garbage()
{
  TRACE_ENTER("GUI::collect_garbage");
  if (prelude_window_destroy)
    {
      if (prelude_window != NULL)
        {
          prelude_window->destroy();
          prelude_window = NULL;
        }
      prelude_window_destroy = false;
    }

  if (break_window_destroy)
    {
      if (break_window != NULL)
        {
          break_window->destroy();
          break_window = NULL;
        }
      break_window_destroy = false;
    }
  TRACE_EXIT();
}


GUI::BlockMode
GUI::get_block_mode()
{
  bool b;
  int mode;
  b = CoreFactory::get_configurator()
    ->get_value(CFG_KEY_GUI_BLOCK_MODE, mode);
  if (! b)
    {
      mode = BLOCK_MODE_INPUT;
    }
  return (BlockMode) mode;
}


void
GUI::set_block_mode(BlockMode mode)
{
  CoreFactory::get_configurator()
    ->set_value(CFG_KEY_GUI_BLOCK_MODE, int(mode));
}


