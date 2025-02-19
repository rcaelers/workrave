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
#  include "config.h"
#endif

#include "ui/SoundTheme.hh"

#include <algorithm>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/format.hpp>

#include "debug.hh"

#include <list>
#include <set>

#include "config/IConfigurator.hh"
#include "config/SettingCache.hh"
#include "utils/AssetPath.hh"
#include "utils/Platform.hh"

using namespace workrave;
using namespace workrave::config;
using namespace workrave::audio;
using namespace workrave::utils;

const std::array<SoundTheme::SoundRegistry, 10> SoundTheme::sound_registry = {{
  {
    .event = SoundEvent::BreakPrelude,
    .id = "break_prelude",
  },
  {
    .event = SoundEvent::BreakIgnored,
    .id = "break_ignored",
  },
  {
    .event = SoundEvent::RestBreakStarted,
    .id = "rest_break_started",
  },
  {
    .event = SoundEvent::RestBreakEnded,
    .id = "rest_break_ended",
  },
  {
    .event = SoundEvent::MicroBreakStarted,
    .id = "micro_break_started",
  },
  {
    .event = SoundEvent::MicroBreakEnded,
    .id = "micro_break_ended",
  },
  {
    .event = SoundEvent::DailyLimit,
    .id = "daily_limit",
  },
  {
    .event = SoundEvent::ExerciseEnded,
    .id = "exercise_ended",
  },
  {
    .event = SoundEvent::ExercisesEnded,
    .id = "exercises_ended",
  },
  {
    .event = SoundEvent::ExerciseStep,
    .id = "exercise_step",
  },
}};

const std::string SoundTheme::CFG_KEY_SOUND_ENABLED = "sound/enabled";
const std::string SoundTheme::CFG_KEY_SOUND_DEVICE = "sound/device";
const std::string SoundTheme::CFG_KEY_SOUND_VOLUME = "sound/volume";
const std::string SoundTheme::CFG_KEY_SOUND_MUTE = "sound/mute";
const std::string SoundTheme::CFG_KEY_SOUND_EVENT = "sound/events/";
const std::string SoundTheme::CFG_KEY_SOUND_EVENT_ENABLED = "_enabled";

workrave::config::Setting<bool> &
SoundTheme::sound_enabled()
{
  return SettingCache::get<bool>(config, CFG_KEY_SOUND_ENABLED, true);
}

workrave::config::Setting<std::string> &
SoundTheme::sound_device()
{
  return SettingCache::get<std::string>(config, CFG_KEY_SOUND_DEVICE, std::string());
}

workrave::config::Setting<int> &
SoundTheme::sound_volume()
{
  return SettingCache::get<int>(config, CFG_KEY_SOUND_VOLUME, 100);
}

workrave::config::Setting<bool> &
SoundTheme::sound_mute()
{
  return SettingCache::get<bool>(config, CFG_KEY_SOUND_MUTE, false);
}

workrave::config::Setting<bool> &
SoundTheme::sound_event_enabled(const std::string &event)
{
  return SettingCache::get<bool>(config, CFG_KEY_SOUND_EVENT + event + CFG_KEY_SOUND_EVENT_ENABLED, true);
}

workrave::config::Setting<std::string> &
SoundTheme::sound_event(const std::string &event)
{
  return SettingCache::get<std::string>(config, CFG_KEY_SOUND_EVENT + event, std::string());
}

workrave::config::Setting<bool> &
SoundTheme::sound_event_enabled(SoundEvent event)
{
  return SettingCache::get<bool>(config, CFG_KEY_SOUND_EVENT + sound_event_to_id(event) + CFG_KEY_SOUND_EVENT_ENABLED, true);
}

workrave::config::Setting<std::string> &
SoundTheme::sound_event(SoundEvent event)
{
  return SettingCache::get<std::string>(config, CFG_KEY_SOUND_EVENT + sound_event_to_id(event), std::string());
}

SoundEvent
SoundTheme::sound_id_to_event(const std::string &id)
{
  const SoundRegistry *item = std::ranges::find_if(sound_registry, [&](const SoundRegistry &item) {
    return item.id == id;
  });
  if (item != std::end(sound_registry))
    {
      return item->event;
    }
  throw std::runtime_error("Sound ID not found");
}

std::string
SoundTheme::sound_event_to_id(SoundEvent event)
{
  const SoundRegistry *item = std::ranges::find_if(sound_registry, [&](const SoundRegistry &item) {
    return item.event == event;
  });
  if (item != std::end(sound_registry))
    {
      return item->id;
    }
  throw std::runtime_error("Sound event not found");
}

SoundTheme::SoundTheme(std::shared_ptr<workrave::config::IConfigurator> config)
  : config(config)
{
  TRACE_ENTRY();
  player = SoundPlayerFactory::create();

#if defined(PLATFORM_OS_WINDOWS)
  windows_remove_deprecated_appevents();
#endif
}

