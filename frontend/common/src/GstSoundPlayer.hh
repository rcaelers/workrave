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
  virtual ~GstSoundPlayer();

  void init(ISoundDriverEvents *events);
  bool capability(SoundCapability cap);
  void play_sound(SoundEvent snd);
  void play_sound(std::string wavfile);

  static gboolean bus_watch(GstBus *bus, GstMessage *msg, gpointer data);

private:
  //! GStreamer init OK.
  gboolean gst_ok;

  //!
  ISoundDriverEvents *events;

  struct WatchData
  {
    GstSoundPlayer *player;
    GstElement *play;
  };
};

#endif

#endif // GSTSOUNDPLAYER_HH
