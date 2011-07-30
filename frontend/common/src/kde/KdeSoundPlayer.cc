// KdeSoundPlayer.cc --- Sound player
//
// Copyright (C) 2002, 2003, 2004, 2007, 2008, 2010 Rob Caelers & Raymond Penners
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

#include <string>
using namespace std;

#include "KdeSoundPlayer.hh"
#include <debug.hh>

#include <dcopclient.h>
#include <kapp.h>
#include <knotifyclient.h>

KdeSoundPlayer::KdeSoundPlayer()
{
  kinstance = new KInstance("kworkrave");
}

KdeSoundPlayer::~KdeSoundPlayer()
{
  TRACE_ENTER("KdeSoundPlayer::~KdeSoundPlayer");
  delete kinstance;
  TRACE_EXIT();
}


bool
KdeSoundPlayer::capability(SoundCapability cap)
{
  (void) cap;
  return false;
}

void
KdeSoundPlayer::play_sound(string wavfile)
{
  (void) wavfile;
}

void
KdeSoundPlayer::play_sound(SoundEvent snd)
{
  const char *map[] = {
    "break_prelude",
    "break_ignored",
    "rest_break_started",
    "rest_break_ended",
    "micro_break_started",
    "micro_break_ended",
    "daily_limit",
    "exercise_ended",
    "exercises_ended"
    "exercise_step",
  };
  TRACE_ENTER_MSG("KdeSoundPlayer::play_sound", map[snd]);
  KNotifyClient::Instance instance(kinstance);
  int rc = KNotifyClient::event(map[snd]);
  TRACE_MSG(rc);
  TRACE_EXIT();
}


bool
KdeSoundPlayer::get_sound_enabled(SoundEvent snd, bool &enabled)
{
  (void) snd;
  (void) enabled;

  return false;
}

void
KdeSoundPlayer::set_sound_enabled(SoundEvent snd, bool enabled)
{
  (void) snd;
  (void) enabled;
}

bool
KdeSoundPlayer::get_sound_wav_file(SoundEvent snd, std::string &wav_file)
{
  (void) snd;
  (void) wav_file;
  return false;
}

void
KdeSoundPlayer::set_sound_wav_file(SoundEvent snd, const std::string &wav_file)
{
  (void) snd;
  (void) wav_file;
}