SoundTheme::~SoundTheme()
{
  TRACE_ENTRY();
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
  TRACE_ENTRY();
  ThemeInfo::Ptr theme = get_theme("subtle");
  if (theme)
    {
      for (SoundInfo sound: theme->sounds)
        {
          std::filesystem::path path(SoundTheme::sound_event(sound.event)());
          if (!std::filesystem::is_regular_file(path))
            {
              SoundTheme::sound_event(sound.event).set(sound.filename);
            }
        }
    }
}

void
SoundTheme::activate_theme(const std::string &theme_id)
{
  TRACE_ENTRY_PAR(theme_id);
  ThemeInfo::Ptr theme = get_theme(theme_id);
  if (theme)
    {
      for (SoundInfo sound: theme->sounds)
        {
          // TRACE_MSG("activating {} {}",  sound.event, sound.filename);
          SoundTheme::sound_event(sound.event).set(sound.filename);
        }
    }
}

void
SoundTheme::load_themes()
{
  TRACE_ENTRY();
  for (const auto &dirname: AssetPath::get_search_path(SearchPathId::Sounds))
    {
      std::filesystem::path dirpath(dirname);

      if (!std::filesystem::is_directory(dirpath))
        {
          continue;
        }

      std::filesystem::directory_iterator dir_end;
      for (std::filesystem::directory_iterator it(dirname); it != dir_end; ++it)
        {
          if (std::filesystem::is_directory(it->status()))
            {
              ThemeInfo::Ptr theme = load_sound_theme(it->path().string());
              if (theme)
                {
                  themes.push_back(theme);
                }
            }
        }
    }
}

SoundTheme::ThemeInfo::Ptr
SoundTheme::load_sound_theme(const std::string &themedir)
{
  TRACE_ENTRY_PAR(themedir);
  ThemeInfo::Ptr theme(new ThemeInfo);

  try
    {
      std::filesystem::path file = themedir;
      file /= "soundtheme";

      std::filesystem::path path(themedir);

      boost::property_tree::ptree pt;
      boost::property_tree::ini_parser::read_ini(file.string(), pt);

      theme->theme_id = path.stem().string();
      theme->description = pt.get<std::string>("general.description");
      TRACE_MSG("id = {}", theme->theme_id);
      TRACE_MSG("descr = {}", theme->description);

      for (const SoundRegistry &snd: sound_registry)
        {
          auto filename = pt.get<std::string>(snd.id + ".file");

          std::filesystem::path soundpath(themedir);

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
  for (SoundTheme::ThemeInfo::Ptr theme: themes)
    {
      bool is_current = true;
      for (SoundInfo sound: theme->sounds)
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
  theme->description = "";

  for (const SoundRegistry &snd: sound_registry)
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
  auto it = std::find_if(themes.begin(), themes.end(), [&](ThemeInfo::Ptr item) { return item->theme_id == theme_id; });
  if (it != themes.end())
    {
      return *it;
    }

  return {};
}

void
SoundTheme::play_sound(SoundEvent snd, bool mute_after_playback)
{
  TRACE_ENTRY_PAR(snd, mute_after_playback);
  bool enabled = sound_event_enabled(snd)() && sound_enabled()();

  if (enabled)
    {
      std::string filename = SoundTheme::sound_event(snd)();
      if (!filename.empty())
        {
          player->play_sound(filename, mute_after_playback, SoundTheme::sound_volume()());
        }
    }
}

void
SoundTheme::play_sound(std::string wavfile)
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

#if defined(PLATFORM_OS_WINDOWS)
void
SoundTheme::windows_remove_deprecated_appevents()
{
  const std::array<std::string, 10> ids = {"WorkraveBreakPrelude",
                                           "WorkraveBreakIgnored",
                                           "WorkraveRestBreakStarted",
                                           "WorkraveRestBreakEnded",
                                           "WorkraveMicroBreakStarted",
                                           "WorkraveMicroBreakEnded",
                                           "WorkraveDailyLimit",
                                           "WorkraveExerciseEnded",
                                           "WorkraveExercisesEnded",
                                           "WorkraveExerciseStep"};

  std::string schemes = R"(AppEvents\Schemes\Apps\Workrave\)";
  std::string event_labels = "AppEvents\\EventLabels\\";

  for (std::string id: ids)
    {
      std::string key = schemes + id + "\\.current";
      Platform::registry_set_value(key.c_str(), nullptr, nullptr);
      key = schemes + id + "\\.default";
      Platform::registry_set_value(key.c_str(), nullptr, nullptr);
      key = schemes + id;
      Platform::registry_set_value(key.c_str(), nullptr, nullptr);
      key = event_labels + id;
      Platform::registry_set_value(key.c_str(), nullptr, nullptr);
    }

  // FIXME: used in ChangeAutoRun.c
  Platform::registry_set_value(schemes.c_str(), nullptr, "");
}
#endif
