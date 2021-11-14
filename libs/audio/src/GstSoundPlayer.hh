// Copyright (C) 2008 - 2013 Rob Caelers <robc@krandor.nl>
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

#include "ISoundDriver.hh"

#include <gst/gst.h>

class GstSoundPlayer : public ISoundDriver
{
public:
  GstSoundPlayer();
  ~GstSoundPlayer() override;

  void init(ISoundPlayerEvents *events) override;
  bool capability(workrave::audio::SoundCapability cap) override;
  void play_sound(std::string wavfile, int volume) override;

  static gboolean bus_watch(GstBus *bus, GstMessage *msg, gpointer data);

private:
  gboolean gst_ok{false};
  ISoundPlayerEvents *events{nullptr};

  struct WatchData
  {
    GstSoundPlayer *player{nullptr};
    GstElement *play{nullptr};
  };
};

#endif // GSTSOUNDPLAYER_HH
