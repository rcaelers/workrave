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

#ifdef PLATFORM_OS_WIN32
#include "utils/Plaform.hh"
#endif

#include <list>
#include <set>

#include "SoundTheme.hh"
#include "GUIConfig.hh"
#include "config/IConfigurator.hh"

#include "utils/AssetPath.hh"

using namespace workrave;
using namespace workrave::config;
using namespace workrave::audio;
using namespace workrave::utils;
using namespace std;

SoundTheme::SoundRegistry SoundTheme::sound_registry[] =
{
  { "break_prelude",
    _("Break prompt")
  },

  { "break_ignored",
    _("Break ignored")
  },

  { "rest_break_started",
    _("Rest break started")
  },

  { "rest_break_ended",
    _("Rest break ended")
  },

  { "micro_break_started",
    _("Micro-break started")
  },

  { "micro_break_ended",
    _("Micro-break ended")
  },

  { "daily_limit",
    _("Daily limit")
  },
  
  {  "exercise_ended",
    _("Exercise ended")
  },

  {  "exercises_ended",
    _("Exercises ended")
  },

  {  "exercise_step",
    _("Exercise change")
  },
};


/**********************************************************************
 * SoundTheme
 **********************************************************************/

SoundTheme::Ptr
SoundTheme::create()
{
  return SoundTheme::Ptr();
}

SoundTheme::SoundTheme()
{
  player = ISoundPlayer::create();

#ifdef PLATFORM_OS_WIN32
  win32_remove_deprecated_appevents();
#endif
}

SoundTheme::~SoundTheme()
{
}


void
SoundTheme::init()
{
  player->init();
  register_sound_events();
}

void
SoundTheme::register_sound_events(string theme_name)
{
  TRACE_ENTER_MSG("SoundTheme::register_sound_events", theme_name);
  if (theme_name == "")
    {
      theme_name = "default";
    }

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
      string current_filename = GUIConfig::sound_event(sound_registry[idx].id)();

      boost::filesystem::path path(current_filename);
      
      if (force || !boost::filesystem::is_regular_file(path))
        {
          set_sound_wav_file((SoundEvent)idx, filename);
        }

      idx++;
    }
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
              string current = GUIConfig::sound_event(snd->id)();
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
          SoundRegistry *snd = &sound_registry[i];
          string file = GUIConfig::sound_event(snd->id)();
          if (file != "")
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
  return GUIConfig::sound_enabled()();
}

void
SoundTheme::set_enabled(bool b)
{
  GUIConfig::sound_enabled().set(b);
}

bool
SoundTheme::get_sound_enabled(SoundEvent snd)
{
  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      return GUIConfig::sound_event_enabled(sound_registry[snd].id)();
    }

  return false;
}

void
SoundTheme::set_sound_enabled(SoundEvent snd, bool enabled)
{
  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      GUIConfig::sound_event_enabled(sound_registry[snd].id).set(enabled);
    }
}


string
SoundTheme::get_sound_wav_file(SoundEvent snd)
{
  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      return GUIConfig::sound_event(sound_registry[snd].id)();
    }
  return "";
}

void
SoundTheme::set_sound_wav_file(SoundEvent snd, const string &wav_file)
{
  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      GUIConfig::sound_event(sound_registry[snd].id).set(wav_file);
    }
}

void
SoundTheme::play_sound(SoundEvent snd, bool mute_after_playback)
{
  TRACE_ENTER_MSG("SoundPlayer::play_sound ", snd << " " << mute_after_playback);
  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      bool enabled = get_sound_enabled(snd);

      if (enabled)
        {
          string filename = get_sound_wav_file(snd);
          int volume = GUIConfig::sound_volume()();
          
          if (filename != "")
            {
              player->play_sound(snd, filename, mute_after_playback, volume);
            }
        }
    }
  TRACE_EXIT();
}


void
SoundTheme::play_sound(string wavfile)
{
  TRACE_ENTER("SoundPlayer::play_sound");

  int volume = GUIConfig::sound_volume()();
  
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
void
SoundTheme::win32_remove_deprecated_appevents()
{
  const string ids[] = { "WorkraveBreakPrelude",
                         "WorkraveBreakIgnored",
                         "WorkraveRestBreakStarted",
                         "WorkraveRestBreakEnded",
                         "WorkraveMicroBreakStarted",
                         "WorkraveMicroBreakEnded",
                         "WorkraveDailyLimit",
                         "WorkraveExerciseEnded",
                         "WorkraveExercisesEnded",
                         "WorkraveExerciseStep"  };
  
  string schems = "AppEvents\\Schemes\\Apps\\Workrave\\";
  string event_labels = "AppEvents\\EventLabels\\";

  for (string id : ids)
    {
      Platform::registry_set_value(schemes + id + "\\.current", NULL);
      Platform::registry_set_value(schemes + id + "\\.default", NULL);
      Platform::registry_set_value(schemes + id, NULL);
      Platform::registry_set_value(event_labels + id, NULL);
    }

  // FIXME: used in ChangeAutoRun.c
  Platform::registry_set_value(schemes, "");
}


#endif
