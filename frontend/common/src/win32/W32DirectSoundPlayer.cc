// W32DirectSoundPlayer.cc --- Sound player
//
// Copyright (C) 2002 - 2010, 2012, 2013 Raymond Penners & Ray Satiro
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define STRICT
#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <dxerr8.h>
#ifndef DXGetErrorString8
#define DXGetErrorString8 DXGetErrorString
#endif
#include <dsound.h>

#include <glib.h>
#include "W32DirectSoundPlayer.hh"

#include "CoreFactory.hh"
#include "config/IConfigurator.hh"
#include "SoundPlayer.hh"
#include "Exception.hh"
#include "Util.hh"

#define	SAMPLE_BITS		    (8)
#define	WAVE_BUFFER_SIZE  (4096)

using namespace workrave;

static std::string sound_filename;

#ifndef PLATFORM_OS_WIN32_NATIVE
extern "C"
{
  void _chkstk()
  {
  }
}
#endif

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


//! Constructor
W32DirectSoundPlayer::W32DirectSoundPlayer()
{
}


//! Destructor
W32DirectSoundPlayer::~W32DirectSoundPlayer()
{
}

//!
void
W32DirectSoundPlayer::init(ISoundDriverEvents *events)
{
  this->events = events;
}


void
W32DirectSoundPlayer::play_sound(SoundEvent snd)
{
  TRACE_ENTER_MSG( "W32DirectSoundPlayer::play_sound", SoundPlayer::sound_registry[snd].friendly_name );
  TRACE_EXIT();
}


bool
W32DirectSoundPlayer::capability(SoundCapability cap)
{
  if (cap == SOUND_CAP_EDIT)
    {
      return true;
    }
  if (cap == SOUND_CAP_VOLUME)
    {
      return true;
    }
  if (cap == SOUND_CAP_EOS_EVENT)
    {
      return true;
    }
  return false;
}


void
W32DirectSoundPlayer::play_sound(string wavfile)
{
  TRACE_ENTER_MSG( "W32DirectSoundPlayer::play_sound", wavfile);

  if (wavfile != "")
    {
      DWORD id;
      
      SoundClip *clip = new SoundClip(wavfile, events);
      CloseHandle(CreateThread(NULL, 0, play_thread, clip, 0, &id));
    }

  TRACE_EXIT();
}


bool
W32DirectSoundPlayer::get_sound_enabled(SoundEvent snd, bool &enabled)
{
  char key[MAX_PATH], val[MAX_PATH];

  strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
  strcat(key, SoundPlayer::sound_registry[snd].label);
  strcat(key, "\\.current");

  if (registry_get_value(key, NULL, val))
    {
      enabled = (val[0] != '\0');
      return true;
    }

  return false;
}


void
W32DirectSoundPlayer::set_sound_enabled(SoundEvent snd, bool enabled)
{
  if (enabled)
    {
      char key[MAX_PATH], def[MAX_PATH];

      strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
      strcat(key, SoundPlayer::sound_registry[snd].label);
      strcat(key, "\\.default");

      if (registry_get_value(key, NULL, def))
        {
          char *defkey = strrchr(key, '.');
          strcpy(defkey, ".current");
          registry_set_value(key, NULL, def);
        }
    }
  else
    {
      char key[MAX_PATH];

      strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
      strcat(key, SoundPlayer::sound_registry[snd].label);
      strcat(key, "\\.current");

      registry_set_value(key, NULL, "");
    }
}


bool
W32DirectSoundPlayer::get_sound_wav_file(SoundEvent snd, std::string &wav_file)
{
  char key[MAX_PATH], val[MAX_PATH];

  strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
  strcat(key, SoundPlayer::sound_registry[snd].label);
  strcat(key, "\\.current");

  if (registry_get_value(key, NULL, val))
    {
      wav_file = val;
    }

  if (wav_file == "")
    {
      char *cur = strrchr(key, '.');
      strcpy(cur, ".default");
      if (registry_get_value(key, NULL, val))
        {
          wav_file = val;
        }
    }

  return true;
}

