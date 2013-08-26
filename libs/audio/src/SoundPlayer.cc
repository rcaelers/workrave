// SoundPlayer.cc --- Sound player
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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

#include "debug.hh"
#include "nls.h"

#ifdef HAVE_REALPATH
#include <limits.h>
#include <stdlib.h>
#endif

#include <list>
#include <set>

#include "Thread.hh"

#include "ISoundDriver.hh"
#include "IMixer.hh"

#include "config/IConfigurator.hh"
#include "utils/Platform.hh"

#include "Util.hh"

#if defined HAVE_GSTREAMER
#include "GstSoundPlayer.hh"
#elif defined PLATFORM_OS_UNIX
#include <X11/Xlib.h>
#elif defined PLATFORM_OS_WIN32
#include <windows.h>
#include "W32SoundPlayer.hh"
#include "W32DirectSoundPlayer.hh"
#include "W32Mixer.hh"
#elif defined PLATFORM_OS_OSX
#include "OSXSoundPlayer.hh"
#endif

#if defined HAVE_PULSE
#include "PulseMixer.hh"
#endif

#ifdef HAVE_GTK
#include <gdk/gdk.h>
#endif

const char *ISoundPlayer::CFG_KEY_SOUND_ENABLED = "sound/enabled";
const char *ISoundPlayer::CFG_KEY_SOUND_DEVICE = "sound/device";
const char *ISoundPlayer::CFG_KEY_SOUND_VOLUME = "sound/volume";
const char *ISoundPlayer::CFG_KEY_SOUND_MUTE = "sound/mute";
const char *ISoundPlayer::CFG_KEY_SOUND_EVENTS = "sound/events/";
const char *ISoundPlayer::CFG_KEY_SOUND_EVENTS_ENABLED = "_enabled";

using namespace workrave;
using namespace workrave::config;
using namespace workrave::utils;
using namespace std;

SoundPlayer::SoundRegistry SoundPlayer::sound_registry[] =
{
  { "WorkraveBreakPrelude",
    "break_prelude",
    "break-prelude.wav",
    _("Break prompt")
  },

  { "WorkraveBreakIgnored",
    "break_ignored",
    "break-ignored.wav",
    _("Break ignored")
  },

  { "WorkraveRestBreakStarted",
    "rest_break_started",
    "rest-break-started.wav",
    _("Rest break started")
  },

  {
    "WorkraveRestBreakEnded",
    "rest_break_ended",
    "rest-break-ended.wav",
    _("Rest break ended")
  },

  { "WorkraveMicroBreakStarted",
    "micro_break_started",
    "micro-break-started.wav",
    _("Micro-break started")
  },

  { "WorkraveMicroBreakEnded",
    "micro_break_ended",
    "micro-break-ended.wav",
    _("Micro-break ended")
  },

  { "WorkraveDailyLimit",
    "daily_limit",
    "daily-limit.wav",
    _("Daily limit")
  },

  { "WorkraveExerciseEnded",
    "exercise_ended",
    "exercise-ended.wav",
    _("Exercise ended")
  },

  { "WorkraveExercisesEnded",
    "exercises_ended",
    "exercises-ended.wav",
    _("Exercises ended")
  },

  { "WorkraveExerciseStep",
    "exercise_step",
    "exercise-step.wav",
    _("Exercise change")
  },
};


/**********************************************************************
 * SoundPlayer
 **********************************************************************/

ISoundPlayer::Ptr
ISoundPlayer::create(workrave::config::IConfigurator::Ptr config)
{
  return ISoundPlayer::Ptr(new SoundPlayer(config));
}

SoundPlayer::SoundPlayer(workrave::config::IConfigurator::Ptr config)
  : config(config)
{
  driver =
#if defined HAVE_GSTREAMER
     new GstSoundPlayer()
#elif defined PLATFORM_OS_WIN32
     new W32DirectSoundPlayer()
#elif defined PLATFORM_OS_OSX
     new OSXSoundPlayer()
#else
#  warning Sound card support disabled.
     NULL
#endif
    ;

  mixer =
#if defined HAVE_PULSE
    new PulseMixer()
#elif defined PLATFORM_OS_WIN32
    new W32Mixer()
#else
    NULL
#endif
    ;

  must_unmute = false;
  delayed_mute = false;
}

