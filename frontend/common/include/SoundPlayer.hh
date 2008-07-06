// SoundPlayer.hh
//
// Copyright (C) 2002, 2003, 2006, 2007, 2008 Rob Caelers & Raymond Penners
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
// $Id$
//

#ifndef SOUNDPLAYER_HH
#define SOUNDPLAYER_HH

#include <string>
#include <list>
#include <vector>
#include <map>

class ISoundDriver;

class SoundPlayer
{
public:
  enum Device
    {
      DEVICE_SPEAKER = 0,
      DEVICE_SOUNDCARD
    };

  enum SoundEvent
    {
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
      SOUND_EXERCISE_MAX
    };

  enum SoundCapability
    {
      SOUND_CAP_EVENTS = 0,
      SOUND_CAP_EDIT,
    };


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
  
  SoundPlayer();
  virtual ~SoundPlayer();
  void play_sound(SoundEvent snd);
  void play_sound(std::string wavfile);

  static bool is_enabled();
  static void set_enabled(bool enabled);
  static Device get_device();
  static void set_device(Device dev);

  bool capability(SoundCapability cap);
  
  bool get_sound_enabled(SoundEvent snd, bool &enabled);
  void set_sound_enabled(SoundEvent snd, bool enabled);
  bool get_sound_wav_file(SoundEvent snd, std::string &filename);
  void set_sound_wav_file(SoundEvent snd, const std::string &wav_file);
  
  void get_sound_themes(std::vector<Theme> &themes);
  void load_sound_theme(const std::string &path, Theme &theme);
  void activate_theme(const Theme &theme, bool force = true);
                      
private:
  void register_sound_events(std::string theme = "");
  void sync_settings();
  
public:
  static const char *CFG_KEY_SOUND_ENABLED;
  static const char *CFG_KEY_SOUND_DEVICE;
  static const char *CFG_KEY_SOUND_VOLUME;
  static const char *CFG_KEY_SOUND_EVENTS;
  static const char *CFG_KEY_SOUND_EVENTS_ENABLED;

  static SoundRegistry sound_registry[SOUND_EXERCISE_MAX];
  
private:  
  ISoundDriver *driver;
};

#endif // SOUNDPLAYER_HH
