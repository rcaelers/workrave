// SoundTheme.cc
//
// Copyright (C) 2002 - 2013 Rob Caelers & Raymond Penners
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

#include <glib.h>

#include <list>
#include <set>

#include "SoundTheme.hh"

#include "config/IConfigurator.hh"

#include "Util.hh"

using namespace workrave;
using namespace workrave::config;
using namespace workrave::audio;
using namespace std;

const char *SoundTheme::CFG_KEY_SOUND_ENABLED = "sound/enabled";
const char *SoundTheme::CFG_KEY_SOUND_DEVICE = "sound/device";
const char *SoundTheme::CFG_KEY_SOUND_VOLUME = "sound/volume";
const char *SoundTheme::CFG_KEY_SOUND_MUTE = "sound/mute";
const char *SoundTheme::CFG_KEY_SOUND_EVENTS = "sound/events/";
const char *SoundTheme::CFG_KEY_SOUND_EVENTS_ENABLED = "_enabled";


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

SoundTheme::Ptr
SoundTheme::create(workrave::config::IConfigurator::Ptr config)
{
  return SoundTheme::Ptr(new SoundTheme(config));
}

SoundTheme::SoundTheme(workrave::config::IConfigurator::Ptr config)
  : config(config)
{
  player = ISoundPlayer::create();
}

SoundTheme::~SoundTheme()
{
}


void
SoundTheme::init()
{
  player->init();
  register_sound_events();

  int volume = 0;
  if( !config->get_value(CFG_KEY_SOUND_VOLUME, volume) )
    {
      // Volume value was not found, so set default volume @100.
      // This doesn't belong here if Workrave won't honor it on all platforms.
      config->set_value(CFG_KEY_SOUND_VOLUME, (int)100);
    }
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
#ifdef PLATFORM_OS_WIN32
  for (unsigned int i = 0; i < sizeof(sound_registry)/sizeof(sound_registry[0]); i++)
    {
      SoundRegistry *snd = &sound_registry[i];

      bool enabled = false;
      bool valid = win32_get_sound_enabled((SoundEvent)i, enabled);

      if (valid)
        {
          config->set_value(string(SoundTheme::CFG_KEY_SOUND_EVENTS) +
                            snd->id +
                            SoundTheme::CFG_KEY_SOUND_EVENTS_ENABLED,
                            enabled);
        }
      else
        {
          win32_set_sound_enabled((SoundEvent)i, true);
        }

      string wav_file;
      valid = win32_get_sound_wav_file((SoundEvent)i, wav_file);
      if (valid)
        {
          config->set_value(string(SoundTheme::CFG_KEY_SOUND_EVENTS) +
                            snd->id,
                            wav_file);
        }
    }
#endif
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
      
#ifdef PLATFORM_OS_WIN32
          win32_set_sound_enabled(snd, enabled);
#endif
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
#ifdef PLATFORM_OS_WIN32
      win32_set_sound_wav_file(snd, wav_file);
#endif
    }
}



void
SoundTheme::play_sound(SoundEvent snd, bool mute_after_playback)
{
  TRACE_ENTER_MSG("SoundPlayer::play_sound ", snd << " " << mute_after_playback);
  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      bool enabled = false;
      bool valid = get_sound_enabled(snd, enabled);

      if (valid && enabled)
        {
          string filename;
          bool valid = get_sound_wav_file(snd, filename);

          int volume = 100;
          config->get_value(SoundTheme::CFG_KEY_SOUND_VOLUME, volume);
          
          if (valid)
            {
              player->play_sound(snd, filename, false, volume);
            }
          else
            {
              player->play_sound(snd, "", false, volume);
            }
        }
    }
  TRACE_EXIT();
}


void
SoundTheme::play_sound(string wavfile)
{
  TRACE_ENTER("SoundPlayer::play_sound");

  int volume = 100;
  config->get_value(SoundTheme::CFG_KEY_SOUND_VOLUME, volume);
  
  player->play_sound(wavfile, volume);
  TRACE_EXIT();
}

