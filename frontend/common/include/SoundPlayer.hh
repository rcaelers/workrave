// SoundPlayer.hh
//
// Copyright (C) 2002, 2003 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef SOUNDPLAYER_HH
#define SOUNDPLAYER_HH

#include "SoundPlayerInterface.hh"

class SoundPlayer : public SoundPlayerInterface
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

  SoundPlayerInterface *player;
};

#endif // SOUNDPLAYER_HH
