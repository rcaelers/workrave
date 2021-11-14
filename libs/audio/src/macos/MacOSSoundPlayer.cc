// Copyright (C) 2007 - 2013 Rob Caelers <robc@krandor.nl>
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

#include <Cocoa/Cocoa.h>
#import "Foundation/Foundation.h"

@interface SoundDelegate : NSObject <NSSoundDelegate>
{
  ISoundPlayerEvents *callback;
}

- (void)setCallback:(ISoundPlayerEvents *)callback;
- (void)sound:(NSSound *)sound didFinishPlaying:(BOOL)finishedPlaying;
@end

@implementation SoundDelegate : NSObject

- (void)setCallback:(ISoundPlayerEvents *)aCallback;
{
  self->callback = aCallback;
}

- (void)sound:(NSSound *)sound didFinishPlaying:(BOOL)finishedPlaying
{
  callback->eos_event();
}
@end

class MacOSSoundPlayer::Private
{
public:
  NSMutableDictionary *soundDictionary;
  SoundDelegate *delegate;

public:
  Private()
  {
    soundDictionary = [NSMutableDictionary dictionaryWithCapacity:10];
    delegate = [SoundDelegate alloc];
  }

  ~Private()
  {
    [soundDictionary removeAllObjects];
  }
};

MacOSSoundPlayer::MacOSSoundPlayer()
{
  priv = std::make_shared<Private>();
}

void
MacOSSoundPlayer::init(ISoundPlayerEvents *events)
{
  this->events = events;
  [priv->delegate setCallback:events];
}

bool
MacOSSoundPlayer::capability(workrave::audio::SoundCapability cap)
{
  if (cap == workrave::audio::SoundCapability::VOLUME)
    {
      return true;
    }
  if (cap == workrave::audio::SoundCapability::EOS_EVENT)
    {
      return true;
    }
  return false;
}

void
MacOSSoundPlayer::play_sound(std::string file, int volume)
{
  NSString *filename = [NSString stringWithUTF8String:file.c_str()];
  NSSound *sound = [priv->soundDictionary objectForKey:filename];
  if (sound == nil)
    {
      sound = [[NSSound alloc] initWithContentsOfFile:filename byReference:NO];
      [sound setDelegate:priv->delegate];
      [priv->soundDictionary setObject:sound forKey:filename];
    }
  [sound setVolume:static_cast<float>(volume / 100.0)];
  [sound stop];
  [sound play];
}

void
MacOSSoundPlayer::fire_eos()
{
  events->eos_event();
}
