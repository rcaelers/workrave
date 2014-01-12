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

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/format.hpp>

#include "debug.hh"
#include "nls.h"

#ifdef HAVE_REALPATH
#include <limits.h>
#include <stdlib.h>
#endif
#ifdef PLATFORM_OS_WIN32
#include <windows.h>
#endif

#include <list>
#include <set>

#include "SoundTheme.hh"

#include "config/IConfigurator.hh"

#include "utils/AssetPath.hh"

using namespace workrave;
using namespace workrave::config;
using namespace workrave::audio;
using namespace workrave::utils;
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
    _("Break prompt")
  },

  { "WorkraveBreakIgnored",
    "break_ignored",
    _("Break ignored")
  },

  { "WorkraveRestBreakStarted",
    "rest_break_started",
    _("Rest break started")
  },

  {
    "WorkraveRestBreakEnded",
    "rest_break_ended",
    _("Rest break ended")
  },

  { "WorkraveMicroBreakStarted",
    "micro_break_started",
    _("Micro-break started")
  },

  { "WorkraveMicroBreakEnded",
    "micro_break_ended",
    _("Micro-break ended")
  },

  { "WorkraveDailyLimit",
    "daily_limit",
    _("Daily limit")
  },

  { "WorkraveExerciseEnded",
    "exercise_ended",
    _("Exercise ended")
  },

  { "WorkraveExercisesEnded",
    "exercises_ended",
    _("Exercises ended")
  },

  { "WorkraveExerciseStep",
    "exercise_step",
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
SoundTheme::register_sound_events(string theme_name)
{
  TRACE_ENTER_MSG("SoundTheme::register_sound_events", theme_name);
  if (theme_name == "")
    {
      theme_name = "default";
    }

  sync_settings();

  boost::filesystem::path path(theme_name);
  path /= "soundtheme";
  
  string file = AssetPath::complete_directory(path.string(), AssetPath::SEARCH_PATH_SOUNDS);
  TRACE_MSG(file);

  Theme theme;
  load_sound_theme(file, theme);
  
  activate_theme(theme, false);

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

      boost::filesystem::path path(current_filename);
      
      if (valid && !boost::filesystem::is_regular_file(path))
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

  try
    {
      bool is_current = true;

      boost::filesystem::path path(themefilename);
      boost::filesystem::path themedir = path.parent_path();
  
      boost::property_tree::ptree pt;
      boost::property_tree::ini_parser::read_ini(themefilename, pt);

      theme.description = pt.get<std::string>("general.description");

      int size = sizeof(sound_registry)/sizeof(sound_registry[0]);
      for (int i = 0; i < size; i++)
        {
          SoundRegistry *snd = &sound_registry[i];

          string item = boost::str(boost::format("%1%.file") % snd->id);
          string filename = pt.get<std::string>(item);

          boost::filesystem::path soundpath(themedir);
          soundpath /= filename;

          soundpath = canonical(soundpath);

          if (is_current)
            {
              string current = "";
              config->get_value(string(CFG_KEY_SOUND_EVENTS) +
                                snd->id,
                                current);
          
              if (current != soundpath.string())
                {
                  is_current = false;
                }
            }

          theme.files.push_back(soundpath.string());

          theme.active = is_current;
        }
      TRACE_MSG(is_current);
    }
  catch (boost::exception &)
    {
      theme.active = false;
    }

  TRACE_EXIT();
}


void
SoundTheme::get_sound_themes(std::vector<Theme> &themes)
{
  TRACE_ENTER("SoundTheme::get_sound_themes");
  set<string> searchpath = AssetPath::get_search_path(AssetPath::SEARCH_PATH_SOUNDS);
  bool has_active = false;

  sync_settings();

  for (const auto & dirname : searchpath)
    {
      boost::filesystem::path dirpath(dirname);

      if (! boost::filesystem::is_directory(dirpath))
        {
          continue;
        }
      
      boost::filesystem::directory_iterator dir_end;
      for (boost::filesystem::directory_iterator it(dirname); it != dir_end; ++it)
        {
          if (boost::filesystem::is_directory(it->status()))
            {
              boost::filesystem::path file = it->path();
              file /= "soundtheme";
              
              if (boost::filesystem::is_regular_file(file))
                {
                  Theme theme;
                  
                  load_sound_theme(file.string(), theme);
                  themes.push_back(theme);
                  
                  if (theme.active)
                    {
                      has_active = true;
                    }
                }
            }
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
  strcat(key, sound_registry[snd].label);
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
      strcat(key, sound_registry[snd].label);
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
      strcat(key, sound_registry[snd].label);
      strcat(key, "\\.current");

      registry_set_value(key, NULL, "");
    }
}


bool
SoundTheme::win32_get_sound_wav_file(SoundEvent snd, std::string &wav_file)
{
  char key[MAX_PATH], val[MAX_PATH];

  strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
  strcat(key, sound_registry[snd].label);
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
  strcat(key, sound_registry[snd].label);
  if (! registry_get_value(key, NULL, val))
    {
      registry_set_value(key, NULL, sound_registry[snd].friendly_name);
    }

  strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
  strcat(key, sound_registry[snd].label);
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
