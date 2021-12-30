// Copyright (C) 2002 - 2013 Raymond Penners & Ray Satiro
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
#  include "config.h"
#endif

#include "debug.hh"

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include <mmreg.h>

#include "W32SoundPlayer.hh"

#include "utils/Exception.hh"

#define SAMPLE_BITS (8)
#define WAVE_BUFFER_SIZE (4096)

using namespace workrave::utils;
using namespace std;

static std::string sound_filename;

W32SoundPlayer::W32SoundPlayer()
{
}

W32SoundPlayer::~W32SoundPlayer()
{
}

bool
W32SoundPlayer::capability(workrave::audio::SoundCapability cap)
{
  if (cap == workrave::audio::SoundCapability::VOLUME)
    {
      return true;
    }
  return false;
}

/*
thread routine changed
jay satiro, workrave project, june 2007
redistribute under GNU terms.
*/

void
W32SoundPlayer::play_sound(std::string wavfile, int volume)
{
  TRACE_ENTRY_PAR(wavfile);

  if (sound_filename != "")
    {
      TRACE_MSG("Sound already queued");
    }
  else
    {
      this->volume = volume;
      DWORD id;
      sound_filename = wavfile;
      CloseHandle(CreateThread(NULL, 0, thread_Play, this, 0, &id));
    }
}

DWORD WINAPI
W32SoundPlayer::thread_Play(LPVOID lpParam)
{
  W32SoundPlayer *pThis = (W32SoundPlayer *)lpParam;
  pThis->Play();

  return (DWORD)0;
}

void
W32SoundPlayer::Play()
{
  TRACE_ENTRY();
  try
    {
      load_wav_file(sound_filename);
      open();
      write(sample, sample_size);
      close();
    }
  catch (Exception &e)
    {
      TRACE_VAR(e.details());
    }
  catch (...)
    {
    }

  sound_filename = "";
}

void
W32SoundPlayer::open()
{
  MMRESULT res = MMSYSERR_NOERROR;
  int i;

  wave_event = CreateEvent(NULL, FALSE, FALSE, NULL);

  number_of_buffers = 16;
  buffer_position = 0;

  res = waveOutOpen(&waveout, WAVE_MAPPER, &format, (DWORD_PTR)wave_event, (DWORD_PTR)0, CALLBACK_EVENT);
  if (res != MMSYSERR_NOERROR)
    {
      throw Exception("waveOutOpen");
    }

  res = waveOutPause(waveout);
  if (res != MMSYSERR_NOERROR)
    {
      throw Exception("waveOutPause");
    }

  int vol = volume;
  vol = (vol * 0xFFFF / 100);
  vol = vol | (vol << 16);

  res = waveOutSetVolume(waveout, vol);
  if (res != MMSYSERR_NOERROR)
    {
      throw Exception("waveOutSetVolume");
    }

  buffers = (WAVEHDR **)malloc(number_of_buffers * sizeof(WAVEHDR **));
  for (i = 0; i < number_of_buffers; i++)
    {
      buffers[i] = (WAVEHDR *)calloc(1, sizeof(WAVEHDR));
      if (buffers[i] == NULL)
        {
          throw Exception("buffers malloc");
        }

      if (buffers[i] != NULL)
        {
          buffers[i]->lpData = (CHAR *)malloc(WAVE_BUFFER_SIZE);
          if (buffers[i]->lpData == NULL)
            {
              throw Exception("buffer malloc");
            }
        }
    }
}

size_t
W32SoundPlayer::write(unsigned char *buf, size_t size)
{
  unsigned char *ptr = buf;
  unsigned char *end = buf + size;
  MMRESULT res;

  for (int i = buffer_position; ptr < end; i = (i + 1) % number_of_buffers)
    {
      while ((buffers[i]->dwFlags & WHDR_INQUEUE) != 0)
        {
          res = waveOutRestart(waveout);
          if (res != MMSYSERR_NOERROR)
            {
              throw Exception("waveOutRestart");
            }

          WaitForSingleObject(wave_event, INFINITE);
        }

      DWORD chunck_size = WAVE_BUFFER_SIZE - buffers[i]->dwBytesRecorded;
      if (ptr + chunck_size > end)
        {
          chunck_size = static_cast<DWORD>(end - ptr);
        }

      memcpy(buffers[i]->lpData + buffers[i]->dwBytesRecorded, ptr, chunck_size);
      ptr += chunck_size;
      buffers[i]->dwBytesRecorded += chunck_size;

      if (buffers[i]->dwBytesRecorded == WAVE_BUFFER_SIZE)
        {
          flush_buffer(i);
          buffer_position = (i + 1) % number_of_buffers;
        }
    }

  return ptr - buf;
}

