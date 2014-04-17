// SoundTheme.hh
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

#ifndef SOUNDTHEME_HH
#define SOUNDTHEME_HH

#include <string>
#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>

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
  static workrave::config::Setting<bool> &sound_enabled();
  static workrave::config::Setting<std::string> &sound_device();
  static workrave::config::Setting<int> &sound_volume();
  static workrave::config::Setting<bool> &sound_mute();
  static workrave::config::Setting<std::string> &sound_event(SoundEvent event);
  static workrave::config::Setting<bool> &sound_event_enabled(SoundEvent event);

  class SoundInfo
  {
  public:
    SoundEvent event;
    std::string filename;
  };

  class ThemeInfo
  {
  public:
    typedef boost::shared_ptr<ThemeInfo> Ptr;

    std::string theme_id;
    std::string description;
    std::vector<SoundInfo> sounds;
  };
  typedef std::vector<SoundTheme::ThemeInfo::Ptr> ThemeInfos;
  
  typedef boost::shared_ptr<SoundTheme> Ptr;
  static Ptr create();
  
  SoundTheme();
  virtual ~SoundTheme();

  void init();
  void play_sound(SoundEvent snd, bool mute_after_playback = false);
  void play_sound(std::string wavfile);
  void restore_mute();
  bool capability(workrave::audio::SoundCapability cap);
  
  ThemeInfo::Ptr get_active_theme();
  ThemeInfo::Ptr get_theme(const std::string &theme_id);
  ThemeInfos get_themes();

  void activate_theme(const std::string &theme_id);

  static SoundEvent sound_id_to_event(const std::string &id);
  static const std::string sound_event_to_id(SoundEvent event);
  static const std::string sound_event_to_friendly_name(SoundEvent event);
  
private:
  void load_themes();
  ThemeInfo::Ptr load_sound_theme(const std::string &themedir);
  void register_sound_events();

#ifdef PLATFORM_OS_WIN32  
  void win32_remove_deprecated_appevents();
#endif

  static workrave::config::Setting<std::string> &sound_event(const std::string &event);
  static workrave::config::Setting<bool> &sound_event_enabled(const std::string &event);

private:
  workrave::audio::ISoundPlayer::Ptr player;
  SoundTheme::ThemeInfos themes;
  
  struct SoundRegistry
  {
    SoundEvent event;
    std::string id;
    std::string friendly_name;
  };

  static SoundRegistry sound_registry[];

  static const std::string CFG_KEY_SOUND_ENABLED;
  static const std::string CFG_KEY_SOUND_DEVICE;
  static const std::string CFG_KEY_SOUND_VOLUME;
  static const std::string CFG_KEY_SOUND_EVENT;
  static const std::string CFG_KEY_SOUND_EVENT_ENABLED;
  static const std::string CFG_KEY_SOUND_MUTE;
};

inline std::ostream& operator<<(std::ostream& stream, SoundEvent event)
{
  stream << SoundTheme::sound_event_to_id(event);
  return stream;
}

#endif // SOUNDTHEME_HH
