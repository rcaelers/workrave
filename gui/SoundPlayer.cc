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
#include "Thread.hh"
#include "Sound.hh"

#ifdef HAVE_GNOME
#include <gdk/gdk.h>
#endif
#ifdef HAVE_X
#include <X11/Xlib.h>
#endif

const char *SoundPlayer::CFG_KEY_SOUND_ENABLED = "sound/enabled";
const char *SoundPlayer::CFG_KEY_SOUND_DEVICE = "sound/device";

/**********************************************************************
 * PC-Speaker
 **********************************************************************/
static short prelude_beeps[][2]=
{
    { 250, 50},
    { 300, 50},
    { 0, 0 }
};
    
static short micro_pause_start_beeps[][2]=
{
    { 320, 70 },
    { 350, 70 },
    { 0, 0 },
};

static short micro_pause_end_beeps[][2]=
{
  { 350, 70 },
  { 320, 70 },
  { 0, 0 },
};

static short rest_break_start_beeps[][2]=
{
  { 160, 70 }, 
  { 180, 70 }, 
  { 200, 70 }, 
  { 230, 70 }, 
  { 260, 70 }, 
  { 290, 70 }, 
  { 320, 70 }, 
  { 350, 70 },
  { 0, 0 }
};

static short rest_break_end_beeps[][2]=
{
  { 350, 70 }, 
  { 320, 70 }, 
  { 290, 70 }, 
  { 260, 70 }, 
  { 230, 70 }, 
  { 200, 70 }, 
  { 180, 70 }, 
  { 160, 70 }, 
  { 0, 0 }
};

static short break_ignore_beeps[][2]=
{
    { 60, 250 }, 
    { 50, 400 },
    { 0, 0 }
};

static short (*beep_map[])[2] =
{
  prelude_beeps,
  break_ignore_beeps,
  rest_break_start_beeps,
  rest_break_end_beeps,
  micro_pause_start_beeps,
  micro_pause_end_beeps
};


class SpeakerPlayer : public Thread
{
public:
  SpeakerPlayer(short (*beeps)[2]);
  void run();
private:
  SoundPlayer *player;
  short (*beeps)[2];
};


SpeakerPlayer::SpeakerPlayer(short (*b)[2])
  : Thread(true)
{
  beeps = b;
}

void
SpeakerPlayer::run()
{
  short (*b)[2];
  b = beeps;
#ifdef HAVE_X
  Display *display = NULL;
#  ifdef HAVE_GNOME
  display = XOpenDisplay(gdk_get_display());
#  endif
  if (display) {
#endif  
  while (b[0][0])
    {
#ifdef HAVE_X
      ::Sound::beep(display, b[0][0], b[0][1]);
#else
      ::Sound::beep(b[0][0], b[0][1]);
#endif      
      b++;
    }
#ifdef HAVE_X
    XCloseDisplay(display);
  }
#endif
  
}
  

/**********************************************************************
 * SoundPlayer
 **********************************************************************/


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
 
  if (is_enabled())
    {
      if (get_device() == DEVICE_SOUNDCARD && player != NULL)
        {
          player->play_sound(snd);
        }
      else
        {
          Thread *t = new SpeakerPlayer(beep_map[snd]);
          t->start();
        }
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
