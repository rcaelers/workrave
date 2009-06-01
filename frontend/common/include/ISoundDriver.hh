// ISoundDriver.hh
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008 Rob Caelers & Raymond Penners
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


#ifndef ISOUNDDRIVER_HH
#define ISOUNDDRIVER_HH

#include <string>
#include "SoundPlayer.hh"

class ISoundDriver
{
public:
  virtual ~ISoundDriver() {}

  //! 
  virtual bool capability(SoundPlayer::SoundCapability cap) = 0;

  virtual bool get_sound_enabled(SoundPlayer::SoundEvent snd, bool &enabled) = 0;
  virtual void set_sound_enabled(SoundPlayer::SoundEvent snd, bool enabled) = 0;
  virtual bool get_sound_wav_file(SoundPlayer::SoundEvent snd, std::string &filename) = 0;
  virtual void set_sound_wav_file(SoundPlayer::SoundEvent snd, const std::string &wav_file) = 0;
  
  //! Plays sound, returns immediately.
  virtual void play_sound(SoundPlayer::SoundEvent snd) = 0;

  //! Plays sound, returns immediately.
  virtual void play_sound(std::string wavfile) = 0;
};

#endif // ISOUNDPLAYER_HH
