// Win32SoundPlayer.cc --- Sound player
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

#include <windows.h>
#include "Win32SoundPlayer.hh"
#include "SoundPlayer.hh"
#include "Util.hh"

static short prelude_beeps[][2]=
{
    { 250, 50},
    { 300, 50},
    { 0, 0 }
};
    
static short micro_pause_start_beeps[][2]=
{
    { 320, 70 },
    { 350, 70 },
    { 0, 0 },
};

static short micro_pause_end_beeps[][2]=
{
  { 350, 70 },
  { 320, 70 },
  { 0, 0 },
};

static short rest_break_start_beeps[][2]=
{
  { 160, 70 }, 
  { 180, 70 }, 
  { 200, 70 }, 
  { 230, 70 }, 
  { 260, 70 }, 
  { 290, 70 }, 
  { 320, 70 }, 
  { 350, 70 },
  { 0, 0 }
};

static short rest_break_end_beeps[][2]=
{
  { 350, 70 }, 
  { 320, 70 }, 
  { 290, 70 }, 
  { 260, 70 }, 
  { 230, 70 }, 
  { 200, 70 }, 
  { 180, 70 }, 
  { 160, 70 }, 
  { 0, 0 }
};

static short break_ignore_beeps[][2]=
{
    { 60, 250 }, 
    { 50, 400 },
    { 0, 0 }
};

volatile HANDLE Win32SoundPlayer::thread_handle = NULL;

static struct SoundRegistry 
{
  const char *event_label;
  const char *wav_file;
  const char *friendly_name;
  short (*beeps)[2];
} sound_registry[] =
{
  { "WorkraveBreakPrelude", "break-prelude.wav",
    "Break prompt", prelude_beeps },
  { "WorkraveBreakIgnored", "break-ignored.wav",
    "Break ignored", break_ignore_beeps },
  { "WorkraveRestBreakStarted", "restbreak-started.wav",
    "Rest break started", rest_break_start_beeps },
  { "WorkraveRestBreakEnded", "restbreak-ended.wav",
    "Rest break ended", rest_break_end_beeps },
  { "WorkraveMicroPauseStarted", "micropause-started.wav",
    "Micro-pause started", micro_pause_start_beeps },
  { "WorkraveMicroPauseEnded", "micropause-ended.wav",
    "Micro-pause ended", micro_pause_end_beeps },
};

static bool
registry_get_value(const char *path, const char *name,
                   char *out) 
{
  HKEY handle;
  bool rc = false;
  DWORD disp;
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



Win32SoundPlayer::Win32SoundPlayer()
{
  register_sound_events();
}

Win32SoundPlayer::~Win32SoundPlayer()
{
}


void
Win32SoundPlayer::register_sound_events()
{
  string sound_dir  = Util::get_application_directory() + "\\share\\sounds\\";
  
  for (int i = 0; i < sizeof(sound_registry)/sizeof(sound_registry[0]); i++)
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



void
Win32SoundPlayer::play_speaker(short (*beeps)[2])
{
  while (beeps[0][0])
    {
      Beep(beeps[0][0], beeps[0][1]);
      beeps++;
    }
}

void
Win32SoundPlayer::play_sound(Sound snd)
{
  SoundRegistry *s = &sound_registry[snd];
  if (! thread_handle)
    {
      SECURITY_ATTRIBUTES sa;
      sa.nLength = sizeof(sa);
      sa.bInheritHandle = TRUE;
      sa.lpSecurityDescriptor = NULL;
      DWORD id;
      thread_handle = CreateThread(&sa, 0, thread_proc, s, 0, &id);
    }
}


DWORD WINAPI 
Win32SoundPlayer::thread_proc(LPVOID lpParameter)
{
  SoundRegistry *snd = (SoundRegistry*) lpParameter;

  if (SoundPlayer::get_device() == SoundPlayer::DEVICE_SPEAKER)
    {
      play_speaker(snd->beeps);
    }
  else
    {
      PlaySound(snd->event_label, 0, SND_APPLICATION);
    }

  CloseHandle(thread_handle);
  thread_handle = NULL;
  return 0;
}
