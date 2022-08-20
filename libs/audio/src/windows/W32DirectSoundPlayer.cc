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
#  include "config.h"
#endif

#include "debug.hh"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef STRICT
#  define STRICT
#endif

#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <dsound.h>

#include "W32DirectSoundPlayer.hh"

#include "SoundPlayer.hh"
#include "utils/Exception.hh"

#define SAMPLE_BITS (8)
#define WAVE_BUFFER_SIZE (4096)

using namespace workrave;
using namespace workrave::utils;
using namespace workrave::audio;

static std::string sound_filename;

#if !defined(PLATFORM_OS_WINDOWS_NATIVE)
extern "C"
{
  void _chkstk()
  {
  }
}
#endif

//!
void
W32DirectSoundPlayer::init(ISoundPlayerEvents *events)
{
  this->events = events;
}

bool
W32DirectSoundPlayer::capability(SoundCapability cap)
{
  if (cap == workrave::audio::SoundCapability::VOLUME)
    {
      return true;
    }
  if (cap == workrave::audio::SoundCapability::EOS_EVENT)
    {
      return true;
    }
  return false;
}

void
W32DirectSoundPlayer::play_sound(std::string wavfile, int volume)
{
  TRACE_ENTRY_PAR(wavfile);

  if (wavfile != "")
    {
      DWORD id;

      SoundClip *clip = new SoundClip(wavfile, events, volume);
      CloseHandle(CreateThread(NULL, 0, play_thread, clip, 0, &id));
    }
}

DWORD WINAPI
W32DirectSoundPlayer::play_thread(LPVOID lpParam)
{
  TRACE_ENTRY();
  SoundClip *clip = (SoundClip *)lpParam;

  try
    {
      if (clip != NULL)
        {
          clip->init();
          clip->play();
        }
    }
  catch (Exception &e)
    {
      TRACE_MSG("Exception: {}", e.details());
    }
  catch (...)
    {
    }

  delete clip;
  return (DWORD)0;
}

SoundClip::SoundClip(const std::string &filename, ISoundPlayerEvents *events, int volume)
{
  TRACE_ENTRY();
  this->direct_sound = NULL;
  this->filename = filename;
  this->events = events;
  this->volume = volume;

  wave_file = NULL;
  sound_buffer = NULL;
  sound_buffer_size = 0;
  stop_event = NULL;
}

SoundClip::~SoundClip()
{
  TRACE_ENTRY();
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
}

std::string
SoundClip::get_error_string(DWORD error_code)
{
  if (error_code)
    {
      LPVOID buffer;
      DWORD buffer_len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM
                                         | FORMAT_MESSAGE_IGNORE_INSERTS,
                                       NULL,
                                       error_code,
                                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                       (LPTSTR)&buffer,
                                       0,
                                       NULL);

      if (buffer_len > 0)
        {
          LPCSTR msg = (LPCSTR)buffer;
          std::string result(msg, msg + buffer_len);
          LocalFree(buffer);
          return result;
        }
    }
  return std::string();
}

