// Win32SoundPlayer.hh
//
// Copyright (C) 2002 Rob Caelers & Raymond Penners
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

#ifndef WIN32SOUNDPLAYER_HH
#define WIN32SOUNDPLAYER_HH

#include <SoundPlayerInterface.hh>

class Win32SoundPlayer : public SoundPlayerInterface
{
public:
  void play_sound(Sound snd);
  void destroy();
private:
  void play_speaker(int (*beeps)[2]);
};

#endif // WIN32SOUNDPLAYER_HH
