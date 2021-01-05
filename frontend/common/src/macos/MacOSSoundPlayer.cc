// MacOSSoundPlayer.cc --- Sound player
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
#  include "config.h"
#endif

#include "debug.hh"
#include <strings.h>

#include "MacOSSoundPlayer.hh"
#include "SoundPlayer.hh"
#include "Util.hh"
#import <AppKit/AppKit.h>

MacOSSoundPlayer::MacOSSoundPlayer()
{
  soundDictionary = [NSMutableDictionary dictionaryWithCapacity:10];
}

MacOSSoundPlayer::~MacOSSoundPlayer()
{
  [soundDictionary removeAllObjects];
  [soundDictionary release];
}

bool
MacOSSoundPlayer::capability(SoundCapability cap)
{
  if (cap == SOUND_CAP_EDIT)
    {
      return true;
    }
  return false;
}

void
MacOSSoundPlayer::play_sound(SoundEvent snd)
{
  (void)snd;
}

void
MacOSSoundPlayer::play_sound(string file)
{
  if (wav_file == NULL)
    {
      wav_file = strdup(file.c_str());
      run();
    }
}

void
MacOSSoundPlayer::run()
{
  NSString *filename = [NSString stringWithUTF8String:wav_file];
  NSSound *sound     = [soundDictionary objectForKey:filename];
  if (sound == nil)
    {
      sound = [[NSSound alloc] initWithContentsOfFile:filename byReference:NO];
      [soundDictionary setObject:sound forKey:filename];
    }
  [sound stop];
  [sound play];
  free((void *)wav_file);
  wav_file = NULL;
}

bool
MacOSSoundPlayer::get_sound_enabled(SoundEvent snd, bool &enabled)
{
  (void)snd;
  (void)enabled;
  return false;
}

void
MacOSSoundPlayer::set_sound_enabled(SoundEvent snd, bool enabled)
{
  (void)snd;
  (void)enabled;
}

bool
MacOSSoundPlayer::get_sound_wav_file(SoundEvent snd, std::string &wav_file)
{
  (void)snd;
  (void)wav_file;
  return false;
}

void
MacOSSoundPlayer::set_sound_wav_file(SoundEvent snd, const std::string &wav_file)
{
  (void)snd;
  (void)wav_file;
}
