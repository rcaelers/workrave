// OSXSoundPlayer.cc --- Sound player
//
// Copyright (C) 2002 - 2008, 2010 Raymond Penners & Ray Satiro
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
#include "SoundPlayer.hh"

#include <Cocoa/Cocoa.h>
#include <QTKit/QTKit.h>

OSXSoundPlayer::OSXSoundPlayer()
{
  soundDictionary = [NSMutableDictionary dictionaryWithCapacity:10];
}


OSXSoundPlayer::~OSXSoundPlayer()
{
  [soundDictionary removeAllObjects];
  [soundDictionary release];
}

bool
OSXSoundPlayer::capability(workrave::audio::SoundCapability cap)
{
  if (cap == workrave::audio::SOUND_CAP_EDIT)
    {
      return true;
    }
  return false;
}


void
OSXSoundPlayer::play_sound(workrave::audio::SoundEvent snd, int volume)
{
  (void) snd;
}


void
OSXSoundPlayer::play_sound(std::string file, int volume)
{
  NSString* filename = [NSString stringWithUTF8String: file.c_str()];
  NSSound *sound = [soundDictionary objectForKey:filename];
  if (sound == nil) 
    {
      sound = [[NSSound alloc] initWithContentsOfFile:filename byReference:NO];
      [soundDictionary setObject:sound forKey:filename];
    }
  [sound stop];
  [sound play];
}