SoundPlayer::~SoundPlayer()
{
  delete driver;
}


void
SoundPlayer::init()
{
  if (driver != NULL)
    {
      driver->init(this);
    }

  if (mixer != NULL)
    {
      mixer->init();
    }

  int volume = 0;
  if( !config->get_value(CFG_KEY_SOUND_VOLUME, volume) )
    {
      // Volume value was not found, so set default volume @100.
      // This doesn't belong here if Workrave won't honor it on all platforms.
      config->set_value(CFG_KEY_SOUND_VOLUME, (int)100);
    }

  // first call will set enabled if no setting not found
  is_enabled();

  register_sound_events();
}

void
SoundPlayer::register_sound_events(string theme)
{
  TRACE_ENTER_MSG("SoundPlayer::register_sound_events", theme);
  if (theme == "")
    {
      theme = "default";
    }

  sync_settings();

  gchar *path = g_build_filename(theme.c_str(), "soundtheme", NULL);
  if (path != NULL)
    {
      string file = Util::complete_directory(path, Util::SEARCH_PATH_SOUNDS);
      TRACE_MSG(file);

      Theme theme;
      load_sound_theme(file, theme);

      activate_theme(theme, false);
      g_free(path);
    }
  TRACE_EXIT();
}


void
SoundPlayer::activate_theme(const Theme &theme, bool force)
{
  int idx = 0;
  for (vector<string>::const_iterator it = theme.files.begin();
       it != theme.files.end() && idx < SOUND_MAX;
       it++)
    {
      const string &filename = *it;

      bool enabled = false;
      bool valid = config->get_value(string(CFG_KEY_SOUND_EVENTS) +
                                                              sound_registry[idx].id +
                                                              CFG_KEY_SOUND_EVENTS_ENABLED,
                                                              enabled);

      if (!valid)
        {
          SoundPlayer::set_sound_enabled((SoundEvent)idx, true);
        }

      string current_filename;
      valid = config->get_value(string(CFG_KEY_SOUND_EVENTS) +
                                                         sound_registry[idx].id,
                                                         current_filename);

      if (valid && !g_file_test(current_filename.c_str(), G_FILE_TEST_IS_REGULAR))
        {
          valid = false;
        }

      if (!valid || force)
        {
          set_sound_wav_file((SoundEvent)idx, filename);
        }

      idx++;
    }
}


void
SoundPlayer::sync_settings()
{
  if (driver != NULL)
    {
      for (unsigned int i = 0; i < sizeof(sound_registry)/sizeof(sound_registry[0]); i++)
        {
          SoundRegistry *snd = &sound_registry[i];

          bool enabled = false;
          bool valid = driver->get_sound_enabled((SoundEvent)i, enabled);

          if (valid)
            {
              config->set_value(string(SoundPlayer::CFG_KEY_SOUND_EVENTS) +
                                                         snd->id +
                                                         SoundPlayer::CFG_KEY_SOUND_EVENTS_ENABLED,
                                                         enabled);
            }
          else
            {
              driver->set_sound_enabled((SoundEvent)i, true);
            }

          string wav_file;
          valid = driver->get_sound_wav_file((SoundEvent)i, wav_file);
          if (valid)
           {
              config->set_value(string(SoundPlayer::CFG_KEY_SOUND_EVENTS) +
                                                         snd->id,
                                                         wav_file);
            }
        }
    }
}


