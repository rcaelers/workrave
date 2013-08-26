// OSXSoundPlayer.cc --- Sound player
//
// Copyright (C) 2002 - 2008, 2010, 2013 Raymond Penners & Ray Satiro
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"
#include <strings.h>

#include "OSXSoundPlayer.hh"
#include "audio/SoundPlayer.hh"
#include "Util.hh"

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

OSXSoundPlayer::OSXSoundPlayer()
{
}


OSXSoundPlayer::~OSXSoundPlayer()
{
}

bool
OSXSoundPlayer::capability(SoundCapability cap)
{
  if (cap == SOUND_CAP_EDIT)
    {
      return true;
    }
  return false;
}


void
OSXSoundPlayer::play_sound(SoundEvent snd)
{
  (void) snd;
}


void
OSXSoundPlayer::play_sound(string file)
{
  if (wav_file == NULL)
    {
      wav_file = strdup(file.c_str());
      start();
    }
}


void
OSXSoundPlayer::run()
{
}


bool OSXSoundPlayer::get_sound_enabled(SoundEvent snd, bool &enabled)
{
  (void) snd;
  (void) enabled;
  return false;
}

void OSXSoundPlayer::set_sound_enabled(SoundEvent snd, bool enabled)
{
  (void) snd;
  (void) enabled;
}

bool OSXSoundPlayer::get_sound_wav_file(SoundEvent snd, std::string &wav_file)
{
  (void) snd;
  (void) wav_file;
  return false;
}

void OSXSoundPlayer::set_sound_wav_file(SoundEvent snd, const std::string &wav_file)
{
  (void) snd;
  (void) wav_file;
}

