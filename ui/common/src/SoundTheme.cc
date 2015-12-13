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

#include "commonui/SoundTheme.hh"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/format.hpp>

#include "debug.hh"
#include "nls.h"

#include <list>
#include <set>

#include "commonui/Backend.hh"

#include "config/IConfigurator.hh"
#include "config/SettingCache.hh"
#include "utils/AssetPath.hh"

#ifdef PLATFORM_OS_WIN32
#include "utils/Platform.hh"
#endif

using namespace workrave;
using namespace workrave::config;
using namespace workrave::audio;
using namespace workrave::utils;
using namespace std;

SoundTheme::SoundRegistry SoundTheme::sound_registry[] =
  {
    {
      SoundEvent::BreakPrelude,
      "break_prelude",
      _("Break prompt")
    },

    {
      SoundEvent::BreakIgnored,
      "break_ignored",
      _("Break ignored")
    },

    {
      SoundEvent::RestBreakStarted,
      "rest_break_started",
      _("Rest break started")
    },

    {
      SoundEvent::RestBreakEnded,
      "rest_break_ended",
      _("Rest break ended")
    },

    {
      SoundEvent::MicroBreakStarted,
      "micro_break_started",
      _("Micro-break started")
    },

    {
      SoundEvent::MicroBreakEnded,
      "micro_break_ended",
      _("Micro-break ended")
    },

    {
      SoundEvent::DailyLimit,
      "daily_limit",
      _("Daily limit")
    },

    {
      SoundEvent::ExerciseEnded,
      "exercise_ended",
      _("Exercise ended")
    },

    {
      SoundEvent::ExercisesEnded,
      "exercises_ended",
      _("Exercises ended")
    },

    {
      SoundEvent::ExerciseStep,
      "exercise_step",
      _("Exercise change")
    },
  };

const string SoundTheme::CFG_KEY_SOUND_ENABLED = "sound/enabled";
const string SoundTheme::CFG_KEY_SOUND_DEVICE = "sound/device";
const string SoundTheme::CFG_KEY_SOUND_VOLUME = "sound/volume";
const string SoundTheme::CFG_KEY_SOUND_MUTE = "sound/mute";
const string SoundTheme::CFG_KEY_SOUND_EVENT = "sound/events/";
const string SoundTheme::CFG_KEY_SOUND_EVENT_ENABLED = "_enabled";

workrave::config::Setting<bool> &
SoundTheme::sound_enabled()
{
  return SettingCache::get<bool>(Backend::get_configurator(), CFG_KEY_SOUND_ENABLED, true);
}

workrave::config::Setting<std::string> &
SoundTheme::sound_device()
{
  return SettingCache::get<std::string>(Backend::get_configurator(), CFG_KEY_SOUND_DEVICE, std::string());
}

workrave::config::Setting<int> &
SoundTheme::sound_volume()
{
  return SettingCache::get<int>(Backend::get_configurator(), CFG_KEY_SOUND_VOLUME, 100);
}

workrave::config::Setting<bool> &
SoundTheme::sound_mute()
{
  return SettingCache::get<bool>(Backend::get_configurator(), CFG_KEY_SOUND_MUTE, false);
}

workrave::config::Setting<bool> &
SoundTheme::sound_event_enabled(const std::string &event)
{
  return SettingCache::get<bool>(Backend::get_configurator(), CFG_KEY_SOUND_EVENT + event + CFG_KEY_SOUND_EVENT_ENABLED, true);
}

workrave::config::Setting<std::string> &
SoundTheme::sound_event(const std::string &event)
{
  return SettingCache::get<std::string>(Backend::get_configurator(), CFG_KEY_SOUND_EVENT + event, std::string());
}

workrave::config::Setting<bool> &
SoundTheme::sound_event_enabled(SoundEvent event)
{
  return SettingCache::get<bool>(Backend::get_configurator(), CFG_KEY_SOUND_EVENT + sound_event_to_id(event) + CFG_KEY_SOUND_EVENT_ENABLED, true);
}

workrave::config::Setting<std::string> &
SoundTheme::sound_event(SoundEvent event)
{
  return SettingCache::get<std::string>(Backend::get_configurator(), CFG_KEY_SOUND_EVENT + sound_event_to_id(event), std::string());
}

SoundEvent
SoundTheme::sound_id_to_event(const std::string &id)
{
  SoundRegistry *item = std::find_if(std::begin(sound_registry), std::end(sound_registry), [&] (SoundRegistry &item) { return item.id == id; });
  if (item != std::end(sound_registry))
    {
      return item->event;
    }
  throw "FIXME";
}

const std::string
SoundTheme::sound_event_to_id(SoundEvent event)
{
  SoundRegistry *item = std::find_if(std::begin(sound_registry), std::end(sound_registry), [&] (SoundRegistry &item) { return item.event == event; });
  if (item != std::end(sound_registry))
    {
      return item->id;
    }
  throw "FIXME";
}

const std::string
SoundTheme::sound_event_to_friendly_name(SoundEvent event)
{
  SoundRegistry *item = std::find_if(std::begin(sound_registry), std::end(sound_registry), [&] (SoundRegistry &item) { return item.event == event; });
  if (item != std::end(sound_registry))
    {
      return item->friendly_name;
    }
  throw "FIXME";
}

SoundTheme::SoundTheme()
{
  player = SoundPlayerFactory::create();

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
  load_themes();
  register_sound_events();
}

