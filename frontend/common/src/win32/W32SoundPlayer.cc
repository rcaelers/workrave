// W32SoundPlayer.cc --- Sound player
//
// Copyright (C) 2002 - 2008 Raymond Penners & Ray Satiro
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

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include <mmreg.h>

#include "W32SoundPlayer.hh"

#include "CoreFactory.hh"
#include "IConfigurator.hh"
#include "SoundPlayer.hh"
#include "Exception.hh"
#include "Util.hh"

#define	SAMPLE_BITS		    (8)
#define	WAVE_BUFFER_SIZE  (4096)

using namespace workrave;

static std::string sound_filename;

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
}

W32SoundPlayer::~W32SoundPlayer()
{
}


/*
thread routine changed
jay satiro, workrave project, june 2007
redistribute under GNU terms.
*/

void
W32SoundPlayer::play_sound(SoundPlayer::SoundEvent snd )
{
  TRACE_ENTER_MSG( "W32SoundPlayer::play_sound", SoundPlayer::sound_registry[snd].friendly_name );

  // if ( sound == &sound_registry[snd] )
  //   {
  //     TRACE_MSG( "Sound already queued: sound == &sound_registry[snd]" );
  //   }
  // else if( sound == NULL )
  //   {
  //     DWORD id;
  //     sound = &sound_registry[snd];
  //     CloseHandle( CreateThread( NULL, 0, thread_Play, this, 0, &id ) );
  //   }
  // else
  //   {
  //     TRACE_MSG( "Failed: sound != NULL && sound != &sound_registry[snd]" );
  //   }
  TRACE_EXIT();
}


bool
W32SoundPlayer::capability(SoundPlayer::SoundCapability cap)
{
  (void) cap;
  return false;
}

void
W32SoundPlayer::play_sound(string wavfile)
{
  TRACE_ENTER_MSG( "W32SoundPlayer::play_sound", wavfile);

  if (sound_filename != "")
    {
      TRACE_MSG("Sound already queued");
    }
  else 
    {
      DWORD id;
      sound_filename = wavfile;
      CloseHandle(CreateThread(NULL, 0, thread_Play, this, 0, &id));
    }

  TRACE_EXIT();
}


bool
W32SoundPlayer::get_sound_enabled(SoundPlayer::SoundEvent snd, bool &enabled)
{
  char key[MAX_PATH], val[MAX_PATH];
  
  strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
  strcat(key, SoundPlayer::sound_registry[snd].label);
  strcat(key, "\\.current");

  if (registry_get_value(key, NULL, val))
    {
      enabled = (val[0] != '\0');
    }
  
  return true;
}


void
W32SoundPlayer::set_sound_enabled(SoundPlayer::SoundEvent snd, bool enabled)
{
  if (enabled)
    {
      char key[MAX_PATH], def[MAX_PATH];
      
      strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
      strcat(key, SoundPlayer::sound_registry[snd].label);
      strcat(key, "\\.default");

      if (registry_get_value(key, NULL, def))
        {
          char *def = strrchr(key, '.');
          strcpy(def, ".current");
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
W32SoundPlayer::get_sound_wav_file(SoundPlayer::SoundEvent snd, std::string &wav_file)
{
  char key[MAX_PATH], val[MAX_PATH];

  strcpy(key, "AppEvents\\Schemes\\Apps\\Workrave\\");
  strcat(key, SoundPlayer::sound_registry[snd].label);
  strcat(key, "\\.current");
  
  if (registry_get_value(key, NULL, val))
    {
      wav_file = val;
    }
  
  return true;;
}

void
W32SoundPlayer::set_sound_wav_file(SoundPlayer::SoundEvent snd, const std::string &wav_file)
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
  char *def = strrchr(key, '.');
  strcpy(def, ".current");
  registry_set_value(key, NULL, wav_file.c_str());
}


DWORD WINAPI
W32SoundPlayer::thread_Play(LPVOID lpParam)
{
  W32SoundPlayer *pThis = (W32SoundPlayer *) lpParam;
  pThis->Play();

  return (DWORD) 0;
}

void W32SoundPlayer::Play()
{
  TRACE_ENTER("W32SoundPlayer::Play");

  try
    {
      load_wav_file(sound_filename);
      open();
      write(sample, sample_size);
      close();

    }
  catch(Exception e)
    {
      TRACE_MSG(e.details());
    }
  catch(...)
    {
    }

  sound_filename = "";
  
  TRACE_EXIT();
}


void
W32SoundPlayer::open()
{
	MMRESULT res = MMSYSERR_NOERROR;
	int i;

	wave_event = CreateEvent(NULL, FALSE, FALSE, NULL);

  number_of_buffers          = 16;
	buffer_position            = 0;

	res = waveOutOpen(&waveout, WAVE_MAPPER, &format,
                    (DWORD) wave_event, (DWORD) 0,CALLBACK_EVENT);
  if (res != MMSYSERR_NOERROR)
    {
      throw Exception("waveOutOpen");
    }


  int volume = 100;
  CoreFactory::get_configurator()->get_value(SoundPlayer::CFG_KEY_SOUND_VOLUME, volume);
  
  volume = (volume * 0xFFFF / 100);
  volume = volume | (volume << 16);
 	
  res = waveOutSetVolume(waveout, volume);
  if (res != MMSYSERR_NOERROR)
    {
      throw Exception("waveOutSetVolume");
    }
  
  buffers = (WAVEHDR**) malloc(number_of_buffers * sizeof(WAVEHDR**));
  for (i = 0; i < number_of_buffers; i++)
    {
      buffers[i] = (WAVEHDR*) calloc(1, sizeof(WAVEHDR));
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


int
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
              throw Exception("waveOutOpen");
            }
          
          WaitForSingleObject(wave_event, INFINITE);
        }
      
      int chunck_size = WAVE_BUFFER_SIZE - buffers[i]->dwBytesRecorded;
      if (ptr + chunck_size > end)
        {
          chunck_size = end - ptr;
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
    }
  
  free(buffers);

  res = waveOutClose(waveout);
  if (res != MMSYSERR_NOERROR)
    {
      throw Exception("waveOutRestart");
    }
}

void
W32SoundPlayer::load_wav_file(const string &filename)
{
  MMRESULT res;
  
  HMMIO handle = mmioOpen((CHAR*)filename.c_str(), NULL, MMIO_ALLOCBUF | MMIO_READ);
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
  
  if (parent.ckid != FOURCC_RIFF || parent.fccType != mmioFOURCC('W', 'A', 'V', 'E' ))
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


  int pos = 0;
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

		} while (pos < (int)sample_size && mmioAdvance(handle, &mmio, MMIO_READ) == 0);
  

	mmioClose(handle, 0);
}