void
SoundClip::init()
{
  HRESULT hr = S_OK;

  TRACE_ENTRY();
  hr = DirectSoundCreate8(NULL, &direct_sound, NULL);
  if (FAILED(hr) || direct_sound == NULL)
    {
      throw Exception(std::string("DirectSoundCreate8") + get_error_string(hr));
    }

  hr = direct_sound->SetCooperativeLevel(GetDesktopWindow(), DSSCL_PRIORITY);
  if (FAILED(hr))
    {
      throw Exception(std::string("IDirectSound_SetCooperativeLevel") + get_error_string(hr));
    }

  wave_file = new WaveFile(filename);
  wave_file->init();

  sound_buffer_size = wave_file->get_size();
  if (sound_buffer_size <= 0)
    {
      TRACE_MSG("Exception: WAV has zero size");
      throw Exception(std::string("WAV has zero size"));
    }

  DSBUFFERDESC dsbd;
  ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
  dsbd.dwSize = sizeof(DSBUFFERDESC);
  dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
  dsbd.dwBufferBytes = sound_buffer_size;
  dsbd.guid3DAlgorithm = GUID_NULL;
  dsbd.lpwfxFormat = wave_file->get_format();

  hr = direct_sound->CreateSoundBuffer(&dsbd, &sound_buffer, NULL);
  if (FAILED(hr) || sound_buffer == NULL)
    {
      TRACE_MSG("Exception: IDirectSoundBuffer_CreateSoundBuffer");
      throw Exception(std::string("IDirectSoundBuffer_CreateSoundBuffer") + get_error_string(hr));
    }

  LPDIRECTSOUNDNOTIFY notify;
  hr = sound_buffer->QueryInterface(IID_IDirectSoundNotify8, (LPVOID *)&notify);
  if (FAILED(hr) || notify == NULL)
    {
      TRACE_MSG("Exception: IDirectSoundBuffer_QueryInterface IDirectSoundNotify {}", get_error_string(hr));
      throw Exception(std::string("IDirectSoundBuffer_QueryInterface IDirectSoundNotify") + get_error_string(hr));
    }

  stop_event = CreateEvent(0, false, false, 0);

  DSBPOSITIONNOTIFY pn;
  pn.dwOffset = DSBPN_OFFSETSTOP;
  pn.hEventNotify = stop_event;

  hr = notify->SetNotificationPositions(1, &pn);
  if (FAILED(hr))
    {
      TRACE_MSG("Exception: IDirectSoundNotify_SetPositionNotify {}", get_error_string(hr));
      throw Exception(std::string("IDirectSoundNotify_SetPositionNotify") + get_error_string(hr));
    }
  notify->Release();

  fill_buffer();

  set_volume(volume);
}

void
SoundClip::fill_buffer()
{
  TRACE_ENTRY();
  HRESULT hr;
  VOID *locked_sound_buffer = NULL;
  DWORD locked_sound_buffer_size = 0;

  restore_buffer();

  hr = sound_buffer->Lock(0, sound_buffer_size, &locked_sound_buffer, &locked_sound_buffer_size, NULL, NULL, 0L);
  if (FAILED(hr))
    {
      TRACE_MSG("Exception: IDirectSoundBuffer_Lock");
      throw Exception(std::string("IDirectSoundBuffer_Lock") + get_error_string(hr));
    }

  wave_file->reset_file();
  size_t bytes_read = wave_file->read((BYTE *)locked_sound_buffer, locked_sound_buffer_size);

  if (locked_sound_buffer_size - bytes_read > 0)
    {
      FillMemory((BYTE *)locked_sound_buffer + bytes_read,
                 locked_sound_buffer_size - bytes_read,
                 (BYTE)(wave_file->get_format()->wBitsPerSample == 8 ? 128 : 0));
    }

  sound_buffer->Unlock(locked_sound_buffer, locked_sound_buffer_size, NULL, 0);
  sound_buffer->SetCurrentPosition(0);
}

bool
SoundClip::is_buffer_lost()
{
  TRACE_ENTRY();
  DWORD status;
  HRESULT hr;

  hr = sound_buffer->GetStatus(&status);
  if (FAILED(hr))
    {
      TRACE_MSG("Exception: IDirectSound_GetStatus");
      throw Exception(std::string("IDirectSound_GetStatus") + get_error_string(hr));
    }

  return (status & DSBSTATUS_BUFFERLOST) != 0;
}

void
SoundClip::restore_buffer()
{
  TRACE_ENTRY();
  HRESULT hr = S_OK;
  do
    {
      hr = sound_buffer->Restore();
      if (hr == DSERR_BUFFERLOST)
        Sleep(10);
    }
  while (hr != S_OK);
}

void
SoundClip::play()
{
  TRACE_ENTRY();
  if (is_buffer_lost())
    {
      fill_buffer();
    }

  sound_buffer->Play(0, 0, 0);

  WaitForSingleObject(stop_event, INFINITE);
  if (events != NULL)
    {
      events->eos_event();
    }
}

void
SoundClip::set_volume(int volume)
{
  TRACE_ENTRY();
  if (sound_buffer != NULL)
    {
      long dsVolume;

      if (volume == 0)
        {
          dsVolume = -10000;
        }
      else
        {
          dsVolume = 100 * (long)(20 * log10((double)volume / 100.0));
        }

      dsVolume = dsVolume > 0 ? 0 : (dsVolume < -10000 ? -10000 : dsVolume);

      sound_buffer->SetVolume(dsVolume);
    }
}