void
W32DirectSoundPlayer::set_sound_wav_file(SoundEvent snd, const std::string &wav_file)
{
  char key[MAX_PATH], val[MAX_PATH];

  strcpy(key, "AppEvents\\EventLabels\\");
  strcat(key, SoundPlayer::sound_registry[snd].label);
  if (! registry_get_value(key, NULL, val))
    {
      registry_set_value(key, NULL, SoundPlayer::sound_registry[snd].friendly_name);
    }

  strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
  strcat(key, SoundPlayer::sound_registry[snd].label);
  strcat(key, "\\.default");
  registry_set_value(key, NULL, wav_file.c_str());

  bool enabled = false;
  bool valid = get_sound_enabled(snd, enabled);

  if (!valid || enabled)
    {
      char *def = strrchr(key, '.');
      strcpy(def, ".current");
      registry_set_value(key, NULL, wav_file.c_str());
    }
}


DWORD WINAPI
W32DirectSoundPlayer::play_thread(LPVOID lpParam)
{
  TRACE_ENTER("W32DirectSoundPlayer::play_thread");
  SoundClip *clip = (SoundClip *) lpParam;

  try
    {
      if (clip != NULL)
        {
          clip->init();
          clip->play();
        }
    }
  catch(Exception e)
    {
      TRACE_MSG("Exception: " << e.details());
    }
  catch(...)
    {
    }

  delete clip;
  
  TRACE_EXIT();
  return (DWORD) 0;
}



SoundClip::SoundClip(const string &filename, ISoundDriverEvents *events)
{
  TRACE_ENTER("SoundClip::SoundClip");

  this->direct_sound = NULL;
  this->filename = filename;
  this->events = events;

  wave_file = NULL;
  sound_buffer = NULL;
  sound_buffer_size = 0;
  stop_event = NULL;

  TRACE_EXIT();
}


SoundClip::~SoundClip()
{
  TRACE_ENTER("SoundClip::~SoundClip");
  if (sound_buffer != NULL)
    {
      sound_buffer->Release();
      sound_buffer = NULL;
    }

  if (stop_event != NULL)
    {
      CloseHandle(stop_event);
      stop_event = NULL;
    }

  if (direct_sound != NULL)
    {
      direct_sound->Release();
      direct_sound = NULL;
    }

  delete wave_file;
  wave_file = NULL;

  TRACE_EXIT();
}


void
SoundClip::init()
{
  HRESULT hr = S_OK;

  TRACE_ENTER("SoundClip::init");

  hr = DirectSoundCreate8(NULL, &direct_sound, NULL);
  if (FAILED(hr) || direct_sound == NULL)
    {
      throw Exception(string("DirectSoundCreate8") + DXGetErrorString8(hr));
    }

  hr = direct_sound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
  if (FAILED(hr))
    {
      throw Exception(string("IDirectSound_SetCooperativeLevel") + DXGetErrorString8(hr));
    }
  
  wave_file = new WaveFile(filename);
  wave_file->init();

  sound_buffer_size = wave_file->get_size();
  if (sound_buffer_size <= 0)
    {
      TRACE_RETURN("Exception: WAV has zero size");
      throw Exception(string("WAV has zero size"));
    }

  DSBUFFERDESC dsbd;
  ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
  dsbd.dwSize          = sizeof(DSBUFFERDESC);
  dsbd.dwFlags         = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
  dsbd.dwBufferBytes   = sound_buffer_size;
  dsbd.guid3DAlgorithm = GUID_NULL;
  dsbd.lpwfxFormat     = wave_file->get_format();

  hr = direct_sound->CreateSoundBuffer(&dsbd, &sound_buffer, NULL);
  if (FAILED(hr) || sound_buffer == NULL)
    {
      TRACE_RETURN("Exception: IDirectSoundBuffer_CreateSoundBuffer");
      throw Exception(string("IDirectSoundBuffer_CreateSoundBuffer") + DXGetErrorString8(hr));
    }

  LPDIRECTSOUNDNOTIFY notify;
  hr = sound_buffer->QueryInterface(IID_IDirectSoundNotify8, (LPVOID*)&notify);
  if (FAILED(hr) || notify == NULL)
    {
      TRACE_RETURN("Exception: IDirectSoundBuffer_QueryInterface IDirectSoundNotify" << DXGetErrorString8(hr));
      throw Exception(string("IDirectSoundBuffer_QueryInterface IDirectSoundNotify") + DXGetErrorString8(hr));
    }

  stop_event = CreateEvent(0, false, false, 0);

  DSBPOSITIONNOTIFY pn;
  pn.dwOffset = DSBPN_OFFSETSTOP;
  pn.hEventNotify = stop_event;

  hr = notify->SetNotificationPositions(1, &pn);
  if (FAILED(hr))
    {
      TRACE_RETURN("Exception: IDirectSoundNotify_SetPositionNotify" << DXGetErrorString8(hr));
      throw Exception(string("IDirectSoundNotify_SetPositionNotify") + DXGetErrorString8(hr));
    }
  notify->Release();

  fill_buffer();

  int volume = 100;
  CoreFactory::get_configurator()->get_value(SoundPlayer::CFG_KEY_SOUND_VOLUME, volume);
  set_volume(volume);

  TRACE_EXIT();
}

