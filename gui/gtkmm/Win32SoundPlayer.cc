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

#include <windows.h>
#include "Win32SoundPlayer.hh"

static int prelude_beeps[][2]=
{
    { 400, 50},
    { 450, 50},
    { 500, 50},
    { 550, 50},
    { 0, 0 }
};
    
static int break_start_beeps[][2]=
{
    { 450, 150},
    { 550, 200},
    { 0, 0 },
};

static int break_end_beeps[][2]=
{
    { 550, 50},
    { 500, 50},
    { 450, 50},
    { 400, 50},
    { 0, 0 },
};

static int break_ignore_beeps[][2]=
{
    { 60, 250 }, 
    { 50, 400 },
    { 0, 0 }
};

void
Win32SoundPlayer::destroy()
{
  delete this;
}


void
Win32SoundPlayer::play_speaker(int (*beeps)[2])
{
  while (beeps[0][0])
    {
      Beep(beeps[0][0], beeps[0][1]);
      beeps++;
    }
}

void
Win32SoundPlayer::play_sound(Sound snd)
{
  switch (snd)
    {
    case SOUND_BREAK_PRELUDE:
      play_speaker(prelude_beeps);
      break;
    case SOUND_BREAK_IGNORED:
      play_speaker(break_ignore_beeps);
      break;
    case SOUND_BREAK_STARTED:
      play_speaker(break_start_beeps);
      break;
    case SOUND_BREAK_ENDED:
      play_speaker(break_end_beeps);
      break;
    }
}

