// Win32SoundPlayer.cc --- Sound player
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "SoundPlayer.hh"
#include "GUIControl.hh"
#include "Configurator.hh"

const char *SoundPlayer::CFG_KEY_SOUND_ENABLED = "sound/enabled";
const char *SoundPlayer::CFG_KEY_SOUND_DEVICE = "sound/device";


SoundPlayer::SoundPlayer(SoundPlayerInterface *p)
{
  player = p;
}

SoundPlayer::~SoundPlayer()
{
  delete player;
}

void
SoundPlayer::play_sound(Sound snd)
{
  if (is_enabled() && player != NULL)
    {
      player->play_sound(snd);
    }
}


bool
SoundPlayer::is_enabled() 
{
  bool b;
  bool rc;
  b = GUIControl::get_instance()->get_configurator()
    ->get_value(CFG_KEY_SOUND_ENABLED, &rc);
  if (! b)
    {
      rc = true;
    }
  return rc;
}

void
SoundPlayer::set_enabled(bool b)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_SOUND_ENABLED, b);
}

SoundPlayer::Device
SoundPlayer::get_device() 
{
  bool b;
  Device dev = DEVICE_SOUNDCARD;
  string devstr;
  b = GUIControl::get_instance()->get_configurator()
    ->get_value(CFG_KEY_SOUND_DEVICE, &devstr);
  if (b)
    {
      if (devstr == "speaker")
        {
          dev = DEVICE_SPEAKER;
        }
    }
  return dev;
}

void
SoundPlayer::set_device(Device dev)
{
  const char *devstr;
  switch (dev)
    {
    case DEVICE_SPEAKER:
      devstr = "speaker";
      break;
    default:
      devstr = "soundcard";
      break;
    }
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_SOUND_DEVICE, string(devstr));
}
