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

@interface SoundDelegate : NSObject<NSSoundDelegate>
{
  OSXSoundPlayer *m_player;
}

- (id) initWithPlayer:(OSXSoundPlayer*)player;
- (void)sound: (NSSound*)sound didFinishPlaying: (BOOL)finishedPlaying;
@end

@implementation SoundDelegate : NSObject

- (id) initWithPlayer:(OSXSoundPlayer*)player
{
  self = [super init];
  if (self) {
    m_player = player;
  }
  return self;
}

- (void) sound: (NSSound *) sound didFinishPlaying: (BOOL) aBool
{
  m_player->fire_eos();
}
@end

OSXSoundPlayer::OSXSoundPlayer()
{
  soundDictionary = [NSMutableDictionary dictionaryWithCapacity:10];
}


OSXSoundPlayer::~OSXSoundPlayer()
{
  [soundDictionary removeAllObjects];
  [soundDictionary release];
}

void
OSXSoundPlayer::init(ISoundPlayerEvents *events)
{
  this->events = events;
}


bool
OSXSoundPlayer::capability(workrave::audio::SoundCapability cap)
{
  if (cap == workrave::audio::SOUND_CAP_VOLUME)
    {
      return true;
    }
  if (cap == workrave::audio::SOUND_CAP_EOS_EVENT)
    {
      return true;
    }
  return false;
}


void
OSXSoundPlayer::play_sound(std::string file, int volume)
{
  NSString* filename = [NSString stringWithUTF8String: file.c_str()];
  NSSound *sound = [soundDictionary objectForKey:filename];
  if (sound == nil)
    {
      sound = [[NSSound alloc] initWithContentsOfFile:filename byReference:NO];
      [sound setDelegate: [[SoundDelegate alloc] initWithPlayer: this]]; // FIXME: leak?
      [soundDictionary setObject:sound forKey:filename];
    }
  [sound setVolume: (float)(volume / 100.0)];
  [sound stop];
  [sound play];
}

void
OSXSoundPlayer::fire_eos()
{
  events->eos_event();
}