void
SoundPlayer::load_sound_theme(const string &themefilename, Theme &theme)
{
  TRACE_ENTER_MSG("SoundPlayer::load_sound_theme", themefilename);

  gboolean r = TRUE;
  bool is_current = true;

  GKeyFile *config_file = g_key_file_new();

  r = g_key_file_load_from_file(config_file, themefilename.c_str(), G_KEY_FILE_KEEP_COMMENTS, NULL);
  TRACE_MSG("load " << r);

  if (r)
    {
      gchar *themedir = g_path_get_dirname(themefilename.c_str());
      TRACE_MSG(themedir);

      char *desc = g_key_file_get_string(config_file, "general", "description", NULL);
      if (desc != NULL)
        {
          theme.description = desc;
          g_free(desc);
        }

      int size = sizeof(sound_registry)/sizeof(sound_registry[0]);
      for (int i = 0; i < size; i++)
        {
          SoundRegistry *snd = &sound_registry[i];
          char *sound_pathname = NULL;

          char *filename = g_key_file_get_string(config_file, snd->id, "file", NULL);
          if (filename != NULL)
            {
              gchar *pathname = g_build_filename(themedir, filename, NULL);
              if (pathname != NULL)
                {
#ifdef HAVE_REALPATH
                  sound_pathname = realpath(pathname, NULL);
                  if (sound_pathname == NULL)
                    {
                      sound_pathname = g_strdup(pathname);
                    }
#else
                  sound_pathname = g_strdup(pathname);
#endif
                  if (is_current)
                    {
                      string current = "";
                      config->get_value(string(CFG_KEY_SOUND_EVENTS) +
                                                                 snd->id,
                                                                 current);

                      if (current != string(sound_pathname))
                        {
                          is_current = false;
                        }
                    }
                  g_free(pathname);
                }
              g_free(filename);
            }

          TRACE_MSG((sound_pathname != NULL ? sound_pathname : "No Sound"));
          theme.files.push_back((sound_pathname != NULL ? sound_pathname : ""));
          g_free(sound_pathname);
        }

      theme.active = is_current;
      g_free(themedir);
    }

  g_key_file_free(config_file);

  TRACE_MSG(is_current);
  TRACE_EXIT();
}


void
SoundPlayer::get_sound_themes(std::vector<Theme> &themes)
{
  TRACE_ENTER("SoundPlayer::get_sound_themes");
  set<string> searchpath = Util::get_search_path(Util::SEARCH_PATH_SOUNDS);
  bool has_active = false;

  sync_settings();

  for (set<string>::iterator it = searchpath.begin(); it != searchpath.end(); it++)
    {
      GDir *dir = g_dir_open(it->c_str(), 0, NULL);

      if (dir != NULL)
        {
          const char *file;
		      while ((file = g_dir_read_name(dir)) != NULL)
            {
              gchar *test_path = g_build_filename(it->c_str(), file, NULL);

              if (test_path != NULL && g_file_test(test_path, G_FILE_TEST_IS_DIR))
                {
                  char *path = g_build_filename(it->c_str(), file, "soundtheme", NULL);

                  if (path != NULL && g_file_test(path, G_FILE_TEST_IS_REGULAR))
                    {
                      Theme theme;

                      load_sound_theme(path, theme);
                      themes.push_back(theme);

                      if (theme.active)
                        {
                          has_active = true;
                        }
                    }

                  g_free(path);
                }

              g_free(test_path);
            }

          g_dir_close(dir);
        }
    }

  if (!has_active)
    {
      Theme active_theme;

      active_theme.active = true;
      active_theme.description = _("Custom");

      bool valid = true;
      for (unsigned int i = 0; valid && i < sizeof(sound_registry)/sizeof(sound_registry[0]); i++)
        {
          string file;

          SoundRegistry *snd = &sound_registry[i];
          bool valid = config->get_value(string(CFG_KEY_SOUND_EVENTS) +
                                                                  snd->id,
                                                                  file);
          if (valid && file != "")
            {
              active_theme.files.push_back(file);
            }
        }

      if (valid)
        {
          themes.push_back(active_theme);
        }
    }

  TRACE_EXIT();
}

