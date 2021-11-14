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

#ifndef W32DIRECTSOUNDPLAYER_HH
#define W32DIRECTSOUNDPLAYER_HH

#include "ISoundDriver.hh"

#include <windows.h>
#include <dsound.h>
#include <string>

class W32DirectSoundPlayer : public ISoundDriver
{
public:
  W32DirectSoundPlayer() = default;
  virtual ~W32DirectSoundPlayer() = default;

  void init(ISoundPlayerEvents *events);
  bool capability(workrave::audio::SoundCapability cap);
  void play_sound(std::string wavfile, int volume);

private:
  static DWORD WINAPI play_thread(LPVOID);

  void play();

private:
  ISoundPlayerEvents *events;
};

class WaveFile
{
public:
  WaveFile(const std::string &filename);
  ~WaveFile();

  void init();

  size_t read(BYTE *buffer, size_t size);
  DWORD get_size();

  void reset_file();
  WAVEFORMATEX *get_format()
  {
    return &format;
  };

private:
  std::string filename;
  HMMIO mmio;
  WAVEFORMATEX format;
  MMCKINFO child;
  MMCKINFO parent;
  DWORD sample_size;
};

class SoundClip
{
public:
  SoundClip(const std::string &filename, ISoundPlayerEvents *events, int volume);
  virtual ~SoundClip();

  void init();
  void play();
  void set_volume(int volume);

private:
  void fill_buffer();
  bool is_buffer_lost();
  void restore_buffer();
  std::string get_error_string(DWORD error_code);

private:
  std::string filename;
  LPDIRECTSOUND8 direct_sound;
  WaveFile *wave_file;
  LPDIRECTSOUNDBUFFER sound_buffer;
  DWORD sound_buffer_size;
  HANDLE stop_event;
  ISoundPlayerEvents *events;
  int volume;
};

#endif // W32DIRECTSOUNDPLAYER_HH
