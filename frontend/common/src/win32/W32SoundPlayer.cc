// W32SoundPlayer.cc --- Sound player
//
// Copyright (C) 2002 - 2007 Raymond Penners & Ray Satiro
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

#include <windows.h>
#include "W32SoundPlayer.hh"
#include "SoundPlayer.hh"
#include "Util.hh"

static struct SoundRegistry
{
  const char *event_label;
  const char *wav_file;
  const char *friendly_name;
} sound_registry[] =
{
  { "WorkraveBreakPrelude", "break-prelude.wav",
    "Break prompt" },
  { "WorkraveBreakIgnored", "break-ignored.wav",
    "Break ignored" },
  { "WorkraveRestBreakStarted", "rest-break-started.wav",
    "Rest break started" },
  { "WorkraveRestBreakEnded", "rest-break-ended.wav",
    "Rest break ended" },
  { "WorkraveMicroBreakStarted", "micro-break-started.wav",
    "Micro-break started" },
  { "WorkraveMicroBreakEnded", "micro-break-ended.wav",
    "Micro-break ended" },
  { "WorkraveDailyLimit", "daily-limit.wav",
    "Daily limit" },
  { "WorkraveExerciseEnded", "exercise-ended.wav",
    "Exercise ended" },
  { "WorkraveExercisesEnded", "exercises-ended.wav",
    "Exercises ended" },
};

static SoundRegistry *sound = NULL;


static bool
registry_get_value(const char *path, const char *name,
                   char *out)
{
  HKEY handle;
  bool rc = false;
  LONG err;

  err = RegOpenKeyEx(HKEY_CURRENT_USER, path, 0, KEY_ALL_ACCESS, &handle);
  if (err == ERROR_SUCCESS)
    {
      DWORD type, size;
      size = MAX_PATH;
      err = RegQueryValueEx(handle, name, 0, &type, (LPBYTE) out, &size);
      if (err == ERROR_SUCCESS)
        {
          rc = true;
        }
      RegCloseKey(handle);
    }
  return rc;
}

static bool
registry_set_value(const char *path, const char *name,
                   const char *value)
{
  HKEY handle;
  bool rc = false;
  DWORD disp;
  LONG err;

  err = RegCreateKeyEx(HKEY_CURRENT_USER, path, 0,
                       "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
                       NULL, &handle, &disp);
  if (err == ERROR_SUCCESS)
    {
      err = RegSetValueEx(handle, name, 0, REG_SZ, (BYTE *) value,
                          strlen(value)+1);
      RegCloseKey(handle);
      rc = (err == ERROR_SUCCESS);
    }
  return rc;
}



W32SoundPlayer::W32SoundPlayer()
{
  register_sound_events();
  sound = NULL;
}

W32SoundPlayer::~W32SoundPlayer()
{
}

void
W32SoundPlayer::register_sound_events()
{
  string sound_dir  = Util::get_application_directory() + "\\share\\sounds\\";

  for (unsigned int i = 0; i < sizeof(sound_registry)/sizeof(sound_registry[0]); i++)
    {
      SoundRegistry *snd = &sound_registry[i];
      char key[MAX_PATH], val[MAX_PATH];
      strcpy(key, "AppEvents\\EventLabels\\");
      strcat(key, snd->event_label);
      if (! registry_get_value(key, NULL, val))
        {
          registry_set_value(key, NULL, snd->friendly_name);
        }
      strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
      strcat(key, snd->event_label);
      strcat(key, "\\.default");
      if (! registry_get_value(key, NULL, val))
        {
          string file = sound_dir + snd->wav_file;
          registry_set_value(key, NULL, file.c_str());
          char *def = strrchr(key, '.');
          strcpy(def, ".current");
          registry_set_value(key, NULL, file.c_str());
        }
    }
}



/*
thread routine changed
jay satiro, workrave project, june 2007
redistribute under GNU terms.
*/

void W32SoundPlayer::play_sound( Sound snd )
{
  TRACE_ENTER_MSG( "W32SoundPlayer::play_sound", sound_registry[snd].friendly_name );

  if ( sound == &sound_registry[snd] )
    {
      TRACE_MSG( "Sound already queued: sound == &sound_registry[snd]" );
    }
  else if( sound == NULL )
    {
      DWORD id;
      sound = &sound_registry[snd];
      CloseHandle( CreateThread( NULL, 0, thread_Play, this, 0, &id ) );
    }
  else
    {
      TRACE_MSG( "Failed: sound != NULL && sound != &sound_registry[snd]" );
    }
  TRACE_EXIT();
}

DWORD WINAPI W32SoundPlayer::thread_Play( LPVOID lpParam )
{
  W32SoundPlayer *pThis = (W32SoundPlayer *) lpParam;
  pThis->Play();
  return (DWORD) 0;
}

void W32SoundPlayer::Play()
{
  TRACE_ENTER("W32SoundPlayer::Play");
  
  if( sound )
    {
      PlaySoundA( sound->event_label, 0, SND_APPLICATION | SND_ASYNC );
      sound = NULL;
    }
  
  TRACE_EXIT();
}
