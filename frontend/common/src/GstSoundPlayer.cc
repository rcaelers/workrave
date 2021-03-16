// GstSoundPlayer.cc --- Sound player
//
// Copyright (C) 2002 - 2011, 2013 Rob Caelers & Raymond Penners
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

#ifdef HAVE_GSTREAMER

#  include "debug.hh"

#  include "IConfigurator.hh"
#  include "ICore.hh"
#  include "CoreFactory.hh"

#  include "GstSoundPlayer.hh"
#  include "SoundPlayer.hh"
#  include "Sound.hh"
#  include "Util.hh"
#  include <debug.hh>

using namespace std;
using namespace workrave;

GstSoundPlayer::GstSoundPlayer()
  : gst_ok(false)
{
  GError *error = NULL;

  gst_ok = gst_init_check(NULL, NULL, &error);
  gst_registry_fork_set_enabled(FALSE);

  if (!gst_ok)
    {
      if (error)
        {
          g_error_free(error);
          error = NULL;
        }
    }
}

GstSoundPlayer::~GstSoundPlayer()
{
  TRACE_ENTER("GstSoundPlayer::~GstSoundPlayer");
  if (gst_ok)
    {
      gst_deinit();
    }
  TRACE_EXIT();
}

void
GstSoundPlayer::init(ISoundDriverEvents *events)
{
  this->events = events;
}

bool
GstSoundPlayer::capability(SoundCapability cap)
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
GstSoundPlayer::play_sound(std::string wavfile)
{
  TRACE_ENTER_MSG("GstSoundPlayer::play_sound", wavfile);

  GstElement *play = NULL;
  GstElement *sink = NULL;
  GstBus *bus = NULL;

  string method = "automatic";

  if (method == "automatic")
    {
      if (Util::running_gnome())
        {
          sink = gst_element_factory_make("gconfaudiosink", "sink");
        }
      if (!sink)
        {
          sink = gst_element_factory_make("autoaudiosink", "sink");
        }
    }
  else if (method == "esd")
    {
      sink = gst_element_factory_make("esdsink", "sink");
    }
  else if (method == "alsa")
    {
      sink = gst_element_factory_make("alsasink", "sink");
    }

  if (sink != NULL)
    {
      play = gst_element_factory_make("playbin", "play");
    }

  if (play != NULL)
    {
      WatchData *watch_data = new WatchData;
      watch_data->player = this;
      watch_data->play = play;

      bus = gst_pipeline_get_bus(GST_PIPELINE(play));
      gst_bus_add_watch(bus, bus_watch, watch_data);

      char *uri = g_strdup_printf("file://%s", wavfile.c_str());

      int volume = 100;
      CoreFactory::get_configurator()->get_value(SoundPlayer::CFG_KEY_SOUND_VOLUME, volume);

      TRACE_MSG((float)volume);
      gst_element_set_state(play, GST_STATE_NULL);

      g_object_set(G_OBJECT(play), "uri", uri, "volume", (float)(volume / 100.0), "audio-sink", sink, NULL);

      gst_element_set_state(play, GST_STATE_PLAYING);

      gst_object_unref(bus);
      g_free(uri);
    }

  TRACE_EXIT();
}

gboolean
GstSoundPlayer::bus_watch(GstBus *bus, GstMessage *msg, gpointer data)
{
  WatchData *watch_data = (WatchData *)data;
  GstElement *play = watch_data->play;
  GError *err = NULL;
  gboolean ret = TRUE;

  (void)bus;

  switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_ERROR:
      gst_message_parse_error(msg, &err, NULL);
      g_error_free(err);
      /* FALLTHROUGH */

    case GST_MESSAGE_EOS:
      gst_element_set_state(play, GST_STATE_NULL);
      gst_object_unref(GST_OBJECT(play));
      ret = FALSE;

      if (watch_data->player->events != NULL)
        {
          watch_data->player->events->eos_event();
        }
      break;

    case GST_MESSAGE_WARNING:
      gst_message_parse_warning(msg, &err, NULL);
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

#endif