void
SoundPlayer::play_sound(SoundEvent snd, bool mute_after_playback)
{
  TRACE_ENTER_MSG("SoundPlayer::play_sound ", snd << " " << mute_after_playback);
  if (is_enabled() &&
      snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      sync_settings();

      bool enabled = false;
      bool valid = SoundPlayer::get_sound_enabled(snd, enabled);

      if (valid && enabled)
        {
          delayed_mute = false;
          if (mute_after_playback &&
              mixer != NULL && driver != NULL &&
              driver->capability(SOUND_CAP_EOS_EVENT))
            {
              delayed_mute = true;
            }

          if (driver != NULL)
            {
              if (driver->capability(SOUND_CAP_EVENTS))
                {
                  driver->play_sound(snd);
                }
              else
                {
                  string filename;
                  bool valid = SoundPlayer::get_sound_wav_file(snd, filename);

                  if (valid)
                    {
                      driver->play_sound(filename);
                    }
                  else
                    {
                      delayed_mute = false;
                    }
                }
            }
        }
    }
  TRACE_EXIT();
}


void
SoundPlayer::play_sound(string wavfile)
{
  TRACE_ENTER("SoundPlayer::play_sound");
  if (is_enabled())
    {
      if (driver != NULL)
        {
          driver->play_sound(wavfile);
        }
    }
  TRACE_EXIT();
}

bool
SoundPlayer::is_enabled()
{
  bool b;
  bool rc;
  b = config
    ->get_value(CFG_KEY_SOUND_ENABLED, rc);
  if (! b)
    {
      rc = true;
      set_enabled(true);
    }
  return rc;
}

void
SoundPlayer::set_enabled(bool b)
{
  config
    ->set_value(CFG_KEY_SOUND_ENABLED, b);
}

bool
SoundPlayer::get_sound_enabled(SoundEvent snd, bool &enabled)
{
  bool ret = false;

  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      enabled = true;

      ret = config->get_value(string(CFG_KEY_SOUND_EVENTS) +
                                                       sound_registry[snd].id +
                                                       CFG_KEY_SOUND_EVENTS_ENABLED,
                                                       enabled);
    }
  else
    {
      enabled = false;
    }

  return ret;
}

void
SoundPlayer::set_sound_enabled(SoundEvent snd, bool enabled)
{
  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      config->set_value(string(SoundPlayer::CFG_KEY_SOUND_EVENTS) +
                                                 sound_registry[snd].id +
                                                 SoundPlayer::CFG_KEY_SOUND_EVENTS_ENABLED,
                                                 enabled);
      if (driver != NULL)
        {
          driver->set_sound_enabled(snd, enabled);
        }
    }
}


bool
SoundPlayer::get_sound_wav_file(SoundEvent snd, string &filename)
{
  bool ret = false;
  filename = "";

  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      ret = config->get_value(string(SoundPlayer::CFG_KEY_SOUND_EVENTS) +
                                                       sound_registry[snd].id,
                                                       filename);
    }
  return ret;
}


void
SoundPlayer::set_sound_wav_file(SoundEvent snd, const string &wav_file)
{
  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      config->set_value(string(SoundPlayer::CFG_KEY_SOUND_EVENTS) +
                                                 sound_registry[snd].id,
                                                 wav_file);
      if (driver != NULL)
        {
          driver->set_sound_wav_file(snd, wav_file);
        }
    }
}


bool
SoundPlayer::capability(SoundCapability cap)
{
  bool ret = false;

  if (mixer != NULL && cap == SOUND_CAP_MUTE)
    {
      ret = true;
    }

  if (!ret && driver != NULL)
    {
      ret = driver->capability(cap);
    }

  return ret;
}

void
SoundPlayer::restore_mute()
{
  TRACE_ENTER("SoundPlayer::restore_mute");

  if (mixer != NULL && must_unmute)
    {
      mixer->set_mute(false);
    }

  TRACE_EXIT();
}


void
SoundPlayer::eos_event()
{
  TRACE_ENTER("SoundPlayer::eos_event");
  if (delayed_mute && mixer != NULL)
    {
      TRACE_MSG("delayed muting");
      bool was_muted = mixer->set_mute(true);
      if (!was_muted)
        {
          must_unmute = true;
        }
    }
  TRACE_EXIT();
}