void
W32SoundPlayer::flush_buffer(int i)
{
  MMRESULT res;

  if (buffers[i]->dwBytesRecorded != 0)
    {
      buffers[i]->dwBufferLength = buffers[i]->dwBytesRecorded;
      buffers[i]->dwBytesRecorded = 0;
      buffers[i]->dwFlags = 0;

      res = waveOutPrepareHeader(waveout, buffers[i], sizeof(WAVEHDR));
      if (res != MMSYSERR_NOERROR)
        {
          throw Exception("waveOutPrepareHeader");
        }

      res = waveOutWrite(waveout, buffers[i], sizeof(WAVEHDR));
      if (res != MMSYSERR_NOERROR)
        {
          throw Exception("waveOutWrite");
        }
    }
}

void
W32SoundPlayer::close(void)
{
  MMRESULT res;

  flush_buffer(buffer_position);

  res = waveOutRestart(waveout);
  if (res != MMSYSERR_NOERROR)
    {
      throw Exception("waveOutRestart");
    }

  for (int i = 0; i < number_of_buffers; ++i)
    {
      while ((buffers[i]->dwFlags & WHDR_INQUEUE) != 0)
        {
          WaitForSingleObject(wave_event, INFINITE);
        }

      res = waveOutUnprepareHeader(waveout, buffers[i], sizeof(WAVEHDR));
      if (res != MMSYSERR_NOERROR)
        {
          throw Exception("waveOutUnprepareHeader");
        }

      free(buffers[i]->lpData);
      free(buffers[i]);
      buffers[i] = NULL;
    }

  free(buffers);
  free(sample);

  buffers = NULL;
  sample = NULL;

  res = waveOutClose(waveout);
  if (res != MMSYSERR_NOERROR)
    {
      throw Exception("waveOutClose");
    }
}

void
W32SoundPlayer::load_wav_file(const std::string &filename)
{
  MMRESULT res;

  HMMIO handle = mmioOpen((CHAR *)filename.c_str(), NULL, MMIO_ALLOCBUF | MMIO_READ);
  if (handle == NULL)
    {
      throw Exception("mmioOpen");
    }

  MMCKINFO parent;
  memset((void *)&parent, 0, sizeof(parent));

  res = mmioDescend(handle, &parent, NULL, 0);
  if (res != MMSYSERR_NOERROR)
    {
      throw Exception("mmioDescend");
    }

  if (parent.ckid != FOURCC_RIFF || parent.fccType != mmioFOURCC('W', 'A', 'V', 'E'))
    {
      throw Exception("no Wave");
    }

  MMCKINFO child;
  memset((void *)&child, 0, sizeof(child));

  parent.ckid = mmioFOURCC('f', 'm', 't', ' ');

  res = mmioDescend(handle, &child, &parent, MMIO_FINDCHUNK);
  if (res != MMSYSERR_NOERROR)
    {
      throw Exception("mmioDescend");
    }
  if (child.cksize < sizeof(PCMWAVEFORMAT))
    {
      throw Exception("chunk size");
    }

  int len = mmioRead(handle, (HPSTR)&format, sizeof(format));
  if (len != sizeof(format))
    {
      throw Exception("format size");
    }

  if (format.wFormatTag != WAVE_FORMAT_PCM)
    {
      throw Exception("format supported");
    }

  res = mmioAscend(handle, &child, 0);
  if (res != MMSYSERR_NOERROR)
    {
      throw Exception("mmioAscend");
    }

  memset((void *)&child, 0, sizeof(child));

  parent.ckid = mmioFOURCC('d', 'a', 't', 'a');
  res = mmioDescend(handle, &child, &parent, MMIO_FINDCHUNK);
  if (res != MMSYSERR_NOERROR)
    {
      throw Exception("mmioAscend");
    }

  sample = (unsigned char *)malloc(child.cksize);
  sample_size = child.cksize;
  if (sample == NULL)
    {
      throw Exception("malloc");
    }

  MMIOINFO mmio;
  res = mmioGetInfo(handle, &mmio, 0);
  if (res != MMSYSERR_NOERROR)
    {
      throw Exception("mmioAscend");
    }

  size_t pos = 0;
  do
    {
      size_t copy = mmio.pchEndRead - mmio.pchNext;

      if (copy > 0)
        {
          if (copy > sample_size - pos)
            {
              copy = sample_size - pos;
            }

          memcpy(sample + pos, mmio.pchNext, copy);
          pos += copy;
        }

      mmio.pchNext = mmio.pchEndRead;
    }
  while (pos < sample_size && mmioAdvance(handle, &mmio, MMIO_READ) == 0);

  mmioClose(handle, 0);
}
