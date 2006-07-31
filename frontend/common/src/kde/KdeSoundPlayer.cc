// KdeSoundPlayer.cc --- Sound player
//
// Copyright (C) 2002, 2003, 2004 Rob Caelers & Raymond Penners
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


void
KdeSoundPlayer::play_sound(Sound snd)
{
  char *map[] = {
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
  TRACE_ENTER_MSG("KdeSoundPlayer::play_sound", map[snd]);
  KNotifyClient::Instance instance(kinstance);
  int rc = KNotifyClient::event(map[snd]);
  TRACE_MSG(rc);
  TRACE_EXIT();
}

