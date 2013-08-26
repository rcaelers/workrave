// SoundPlayer.hh
//
// Copyright (C) 2002, 2003, 2006, 2007, 2008, 2009, 2010, 2011, 2013 Rob Caelers & Raymond Penners
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

#ifndef ISOUNDPLAYER_HH
#define ISOUNDPLAYER_HH

#include <string>
#include <list>
#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>

#include "config/IConfigurator.hh"

enum SoundCapability
  {
    SOUND_CAP_EVENTS = 0,
    SOUND_CAP_EDIT,
    SOUND_CAP_VOLUME,
    SOUND_CAP_MUTE,
    SOUND_CAP_EOS_EVENT,
  };

enum SoundEvent
  {
    SOUND_MIN = 0,
    SOUND_BREAK_PRELUDE = 0,
    SOUND_BREAK_IGNORED,
    SOUND_REST_BREAK_STARTED,
    SOUND_REST_BREAK_ENDED,
    SOUND_MICRO_BREAK_STARTED,
    SOUND_MICRO_BREAK_ENDED,
    SOUND_DAILY_LIMIT,
    SOUND_EXERCISE_ENDED,
    SOUND_EXERCISES_ENDED,
    SOUND_EXERCISE_STEP,
    SOUND_MAX
  };

class ISoundPlayer
{
public:
  class Theme
  {
  public:
    std::string description;
    std::vector<std::string> files;
    bool active;
  };

  struct SoundRegistry
  {
    const char *label;
    const char *id;
    const char *wav_file;
    const char *friendly_name;
  };

  typedef boost::shared_ptr<ISoundPlayer> Ptr;

  // FIXME: move config to frontend/common
  static Ptr create(workrave::config::IConfigurator::Ptr config);

  virtual ~ISoundPlayer() {};

  virtual void play_sound(SoundEvent snd, bool mute_after_playback = false) = 0;
  virtual void play_sound(std::string wavfile) = 0;

  virtual bool is_enabled() = 0;
  virtual void set_enabled(bool enabled) = 0;

  virtual void init() = 0;
  virtual bool capability(SoundCapability cap) = 0;
  virtual void restore_mute() = 0;

  virtual bool get_sound_enabled(SoundEvent snd, bool &enabled) = 0;
  virtual void set_sound_enabled(SoundEvent snd, bool enabled) = 0;
  virtual bool get_sound_wav_file(SoundEvent snd, std::string &filename) = 0;
  virtual void set_sound_wav_file(SoundEvent snd, const std::string &wav_file) = 0;

  virtual void get_sound_themes(std::vector<Theme> &themes) = 0;
  virtual void load_sound_theme(const std::string &path, Theme &theme) = 0;
  virtual void activate_theme(const Theme &theme, bool force = true) = 0;
  virtual void sync_settings() = 0;

public:
  static const char *CFG_KEY_SOUND_ENABLED;
  static const char *CFG_KEY_SOUND_DEVICE;
  static const char *CFG_KEY_SOUND_VOLUME;
  static const char *CFG_KEY_SOUND_EVENTS;
  static const char *CFG_KEY_SOUND_EVENTS_ENABLED;
  static const char *CFG_KEY_SOUND_MUTE;
};

#endif // ISOUNDPLAYER_HH