void
SoundClip::fill_buffer()
{
  TRACE_ENTER("SoundClip::fill_buffer");

  HRESULT hr;
  VOID *locked_sound_buffer = NULL;
  DWORD locked_sound_buffer_size = 0;

  restore_buffer();

  hr = sound_buffer->Lock(0, sound_buffer_size,
                          &locked_sound_buffer, &locked_sound_buffer_size,
                          NULL, NULL, 0L);
  if (FAILED(hr))
    {
      TRACE_RETURN("Exception: IDirectSoundBuffer_Lock");
      throw Exception(string("IDirectSoundBuffer_Lock") + DXGetErrorString8(hr));
    }

  wave_file->reset_file();
  int bytes_read = wave_file->read((BYTE*)locked_sound_buffer, locked_sound_buffer_size);

  if (locked_sound_buffer_size - bytes_read > 0)
    {
      FillMemory((BYTE*) locked_sound_buffer + bytes_read,
                 locked_sound_buffer_size - bytes_read,
                 (BYTE)(wave_file->get_format()->wBitsPerSample == 8 ? 128 : 0));
    }

  sound_buffer->Unlock(locked_sound_buffer, locked_sound_buffer_size, NULL, 0);
  sound_buffer->SetCurrentPosition(0);
  TRACE_EXIT();
}


bool
SoundClip::is_buffer_lost()
{
  TRACE_ENTER("SoundClip::is_buffer_lost");

  DWORD status;
  HRESULT hr;

  hr = sound_buffer->GetStatus(&status);
  if (FAILED(hr))
    {
      TRACE_RETURN("Exception: IDirectSound_GetStatus");
      throw Exception(string("IDirectSound_GetStatus") + DXGetErrorString8(hr));
    }

  TRACE_EXIT();
  return (status & DSBSTATUS_BUFFERLOST) != 0;
}


void
SoundClip::restore_buffer()
{
  TRACE_ENTER("SoundClip::restore_buffer");
  HRESULT hr = S_OK;
  do
    {
      hr = sound_buffer->Restore();
      if (hr == DSERR_BUFFERLOST)
        Sleep(10);
    }
  while (hr != S_OK);
  TRACE_EXIT();
}


void
SoundClip::play()
{
  TRACE_ENTER("SoundClip::play");

  if (is_buffer_lost())
    {
      fill_buffer();
    }

  HRESULT hr = sound_buffer->Play(0, 0, 0);

  WaitForSingleObject(stop_event, INFINITE);
  if (events != NULL)
    {
      events->eos_event();
    }

  TRACE_EXIT();
}


void
SoundClip::set_volume(int volume)
{
  TRACE_ENTER("SoundClip::set_volume");
  if (sound_buffer != NULL)
    {
      long dsVolume;

      if (volume == 0)
        {
          dsVolume = -10000;
        }
      else
        {
          dsVolume = 100 * (long) (20 * log10((double) volume / 100.0));
        }

      dsVolume = CLAMP(dsVolume, -10000, 0);

      sound_buffer->SetVolume(dsVolume);
    }
  TRACE_EXIT();
}


WaveFile::WaveFile(const string &filename)
  : filename(filename)
{
  TRACE_ENTER("WaveFile::WaveFile");
  mmio = NULL;
  sample_size = 0;

  memset((void *)&child, 0, sizeof(child));
  memset((void *)&parent, 0, sizeof(parent));
  TRACE_EXIT();
}