void
SoundTheme::register_sound_events()
{
  TRACE_ENTER("SoundTheme::register_sound_events");

  ThemeInfo::Ptr theme = get_theme("default");
  if (theme)
    {
      for (SoundInfo sound : theme->sounds)
        {
          boost::filesystem::path path(SoundTheme::sound_event(sound.event)());
          if (!boost::filesystem::is_regular_file(path))
            {
              SoundTheme::sound_event(sound.event).set(sound.filename);
            }
        }
    }

  TRACE_EXIT();
}

void
SoundTheme::activate_theme(const std::string &theme_id)
{
  TRACE_ENTER_MSG("SoundTheme::activate_theme", theme_id);
  ThemeInfo::Ptr theme = get_theme(theme_id);
  if (theme)
    {
      for (SoundInfo sound : theme->sounds)
        {
          TRACE_MSG("activating " << sound.event << " " << sound.filename);
          SoundTheme::sound_event(sound.event).set(sound.filename);
        }
    }
  TRACE_EXIT();
}

void
SoundTheme::load_themes()
{
  TRACE_ENTER("SoundTheme::get_sound_themes");

  for (const auto &dirname : AssetPath::get_search_path(AssetPath::SEARCH_PATH_SOUNDS))
    {
      boost::filesystem::path dirpath(dirname);

      if (!boost::filesystem::is_directory(dirpath))
        {
          continue;
        }

      boost::filesystem::directory_iterator dir_end;
      for (boost::filesystem::directory_iterator it(dirname); it != dir_end; ++it)
        {
          if (boost::filesystem::is_directory(it->status()))
            {
              ThemeInfo::Ptr theme = load_sound_theme(it->path().string());
              if (theme)
                {
                  themes.push_back(theme);
                }
            }
        }
    }

  TRACE_EXIT();
}

SoundTheme::ThemeInfo::Ptr
SoundTheme::load_sound_theme(const string &themedir)
{
  TRACE_ENTER_MSG("SoundTheme::load_sound_theme", themedir);
  ThemeInfo::Ptr theme(new ThemeInfo);

  try
    {
      boost::filesystem::path file = themedir;
      file /= "soundtheme";

      boost::filesystem::path path(themedir);

      boost::property_tree::ptree pt;
      boost::property_tree::ini_parser::read_ini(file.string(), pt);

      theme->theme_id = path.stem().string();
      theme->description = pt.get<std::string>("general.description");
      TRACE_MSG("id = " << theme->theme_id);
      TRACE_MSG("descr = " << theme->description);

      for (SoundRegistry &snd : sound_registry)
        {
          string filename = pt.get<std::string>(snd.id + ".file");

          boost::filesystem::path soundpath(themedir);
          soundpath /= filename;
          soundpath = canonical(soundpath);

          SoundInfo sound_info;
          sound_info.event = sound_id_to_event(snd.id);
          sound_info.filename = soundpath.string();
          theme->sounds.push_back(sound_info);
        }
    }
  catch (boost::exception &)
    {
      theme.reset();
    }

  TRACE_EXIT();
  return theme;
}

SoundTheme::ThemeInfos
SoundTheme::get_themes()
{
  return themes;
}

SoundTheme::ThemeInfo::Ptr
SoundTheme::get_active_theme()
{
   for (SoundTheme::ThemeInfo::Ptr theme : themes)
    {
      bool is_current = true;
      for (SoundInfo sound : theme->sounds)
        {
          if (sound.filename != SoundTheme::sound_event(sound.event)())
            {
              is_current = false;
              break;
            }
        }
      if (is_current)
        {
          return theme;
        }
    }


  ThemeInfo::Ptr theme(new ThemeInfo);
  theme->theme_id = "custom";
  theme->description = _("Custom");

  for (SoundRegistry &snd : sound_registry)
    {
      SoundInfo sound_info;
      sound_info.event = sound_id_to_event(snd.id);
      sound_info.filename = SoundTheme::sound_event(snd.event)();
      theme->sounds.push_back(sound_info);
    }

  return theme;
}

SoundTheme::ThemeInfo::Ptr
SoundTheme::get_theme(const std::string &theme_id)
{
  ThemeInfos::iterator it = std::find_if(themes.begin(), themes.end(), [&] (ThemeInfo::Ptr item) { return item->theme_id == theme_id; });
  if (it != themes.end())
    {
      return *it;
    }

  return ThemeInfo::Ptr();
}

void
SoundTheme::play_sound(SoundEvent snd, bool mute_after_playback)
{
  TRACE_ENTER_MSG("SoundPlayer::play_sound ", snd << " " << mute_after_playback);
  bool enabled = SoundTheme::sound_event_enabled(snd)();

  if (enabled)
    {
      string filename = SoundTheme::sound_event(snd)();
      if (filename != "")
        {
          player->play_sound(filename, mute_after_playback, SoundTheme::sound_volume()());
        }
    }

  TRACE_EXIT();
}

void
SoundTheme::play_sound(string wavfile)
{
  player->play_sound(wavfile, false, SoundTheme::sound_volume()());
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

  string schemes = "AppEvents\\Schemes\\Apps\\Workrave\\";
  string event_labels = "AppEvents\\EventLabels\\";

  for (string id : ids)
    {
      string key = schemes + id + "\\.current";
      Platform::registry_set_value(key.c_str(), NULL, NULL);
      key = schemes + id + "\\.default";
      Platform::registry_set_value(key.c_str(), NULL, NULL);
      key = schemes + id;
      Platform::registry_set_value(key.c_str(), NULL, NULL);
      key = event_labels + id;
      Platform::registry_set_value(key.c_str(), NULL, NULL);
    }

  // FIXME: used in ChangeAutoRun.c
  Platform::registry_set_value(schemes.c_str(), NULL, "");
}
#endif
