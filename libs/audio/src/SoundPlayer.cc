// Copyright (C) 2002 - 2013 Rob Caelers & Raymond Penners
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

#include <climits>
#include <cstdlib>

#include "SoundPlayer.hh"
#include "ISoundDriver.hh"
#include "IMixer.hh"

#if defined(HAVE_GSTREAMER)
#  include "GstSoundPlayer.hh"
#elif defined(PLATFORM_OS_WINDOWS)
#  include <windows.h>
#  include "W32SoundPlayer.hh"
#  include "W32DirectSoundPlayer.hh"
#  include "W32Mixer.hh"
#elif defined(PLATFORM_OS_MACOS)
#  include "MacOSSoundPlayer.hh"
#endif

#if defined HAVE_PULSE
#  include "PulseMixer.hh"
#endif

using namespace workrave;
using namespace workrave::audio;
using namespace std;

ISoundPlayer::Ptr
SoundPlayerFactory::create()
{
  return std::make_shared<SoundPlayer>();
}

SoundPlayer::SoundPlayer()
{
  driver =
#if defined HAVE_GSTREAMER
    new GstSoundPlayer()
#elif defined PLATFORM_OS_WINDOWS
    new W32DirectSoundPlayer()
#elif defined PLATFORM_OS_MACOS
    new MacOSSoundPlayer()
#else
#  warning Sound card support disabled.
    NULL
#endif
    ;

  mixer =
#if defined HAVE_PULSE
    new PulseMixer()
#elif defined PLATFORM_OS_WINDOWS
    new W32Mixer()
#else
    nullptr
#endif
    ;
}

SoundPlayer::~SoundPlayer()
{
  delete driver;
  delete mixer;
}

void
SoundPlayer::init()
{
  if (driver != nullptr)
    {
      driver->init(this);
    }

  if (mixer != nullptr)
    {
      mixer->init();
    }
}

void
SoundPlayer::play_sound(const std::string &wavfile, bool mute_after_playback, int volume)
{
  TRACE_ENTRY_PAR(wavfile, mute_after_playback, volume);
  delayed_mute = false;

  if (mute_after_playback && mixer != nullptr && driver != nullptr && driver->capability(SoundCapability::EOS_EVENT))
    {
      delayed_mute = true;
    }

  if (driver != nullptr)
    {
      if (!wavfile.empty())
        {
          driver->play_sound(wavfile, volume);
        }
      else
        {
          delayed_mute = false;
        }
    }
}

bool
SoundPlayer::capability(SoundCapability cap)
{
  bool ret = false;

  if (mixer != nullptr && cap == SoundCapability::MUTE)
    {
      ret = true;
    }

  if (!ret && driver != nullptr)
    {
      ret = driver->capability(cap);
    }

  return ret;
}

void
SoundPlayer::restore_mute()
{
  if (mixer != nullptr && must_unmute)
    {
      mixer->set_mute(false);
    }
}

void
SoundPlayer::eos_event()
{
  if (delayed_mute && mixer != nullptr)
    {
      bool was_muted = mixer->set_mute(true);
      if (!was_muted)
        {
          must_unmute = true;
        }
    }
}