void
SoundTheme::restore_mute()
{
  player->restore_mute();
}

bool
SoundTheme::capability(workrave::audio::SoundCapability cap)
{
  return player->capability(cap);
}

#ifdef PLATFORM_OS_WIN32
static bool
registry_get_value(const char *path, const char *name,
                   char *out)
{
  HKEY handle;
  bool rc = false;
  LONG err;

  err = RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_ALL_ACCESS, &handle);
  if (err == ERROR_SUCCESS)
    {
      DWORD type, size;
      size = MAX_PATH;
      err = RegQueryValueEx(handle, name, 0, &type, (LPBYTE) out, &size);
      if (err == ERROR_SUCCESS)
        {
          rc = true;
        }
      RegCloseKey(handle);
    }
  return rc;
}

static bool
registry_set_value(const char *path, const char *name,
                   const char *value)
{
  HKEY handle;
  bool rc = false;
  DWORD disp;
  LONG err;

  err = RegCreateKeyEx(HKEY_CURRENT_USER, path, 0,
                       "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                       NULL, &handle, &disp);
  if (err == ERROR_SUCCESS)
    {
      err = RegSetValueEx(handle, name, 0, REG_SZ, (BYTE *) value,
                          strlen(value)+1);
      RegCloseKey(handle);
      rc = (err == ERROR_SUCCESS);
    }
  return rc;
}

bool
SoundTheme::win32_get_sound_enabled(SoundEvent snd, bool &enabled)
{
  char key[MAX_PATH], val[MAX_PATH];

  strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
  strcat(key, SoundPlayer::sound_registry[snd].label);
  strcat(key, "\\.current");

  if (registry_get_value(key, NULL, val))
    {
      enabled = (val[0] != '\0');
      return true;
    }

  return false;
}


void
SoundTheme::win32_set_sound_enabled(SoundEvent snd, bool enabled)
{
  if (enabled)
    {
      char key[MAX_PATH], def[MAX_PATH];

      strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
      strcat(key, SoundPlayer::sound_registry[snd].label);
      strcat(key, "\\.default");

      if (registry_get_value(key, NULL, def))
        {
          char *defkey = strrchr(key, '.');
          strcpy(defkey, ".current");
          registry_set_value(key, NULL, def);
        }
    }
  else
    {
      char key[MAX_PATH];

      strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
      strcat(key, SoundPlayer::sound_registry[snd].label);
      strcat(key, "\\.current");

      registry_set_value(key, NULL, "");
    }
}


bool
SoundTheme::win32_get_sound_wav_file(SoundEvent snd, std::string &wav_file)
{
  char key[MAX_PATH], val[MAX_PATH];

  strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
  strcat(key, SoundPlayer::sound_registry[snd].label);
  strcat(key, "\\.current");

  if (registry_get_value(key, NULL, val))
    {
      wav_file = val;
    }

  if (wav_file == "")
    {
      char *cur = strrchr(key, '.');
      strcpy(cur, ".default");
      if (registry_get_value(key, NULL, val))
        {
          wav_file = val;
        }
    }

  return true;
}

void
SoundTheme::win32_set_sound_wav_file(SoundEvent snd, const std::string &wav_file)
{
  char key[MAX_PATH], val[MAX_PATH];

  strcpy(key, "AppEvents\\EventLabels\\");
  strcat(key, SoundPlayer::sound_registry[snd].label);
  if (! registry_get_value(key, NULL, val))
    {
      registry_set_value(key, NULL, SoundPlayer::sound_registry[snd].friendly_name);
    }

  strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
  strcat(key, SoundPlayer::sound_registry[snd].label);
  strcat(key, "\\.default");
  registry_set_value(key, NULL, wav_file.c_str());

  bool enabled = false;
  bool valid = get_sound_enabled(snd, enabled);

  if (!valid || enabled)
    {
      char *def = strrchr(key, '.');
      strcpy(def, ".current");
      registry_set_value(key, NULL, wav_file.c_str());
    }
}


#endif
