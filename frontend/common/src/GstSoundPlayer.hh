// GstSoundPlayer.hh
//
// Copyright (C) 2008, 2009, 2010, 2013 Rob Caelers
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

#ifndef GSTSOUNDPLAYER_HH
#define GSTSOUNDPLAYER_HH

#include <ISoundDriver.hh>

#ifdef HAVE_GSTREAMER

#  include <gst/gst.h>

class GstSoundPlayer : public ISoundDriver
{
public:
  GstSoundPlayer();
  ~GstSoundPlayer() override;

  void init(ISoundDriverEvents *events) override;
  bool capability(SoundCapability cap) override;
  void play_sound(SoundEvent snd) override;
  void play_sound(std::string wavfile) override;

  bool get_sound_enabled(SoundEvent snd, bool &enabled) override;
  void set_sound_enabled(SoundEvent snd, bool enabled) override;
  bool get_sound_wav_file(SoundEvent snd, std::string &wav_file) override;
  void set_sound_wav_file(SoundEvent snd, const std::string &wav_file) override;

  static gboolean bus_watch(GstBus *bus, GstMessage *msg, gpointer data);

private:
  //! GStreamer init OK.
  gboolean gst_ok{false};

  //!
  ISoundDriverEvents *events{nullptr};

  struct WatchData
  {
    GstSoundPlayer *player{nullptr};
    GstElement *play{nullptr};
  };
};

#endif

#endif // GSTSOUNDPLAYER_HH
