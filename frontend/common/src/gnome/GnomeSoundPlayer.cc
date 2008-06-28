// GnomeSoundPlayer.cc --- Sound player
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2008 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include "GnomeSoundPlayer.hh"
#include "Sound.hh"
#include <debug.hh>

#include <libgnome/gnome-triggers.h>
#include <libgnome/gnome-sound.h>

GnomeSoundPlayer::GnomeSoundPlayer()
{
  gnome_sound_init(NULL);
  gnome_sound_sample_load("workrave/break_ignored",      DATADIR "/sounds/workrave/break-ignored.wav");
  gnome_sound_sample_load("workrave/break_prelude",      DATADIR "/sounds/workrave/break-prelude.wav");
  gnome_sound_sample_load("workrave/daily_limit",        DATADIR "/sounds/workrave/daily-limit.wav");
  gnome_sound_sample_load("workrave/exercise_step",      DATADIR "/sounds/workrave/exercise-step.wav");
  gnome_sound_sample_load("workrave/exercise_ended",     DATADIR "/sounds/workrave/exercise-ended.wav");
  gnome_sound_sample_load("workrave/exercises_ended",    DATADIR "/sounds/workrave/exercises-ended.wav");
  gnome_sound_sample_load("workrave/micro_break_ended",  DATADIR "/sounds/workrave/micro-break-ended.wav");
  gnome_sound_sample_load("workrave/micro_break_started",DATADIR "/sounds/workrave/micro-break-started.wav");
  gnome_sound_sample_load("workrave/rest_break_ended",   DATADIR "/sounds/workrave/rest-break-ended.wav");
  gnome_sound_sample_load("workrave/rest_break_started", DATADIR "/sounds/workrave/rest-break-started.wav");
}

GnomeSoundPlayer::~GnomeSoundPlayer()
{
  TRACE_ENTER("GnomeSoundPlayer::~GnomeSoundPlayer");
  // FIXME: check this one:
  gnome_sound_shutdown ();
  TRACE_EXIT();
}


bool
GnomeSoundPlayer::capability(SoundPlayer::SoundCapability cap)
{
  (void) cap;
  return false;
}

void
GnomeSoundPlayer::play_sound(std::string wavfile)
{
  (void) wavfile;
}


void
GnomeSoundPlayer::play_sound(SoundPlayer::SoundEvent snd)
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
    "exercises_ended",
    "exercise_step",
  };
  TRACE_ENTER_MSG("GnomeSoundPlayer::play_sound", map[snd]);
  gnome_triggers_do ("", NULL, "workrave", map[snd], NULL);
  TRACE_EXIT();
}


bool
GnomeSoundPlayer::get_sound_enabled(SoundPlayer::SoundEvent snd, bool &enabled)
{
  (void) snd;
  (void) enabled;
  
  return false;
}

void
GnomeSoundPlayer::set_sound_enabled(SoundPlayer::SoundEvent snd, bool enabled)
{
  (void) snd;
  (void) enabled;
}

bool
GnomeSoundPlayer::get_sound_wav_file(SoundPlayer::SoundEvent snd, std::string &wav_file)
{
  (void) snd;
  (void) wav_file;
  return false;
}

void
GnomeSoundPlayer::set_sound_wav_file(SoundPlayer::SoundEvent snd, const std::string &wav_file)
{
  (void) snd;
  (void) wav_file;
}
