// SoundTheme.cc --- Sound player
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

#include "config/IConfigurator.hh"
//#include "utils/Platform.hh"

#include "Util.hh"

const char *ISoundTheme::CFG_KEY_SOUND_ENABLED = "sound/enabled";
const char *ISoundTheme::CFG_KEY_SOUND_DEVICE = "sound/device";
const char *ISoundTheme::CFG_KEY_SOUND_VOLUME = "sound/volume";
const char *ISoundTheme::CFG_KEY_SOUND_MUTE = "sound/mute";
const char *ISoundTheme::CFG_KEY_SOUND_EVENTS = "sound/events/";
const char *ISoundTheme::CFG_KEY_SOUND_EVENTS_ENABLED = "_enabled";

using namespace workrave;
using namespace workrave::config;
using namespace workrave::utils;
using namespace std;

SoundTheme::SoundRegistry SoundTheme::sound_registry[] =
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
 * SoundTheme
 **********************************************************************/

ISoundTheme::Ptr
ISoundTheme::create(workrave::config::IConfigurator::Ptr config)
{
  return ISoundTheme::Ptr(new SoundTheme(config));
}

SoundTheme::SoundTheme(workrave::config::IConfigurator::Ptr config)
  : config(config)
{
}

SoundTheme::~SoundTheme()
{
  delete driver;
}


void
SoundTheme::init()
{
  register_sound_events();
}

void
SoundTheme::register_sound_events(string theme)
{
  TRACE_ENTER_MSG("SoundTheme::register_sound_events", theme);
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
SoundTheme::activate_theme(const Theme &theme, bool force)
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
          SoundTheme::set_sound_enabled((SoundEvent)idx, true);
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
SoundTheme::sync_settings()
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
              config->set_value(string(SoundTheme::CFG_KEY_SOUND_EVENTS) +
                                                         snd->id +
                                                         SoundTheme::CFG_KEY_SOUND_EVENTS_ENABLED,
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
              config->set_value(string(SoundTheme::CFG_KEY_SOUND_EVENTS) +
                                                         snd->id,
                                                         wav_file);
            }
        }
    }
}


void
SoundTheme::load_sound_theme(const string &themefilename, Theme &theme)
{
  TRACE_ENTER_MSG("SoundTheme::load_sound_theme", themefilename);

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
SoundTheme::get_sound_themes(std::vector<Theme> &themes)
{
  TRACE_ENTER("SoundTheme::get_sound_themes");
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

bool
SoundTheme::is_enabled()
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
SoundTheme::set_enabled(bool b)
{
  config
    ->set_value(CFG_KEY_SOUND_ENABLED, b);
}

bool
SoundTheme::get_sound_enabled(SoundEvent snd, bool &enabled)
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
SoundTheme::set_sound_enabled(SoundEvent snd, bool enabled)
{
  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      config->set_value(string(SoundTheme::CFG_KEY_SOUND_EVENTS) +
                                                 sound_registry[snd].id +
                                                 SoundTheme::CFG_KEY_SOUND_EVENTS_ENABLED,
                                                 enabled);
      if (driver != NULL)
        {
          driver->set_sound_enabled(snd, enabled);
        }
    }
}


bool
SoundTheme::get_sound_wav_file(SoundEvent snd, string &filename)
{
  bool ret = false;
  filename = "";

  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      ret = config->get_value(string(SoundTheme::CFG_KEY_SOUND_EVENTS) +
                                                       sound_registry[snd].id,
                                                       filename);
    }
  return ret;
}


void
SoundTheme::set_sound_wav_file(SoundEvent snd, const string &wav_file)
{
  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      config->set_value(string(SoundTheme::CFG_KEY_SOUND_EVENTS) +
                                                 sound_registry[snd].id,
                                                 wav_file);
      if (driver != NULL)
        {
          driver->set_sound_wav_file(snd, wav_file);
        }
    }
}
