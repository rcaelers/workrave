// GnomeSoundPlayer.cc --- Sound player
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
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
  gnome_sound_sample_load("workrave/exercise_ended",     DATADIR "/sounds/workrave/exercise-ended.wav");
  gnome_sound_sample_load("workrave/break_ignored",      DATADIR "/sounds/workrave/break-ignored.wav");
  gnome_sound_sample_load("workrave/break_prelude",      DATADIR "/sounds/workrave/break-prelude.wav");
  gnome_sound_sample_load("workrave/daily_limit",        DATADIR "/sounds/workrave/daily-limit.wav");
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



void
GnomeSoundPlayer::play_sound(Sound snd)
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
  };
  TRACE_ENTER_MSG("GnomeSoundPlayer::play_sound", map[snd]);
  gnome_triggers_do ("", NULL, "workrave", map[snd], NULL);
  TRACE_EXIT();
}

