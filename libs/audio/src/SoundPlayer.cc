// SoundPlayer.cc --- Sound player
//
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
#include "config.h"
#endif

#include "debug.hh"
#include "nls.h"

#ifdef HAVE_REALPATH
#include <limits.h>
#include <stdlib.h>
#endif

#include "SoundPlayer.hh"
#include "ISoundDriver.hh"
#include "IMixer.hh"

//#include "utils/Platform.hh"

#include "Util.hh"

#if defined HAVE_GSTREAMER
#include "GstSoundPlayer.hh"
#elif defined PLATFORM_OS_UNIX
#include <X11/Xlib.h>
#elif defined PLATFORM_OS_WIN32
#include <windows.h>
#include "W32SoundPlayer.hh"
#include "W32DirectSoundPlayer.hh"
#include "W32Mixer.hh"
#elif defined PLATFORM_OS_OSX
#include "OSXSoundPlayer.hh"
#endif

#if defined HAVE_PULSE
#include "PulseMixer.hh"
#endif

#ifdef HAVE_GTK
#include <gdk/gdk.h>
#endif

using namespace workrave;
using namespace workrave::audio;
using namespace std;

/**********************************************************************
 * SoundPlayer
 **********************************************************************/

ISoundPlayer::Ptr
ISoundPlayer::create()
{
  return ISoundPlayer::Ptr();
}

SoundPlayer::SoundPlayer()
{
  driver =
#if defined HAVE_GSTREAMER
     new GstSoundPlayer()
#elif defined PLATFORM_OS_WIN32
     new W32DirectSoundPlayer()
#elif defined PLATFORM_OS_OSX
     new OSXSoundPlayer()
#else
#  warning Sound card support disabled.
     NULL
#endif
    ;

  mixer =
#if defined HAVE_PULSE
    new PulseMixer()
#elif defined PLATFORM_OS_WIN32
    new W32Mixer()
#else
    NULL
#endif
    ;

  must_unmute = false;
  delayed_mute = false;
}

SoundPlayer::~SoundPlayer()
{
  delete driver;
}


void
SoundPlayer::init()
{
  if (driver != NULL)
    {
      driver->init(this);
    }

  if (mixer != NULL)
    {
      mixer->init();
    }
}

void
SoundPlayer::play_sound(SoundEvent snd, const std::string &wavfile, bool mute_after_playback, int volume)
{
  TRACE_ENTER_MSG("SoundPlayer::play_sound ", snd << " " << mute_after_playback);
  if (snd >= SOUND_MIN && snd < SOUND_MAX)
    {
      delayed_mute = false;
      if (mute_after_playback &&
          mixer != NULL && driver != NULL &&
          driver->capability(SOUND_CAP_EOS_EVENT))
        {
          delayed_mute = true;
        }

      if (driver != NULL)
        {
          if (driver->capability(SOUND_CAP_EVENTS))
            {
              driver->play_sound(snd, volume);
            }
          else
            {
              if (wavfile != "")
                {
                  driver->play_sound(wavfile, volume);
                }
              else
                {
                  delayed_mute = false;
                }
            }
        }
    }

  TRACE_EXIT();
}


void
SoundPlayer::play_sound(const std::string &wavfile, int volume)
{
  TRACE_ENTER("SoundPlayer::play_sound");
  if (driver != NULL)
    {
      driver->play_sound(wavfile, volume);
    }

  TRACE_EXIT();
}


bool
SoundPlayer::capability(SoundCapability cap)
{
  bool ret = false;

  if (mixer != NULL && cap == SOUND_CAP_MUTE)
    {
      ret = true;
    }

  if (!ret && driver != NULL)
    {
      ret = driver->capability(cap);
    }

  return ret;
}

void
SoundPlayer::restore_mute()
{
  TRACE_ENTER("SoundPlayer::restore_mute");

  if (mixer != NULL && must_unmute)
    {
      mixer->set_mute(false);
    }

  TRACE_EXIT();
}


void
SoundPlayer::eos_event()
{
  TRACE_ENTER("SoundPlayer::eos_event");
  if (delayed_mute && mixer != NULL)
    {
      TRACE_MSG("delayed muting");
      bool was_muted = mixer->set_mute(true);
      if (!was_muted)
        {
          must_unmute = true;
        }
    }
  TRACE_EXIT();
}
