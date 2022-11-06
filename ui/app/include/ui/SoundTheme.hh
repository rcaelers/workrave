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

#ifndef WORKRAVE_UI_SOUNDTHEME_HH
#define WORKRAVE_UI_SOUNDTHEME_HH

#include <string>
#include <vector>
#include <map>

#include <memory>

#include "config/IConfigurator.hh"
#include "config/Setting.hh"
#include "audio/ISoundPlayer.hh"

enum class SoundEvent
{
  BreakPrelude,
  BreakIgnored,
  RestBreakStarted,
  RestBreakEnded,
  MicroBreakStarted,
  MicroBreakEnded,
  DailyLimit,
  ExerciseEnded,
  ExercisesEnded,
  ExerciseStep,
};

class SoundTheme
{
public:
  auto sound_enabled() -> workrave::config::Setting<bool> &;
  auto sound_device() -> workrave::config::Setting<std::string> &;
  auto sound_volume() -> workrave::config::Setting<int> &;
  auto sound_mute() -> workrave::config::Setting<bool> &;
  auto sound_event(SoundEvent event) -> workrave::config::Setting<std::string> &;
  auto sound_event_enabled(SoundEvent event) -> workrave::config::Setting<bool> &;

  static auto events() -> std::list<SoundEvent>
  {
    return std::list<SoundEvent>{SoundEvent::BreakPrelude,
                                 SoundEvent::BreakIgnored,
                                 SoundEvent::MicroBreakStarted,
                                 SoundEvent::MicroBreakEnded,
                                 SoundEvent::RestBreakStarted,
                                 SoundEvent::RestBreakEnded,
                                 SoundEvent::DailyLimit,
                                 SoundEvent::ExerciseEnded,
                                 SoundEvent::ExercisesEnded,
                                 SoundEvent::ExerciseStep};
  }

  class SoundInfo
  {
  public:
    SoundEvent event;
    std::string filename;
  };

  class ThemeInfo
  {
  public:
    using Ptr = std::shared_ptr<ThemeInfo>;

    std::string theme_id;
    std::string description;
    std::vector<SoundInfo> sounds;
  };
  using ThemeInfos = std::vector<SoundTheme::ThemeInfo::Ptr>;

  using Ptr = std::shared_ptr<SoundTheme>;

  explicit SoundTheme(std::shared_ptr<workrave::config::IConfigurator> config);
  virtual ~SoundTheme();

  void init();
  void play_sound(SoundEvent snd, bool mute_after_playback = false);
  void play_sound(std::string wavfile);
  void restore_mute();
  auto capability(workrave::audio::SoundCapability cap) -> bool;

  auto get_active_theme() -> ThemeInfo::Ptr;
  auto get_theme(const std::string &theme_id) -> ThemeInfo::Ptr;
  auto get_themes() -> ThemeInfos;

  void activate_theme(const std::string &theme_id);

  static auto sound_id_to_event(const std::string &id) -> SoundEvent;
  static auto sound_event_to_id(SoundEvent event) -> std::string;

private:
  void load_themes();
  auto load_sound_theme(const std::string &themedir) -> ThemeInfo::Ptr;
  void register_sound_events();

#if defined(PLATFORM_OS_WINDOWS)
  void windows_remove_deprecated_appevents();
#endif

  auto sound_event(const std::string &event) -> workrave::config::Setting<std::string> &;
  auto sound_event_enabled(const std::string &event) -> workrave::config::Setting<bool> &;

private:
  std::shared_ptr<workrave::config::IConfigurator> config;
  workrave::audio::ISoundPlayer::Ptr player;
  SoundTheme::ThemeInfos themes;

  struct SoundRegistry
  {
    SoundEvent event;
    std::string id;
  };

  static const SoundRegistry sound_registry[];

  static const std::string CFG_KEY_SOUND_ENABLED;
  static const std::string CFG_KEY_SOUND_DEVICE;
  static const std::string CFG_KEY_SOUND_VOLUME;
  static const std::string CFG_KEY_SOUND_EVENT;
  static const std::string CFG_KEY_SOUND_EVENT_ENABLED;
  static const std::string CFG_KEY_SOUND_MUTE;
};

inline auto
operator<<(std::ostream &stream, SoundEvent event) -> std::ostream &
{
  stream << SoundTheme::sound_event_to_id(event);
  return stream;
}

#endif // WORKRAVE_UI_SOUNDTHEME_HH