WaveFile::WaveFile(const std::string &filename)
  : filename(filename)
{
  TRACE_ENTRY();
  mmio = NULL;
  sample_size = 0;

  memset((void *)&child, 0, sizeof(child));
  memset((void *)&parent, 0, sizeof(parent));
}

WaveFile::~WaveFile()
{
  TRACE_ENTRY();
  if (mmio != NULL)
    {
      mmioClose(mmio, 0);
      mmio = NULL;
    }
}

void
WaveFile::init()
{
  TRACE_ENTRY();
  MMRESULT res;

  mmio = mmioOpen((CHAR *)filename.c_str(), NULL, MMIO_ALLOCBUF | MMIO_READ);
  if (mmio == NULL)
    {
      TRACE_MSG("Exception: mmioOpen");
      throw Exception("mmioOpen");
    }

  memset((void *)&parent, 0, sizeof(parent));

  res = mmioDescend(mmio, &parent, NULL, 0);
  if (res != MMSYSERR_NOERROR)
    {
      TRACE_MSG("Exception: mmioDescend1");
      throw Exception("mmioDescend1");
    }

  if (parent.ckid != FOURCC_RIFF || parent.fccType != mmioFOURCC('W', 'A', 'V', 'E'))
    {
      TRACE_MSG("Exception: no Wave");
      throw Exception("no Wave");
    }

  memset((void *)&child, 0, sizeof(child));

  child.ckid = mmioFOURCC('f', 'm', 't', ' ');

  res = mmioDescend(mmio, &child, &parent, MMIO_FINDCHUNK);
  if (res != MMSYSERR_NOERROR)
    {
      TRACE_MSG("Exception: mmioDescend2");
      throw Exception("mmioDescend2");
    }
  if (child.cksize < sizeof(PCMWAVEFORMAT))
    {
      TRACE_MSG("Exception: chunk size");
      throw Exception("chunk size");
    }

  int len = mmioRead(mmio, (HPSTR)&format, sizeof(format));
  if (len != sizeof(format))
    {
      TRACE_MSG("Exception: format size");
      throw Exception("format size");
    }

  if (format.wFormatTag != WAVE_FORMAT_PCM)
    {
      TRACE_MSG("Exception: format supported");
      throw Exception("format supported");
    }
  format.cbSize = 0;

  res = mmioAscend(mmio, &child, 0);
  if (res != MMSYSERR_NOERROR)
    {
      TRACE_MSG("Exception: mmioAscend");
      throw Exception("mmioAscend");
    }

  reset_file();

  sample_size = child.cksize;
}

DWORD
WaveFile::get_size()
{
  return sample_size;
}

void
WaveFile::reset_file()
{
  TRACE_ENTRY();
  if (-1 == mmioSeek(mmio, parent.dwDataOffset + sizeof(FOURCC), SEEK_SET))
    {
      TRACE_MSG("Exception: mmioSeek");
      throw Exception("mmioSeek");
    }

  memset((void *)&child, 0, sizeof(child));
  child.ckid = mmioFOURCC('d', 'a', 't', 'a');
  if (0 != mmioDescend(mmio, &child, &parent, MMIO_FINDCHUNK))
    {
      TRACE_MSG("Exception: mmioDescend");
      throw Exception("mmioDescend");
    }
}

size_t
WaveFile::read(BYTE *buffer, size_t size)
{
  TRACE_ENTRY_PAR(size);
  MMRESULT res;

  MMIOINFO mmioInfo;
  res = mmioGetInfo(mmio, &mmioInfo, 0);
  if (res != MMSYSERR_NOERROR)
    {
      TRACE_MSG("Exception: mmioGetInfo");
      throw Exception("mmioGetInfo");
    }

  size_t pos = 0;
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
    }
  while (pos < size && mmioAdvance(mmio, &mmioInfo, MMIO_READ) == 0);

  mmioSetInfo(mmio, &mmioInfo, 0);

  return pos;
}
