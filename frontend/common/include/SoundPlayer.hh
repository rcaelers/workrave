// SoundPlayer.hh
//
// Copyright (C) 2002, 2003, 2006, 2007 Rob Caelers & Raymond Penners
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

#include "ISoundPlayer.hh"

class SoundPlayer : public ISoundPlayer
{
public:
  enum Device
  {
    DEVICE_SPEAKER = 0,
    DEVICE_SOUNDCARD
  };

  SoundPlayer();
  virtual ~SoundPlayer();
  void play_sound(Sound snd);

  static bool is_enabled();
  static void set_enabled(bool enabled);
  static Device get_device();
  static void set_device(Device dev);

private:
  static const char *CFG_KEY_SOUND_ENABLED;
  static const char *CFG_KEY_SOUND_DEVICE;

  ISoundPlayer *player;
};

#endif // SOUNDPLAYER_HH
