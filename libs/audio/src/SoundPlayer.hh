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

#ifndef SOUNDPLAYER_HH
#define SOUNDPLAYER_HH

#include <string>
#include <list>
#include <vector>
#include <map>

#include "config/IConfigurator.hh"

#include "ISoundDriver.hh"

class IMixer;

class SoundPlayer : public ISoundPlayer, public ISoundPlayerEvents
{
public:
  SoundPlayer(workrave::config::IConfigurator::Ptr config);
  virtual ~SoundPlayer();
  void play_sound(SoundEvent snd, bool mute_after_playback = false);
  void play_sound(std::string wavfile);

  bool is_enabled();
  void set_enabled(bool enabled);

  void init();
  bool capability(SoundCapability cap);
  void restore_mute();

  bool get_sound_enabled(SoundEvent snd, bool &enabled);
  void set_sound_enabled(SoundEvent snd, bool enabled);
  bool get_sound_wav_file(SoundEvent snd, std::string &filename);
  void set_sound_wav_file(SoundEvent snd, const std::string &wav_file);

  void get_sound_themes(std::vector<Theme> &themes);
  void load_sound_theme(const std::string &path, Theme &theme);
  void activate_theme(const Theme &theme, bool force = true);
  void sync_settings();

  void eos_event();

private:
  void register_sound_events(std::string theme = "");

public:
  static SoundRegistry sound_registry[SOUND_MAX];

private:
  workrave::config::IConfigurator::Ptr config;
  ISoundDriver *driver;
  IMixer *mixer;
  bool delayed_mute;
  bool must_unmute;
};

#endif // SOUNDPLAYER_HH
