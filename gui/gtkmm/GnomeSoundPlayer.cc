// GnomeSoundPlayer.cc --- Sound player
//
// Copyright (C) 2002 Rob Caelers & Raymond Penners
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

#include "GnomeSoundPlayer.hh"
#include "Sound.hh"
#include <debug.hh>

#include <libgnome/gnome-triggers.h>

GnomeSoundPlayer::GnomeSoundPlayer()
{
}

GnomeSoundPlayer::~GnomeSoundPlayer()
{
}



void
GnomeSoundPlayer::play_sound(Sound snd)
{
  char *map[] = {
    "break_prelude",
    "break_ignored",
    "restbreak_started",
    "restbreak_ended",
    "micropause_started",
    "micropause_ended",
    "daily_limit"
  };
  TRACE_ENTER_MSG("GnomeSoundPlayer::play_sound", map[snd]);
  gnome_triggers_do ("", NULL, "workrave", map[snd], NULL);
  TRACE_EXIT();
}

