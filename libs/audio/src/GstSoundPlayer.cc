// Copyright (C) 2002 - 2014 Rob Caelers & Raymond Penners
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

#include "GstSoundPlayer.hh"
#include "ISoundPlayerEvents.hh"

using namespace std;

GstSoundPlayer::GstSoundPlayer()
{
  GError *error = nullptr;

  gst_ok = gst_init_check(nullptr, nullptr, &error);
  gst_registry_fork_set_enabled(FALSE);

  if (error != nullptr)
    {
      g_error_free(error);
      error = nullptr;
    }
}

GstSoundPlayer::~GstSoundPlayer()
{
  if (gst_ok)
    {
      gst_deinit();
    }
}

void
GstSoundPlayer::init(ISoundPlayerEvents *events)
{
  this->events = events;
}

bool
GstSoundPlayer::capability(workrave::audio::SoundCapability cap)
{
  return (cap == workrave::audio::SoundCapability::VOLUME || cap == workrave::audio::SoundCapability::EOS_EVENT);
}

void
GstSoundPlayer::play_sound(std::string wavfile, int volume)
{
  TRACE_ENTRY_PAR(wavfile, volume);

  GstElement *play = nullptr;
  GstElement *sink = gst_element_factory_make("autoaudiosink", "sink");

  if (sink != nullptr)
    {
      play = gst_element_factory_make("playbin", "play");
    }

  if (play != nullptr)
    {
      auto *watch_data = new WatchData;
      watch_data->player = this;
      watch_data->play = play;

      GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(play));
      gst_bus_add_watch(bus, bus_watch, watch_data);

      char *uri = g_strdup_printf("file://%s", wavfile.c_str());

      gst_element_set_state(play, GST_STATE_NULL);

      g_object_set(G_OBJECT(play), "uri", uri, "volume", (float)(volume / 100.0), "audio-sink", sink, NULL);

      gst_element_set_state(play, GST_STATE_PLAYING);

      gst_object_unref(bus);
      g_free(uri);
    }
}

gboolean
GstSoundPlayer::bus_watch(GstBus *bus, GstMessage *msg, gpointer data)
{
  auto *watch_data = (WatchData *)data;
  GstElement *play = watch_data->play;
  GError *err = nullptr;
  gboolean ret = TRUE;

  (void)bus;

  switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_ERROR:
      gst_message_parse_error(msg, &err, nullptr);
      g_error_free(err);
      /* FALLTHROUGH */

    case GST_MESSAGE_EOS:
      gst_element_set_state(play, GST_STATE_NULL);
      gst_object_unref(GST_OBJECT(play));
      ret = FALSE;

      if (watch_data->player->events != nullptr)
        {
          watch_data->player->events->eos_event();
        }
      break;

    case GST_MESSAGE_WARNING:
      gst_message_parse_warning(msg, &err, nullptr);
      g_error_free(err);
      break;

    default:
      break;
    }

  if (!ret)
    {
      delete watch_data;
    }

  return ret;
}
