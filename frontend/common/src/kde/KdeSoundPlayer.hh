// KdeSoundPlayer.hh
//
// Copyright (C) 2002, 2004 Rob Caelers & Raymond Penners
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

#ifndef KDESOUNDPLAYER_HH
#define KDESOUNDPLAYER_HH

#include <SoundPlayerInterface.hh>

class KInstance;

class KdeSoundPlayer : public SoundPlayerInterface
{
public:
  KdeSoundPlayer();
  virtual ~KdeSoundPlayer();
  void play_sound(Sound snd);

private:
  KInstance *kinstance;
};

#endif // KDESOUNDPLAYER_HH
