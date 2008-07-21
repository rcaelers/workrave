// OSXSoundPlayer.hh
//
// Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.nl>
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
// $Id: OSXSoundPlayer.hh 1230 2007-07-03 15:59:16Z rcaelers $
//

#ifndef OSXSOUNDPLAYER_HH
#define OSXSOUNDPLAYER_HH

#include "ISoundDriver.hh"
#include "Thread.hh"

class OSXSoundPlayer : public ISoundDriver, public Thread
{
public:
  OSXSoundPlayer();
  virtual ~OSXSoundPlayer();

  bool capability(SoundPlayer::SoundCapability cap);
  void play_sound(std::string wavfile);
  void play_sound(SoundPlayer::SoundEvent snd);

  bool get_sound_enabled(SoundPlayer::SoundEvent snd, bool &enabled);
  void set_sound_enabled(SoundPlayer::SoundEvent snd, bool enabled);
  bool get_sound_wav_file(SoundPlayer::SoundEvent snd, std::string &wav_file);
  void set_sound_wav_file(SoundPlayer::SoundEvent snd, const std::string &wav_file);
 
private:

  void run();
  
  const char *wav_file;
};

#endif // OSXSOUNDPLAYER_HH
