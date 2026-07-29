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

#include <algorithm>
#include <Cocoa/Cocoa.h>
#include <CoreAudio/CoreAudio.h>
#import "Foundation/Foundation.h"

namespace
{
  constexpr auto default_device_id = "default";

  std::string cf_string_to_utf8(CFStringRef value)
  {
    if (value == nullptr)
      {
        return {};
      }

    const auto buffer_size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(value), kCFStringEncodingUTF8) + 1;
    if (buffer_size <= 0)
      {
        return {};
      }
    std::vector<char> buffer(static_cast<std::size_t>(buffer_size));
    if (!CFStringGetCString(value, buffer.data(), buffer_size, kCFStringEncodingUTF8))
      {
        return {};
      }
    return buffer.data();
  }

  std::string get_device_string(AudioDeviceID device_id, AudioObjectPropertySelector selector)
  {
    AudioObjectPropertyAddress address = {
      selector,
      kAudioObjectPropertyScopeGlobal,
      kAudioObjectPropertyElementMain,
    };
    CFStringRef value = nullptr;
    UInt32 size = sizeof(value);
    if (AudioObjectGetPropertyData(device_id, &address, 0, nullptr, &size, &value) != noErr || value == nullptr)
      {
        return {};
      }

    auto result = cf_string_to_utf8(value);
    CFRelease(value);
    return result;
  }

  bool has_output_channels(AudioDeviceID device_id)
  {
    AudioObjectPropertyAddress address = {
      kAudioDevicePropertyStreamConfiguration,
      kAudioDevicePropertyScopeOutput,
      kAudioObjectPropertyElementMain,
    };
    UInt32 size = 0;
    if (AudioObjectGetPropertyDataSize(device_id, &address, 0, nullptr, &size) != noErr || size == 0)
      {
        return false;
      }

    std::vector<unsigned char> storage(size);
    auto *buffers = reinterpret_cast<AudioBufferList *>(storage.data());
    if (AudioObjectGetPropertyData(device_id, &address, 0, nullptr, &size, buffers) != noErr)
      {
        return false;
      }

    for (UInt32 index = 0; index < buffers->mNumberBuffers; ++index)
      {
        if (buffers->mBuffers[index].mNumberChannels > 0)
          {
            return true;
          }
      }
    return false;
  }
} // namespace

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
  AudioObjectPropertyListenerBlock device_list_listener;

public:
  Private()
  {
    soundDictionary = [NSMutableDictionary dictionaryWithCapacity:10];
    delegate = [SoundDelegate alloc];
    device_list_listener = nil;
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

MacOSSoundPlayer::~MacOSSoundPlayer()
{
  if (priv->device_list_listener != nil)
    {
      AudioObjectPropertyAddress address = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain,
      };
      AudioObjectRemovePropertyListenerBlock(kAudioObjectSystemObject,
                                             &address,
                                             dispatch_get_main_queue(),
                                             priv->device_list_listener);
      priv->device_list_listener = nil;
    }
}

void
MacOSSoundPlayer::init(ISoundPlayerEvents *events)
{
  this->events = events;
  [priv->delegate setCallback:events];

  AudioObjectPropertyAddress address = {
    kAudioHardwarePropertyDevices,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMain,
  };
  priv->device_list_listener = ^(UInt32, const AudioObjectPropertyAddress *) {
    events->device_list_changed();
  };
  if (AudioObjectAddPropertyListenerBlock(kAudioObjectSystemObject,
                                          &address,
                                          dispatch_get_main_queue(),
                                          priv->device_list_listener)
      != noErr)
    {
      priv->device_list_listener = nil;
    }
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
  if (cap == workrave::audio::SoundCapability::DEVICE)
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
  bool use_default_device = current_device.empty() || current_device == default_device_id;
  if (!use_default_device)
    {
      const auto devices = get_devices();
      use_default_device = std::none_of(devices.begin(), devices.end(), [this](const auto &device) {
        return device.id == current_device;
      });
    }

  if (use_default_device)
    {
      [sound setPlaybackDeviceIdentifier:nil];
    }
  else
    {
      NSString *device_identifier = [NSString stringWithUTF8String:current_device.c_str()];
      [sound setPlaybackDeviceIdentifier:device_identifier];
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

std::vector<workrave::audio::SoundDevice>
MacOSSoundPlayer::get_devices()
{
  std::vector<workrave::audio::SoundDevice> result = {
    {default_device_id, "System Default", true},
  };

  AudioObjectPropertyAddress address = {
    kAudioHardwarePropertyDevices,
    kAudioObjectPropertyScopeGlobal,
    kAudioObjectPropertyElementMain,
  };
  UInt32 size = 0;
  if (AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &address, 0, nullptr, &size) != noErr)
    {
      return result;
    }

  std::vector<AudioDeviceID> devices(size / sizeof(AudioDeviceID));
  if (devices.empty()
      || AudioObjectGetPropertyData(kAudioObjectSystemObject, &address, 0, nullptr, &size, devices.data()) != noErr)
    {
      return result;
    }

  for (auto device_id: devices)
    {
      if (!has_output_channels(device_id))
        {
          continue;
        }

      auto uid = get_device_string(device_id, kAudioDevicePropertyDeviceUID);
      if (uid.empty())
        {
          continue;
        }

      auto name = get_device_string(device_id, kAudioObjectPropertyName);
      result.push_back({std::move(uid), name.empty() ? "Unnamed audio device" : std::move(name), false});
    }

  return result;
}

void
MacOSSoundPlayer::set_device(const std::string &device_id)
{
  current_device = device_id;
}

std::string
MacOSSoundPlayer::get_device() const
{
  return current_device;
}