WaveFile::~WaveFile()
{
  TRACE_ENTER("WaveFile::~WaveFile");
  if (mmio != NULL)
    {
      mmioClose(mmio, 0);
      mmio = NULL;
    }
  TRACE_EXIT();
}


void
WaveFile::init()
{
  TRACE_ENTER("WaveFile::init");
  MMRESULT res;

  mmio = mmioOpen((CHAR*)filename.c_str(), NULL, MMIO_ALLOCBUF | MMIO_READ);
  if (mmio == NULL)
    {
      TRACE_RETURN("Exception: mmioOpen");
      throw Exception("mmioOpen");
    }

  memset((void *)&parent, 0, sizeof(parent));

  res = mmioDescend(mmio, &parent, NULL, 0);
  if (res != MMSYSERR_NOERROR)
    {
      TRACE_RETURN("Exception: mmioDescend1");
      throw Exception("mmioDescend1");
    }

  if (parent.ckid != FOURCC_RIFF || parent.fccType != mmioFOURCC('W', 'A', 'V', 'E' ))
    {
      TRACE_RETURN("Exception: no Wave");
      throw Exception("no Wave");
    }

  memset((void *)&child, 0, sizeof(child));

  child.ckid = mmioFOURCC('f', 'm', 't', ' ');

  res = mmioDescend(mmio, &child, &parent, MMIO_FINDCHUNK);
  if (res != MMSYSERR_NOERROR)
    {
      TRACE_RETURN("Exception: mmioDescend2");
      throw Exception("mmioDescend2");
    }
  if (child.cksize < sizeof(PCMWAVEFORMAT))
    {
      TRACE_RETURN("Exception: chunk size");
      throw Exception("chunk size");
    }

  int len = mmioRead(mmio, (HPSTR)&format, sizeof(format));
  if (len != sizeof(format))
    {
      TRACE_RETURN("Exception: format size");
      throw Exception("format size");
    }

  if (format.wFormatTag != WAVE_FORMAT_PCM)
    {
      TRACE_RETURN("Exception: format supported");
      throw Exception("format supported");
    }
  format.cbSize = 0;

  res = mmioAscend(mmio, &child, 0);
  if (res != MMSYSERR_NOERROR)
    {
      TRACE_RETURN("Exception: mmioAscend");
      throw Exception("mmioAscend");
    }

  reset_file();

  sample_size = child.cksize;
  TRACE_EXIT();
}

size_t
WaveFile::get_size()
{
    return sample_size;
}

void
WaveFile::reset_file()
{
  TRACE_ENTER("WaveFile::reset_file");
  if (-1 == mmioSeek(mmio, parent.dwDataOffset + sizeof(FOURCC), SEEK_SET))
    {
      TRACE_RETURN("Exception: mmioSeek");
      throw Exception("mmioSeek");
    }

  memset((void *)&child, 0, sizeof(child));
  child.ckid = mmioFOURCC('d', 'a', 't', 'a');
  if (0 != mmioDescend(mmio, &child, &parent, MMIO_FINDCHUNK))
    {
      TRACE_RETURN("Exception: mmioDescend");
      throw Exception("mmioDescend");
    }
  TRACE_EXIT();
}

size_t
WaveFile::read(BYTE *buffer, size_t size)
{
  TRACE_ENTER_MSG("WaveFile::read", size);
  MMRESULT res;

  MMIOINFO mmioInfo;
  res = mmioGetInfo(mmio, &mmioInfo, 0);
  if (res != MMSYSERR_NOERROR)
    {
      TRACE_RETURN("Exception: mmioGetInfo");
      throw Exception("mmioGetInfo");
    }

  int pos = 0;
  do
    {
      size_t copy = mmioInfo.pchEndRead - mmioInfo.pchNext;

      if (copy > 0)
        {
          if (copy > size - pos)
            {
              copy = size - pos;
            }

          memcpy(buffer + pos, mmioInfo.pchNext, copy);
          pos += copy;
        }

      mmioInfo.pchNext = mmioInfo.pchEndRead;

    } while (pos < (int)size && mmioAdvance(mmio, &mmioInfo, MMIO_READ) == 0);

  mmioSetInfo(mmio, &mmioInfo, 0);

  TRACE_EXIT();
  return pos;
}
