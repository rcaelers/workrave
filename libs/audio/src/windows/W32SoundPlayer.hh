// Copyright (C) 2002 - 2013 Raymond Penners, Ray Satiro, Rob Caelers
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

#ifndef W32SOUNDPLAYER_HH
#define W32SOUNDPLAYER_HH

#include "ISoundDriver.hh"

class W32SoundPlayer : public ISoundDriver
{
public:
  W32SoundPlayer();
  virtual ~W32SoundPlayer();

  void init(ISoundPlayerEvents *)
  {
  }
  bool capability(workrave::audio::SoundCapability cap);
  void play_sound(std::string wavfile, int volume);

protected:
  static DWORD WINAPI thread_Play(LPVOID);

private:
  void Play();

  void open();
  void close();
  size_t write(unsigned char *buffer, size_t size);
  void flush_buffer(int buffer);
  void load_wav_file(const std::string &filename);

  HWAVEOUT waveout;
  HANDLE wave_event;

  int buffer_position;
  int number_of_buffers;
  WAVEHDR **buffers;
  unsigned char *sample;
  size_t sample_size;
  WAVEFORMATEX format;
  int volume;
};

#endif // W32SOUNDPLAYER_HH
