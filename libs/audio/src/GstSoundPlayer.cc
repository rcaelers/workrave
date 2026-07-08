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
  if (cap == workrave::audio::SoundCapability::VOLUME || cap == workrave::audio::SoundCapability::EOS_EVENT)
    {
      return true;
    }
  if (cap == workrave::audio::SoundCapability::DEVICE)
    {
      return true;
    }
  return false;
}

void
GstSoundPlayer::play_sound(std::string wavfile, int volume)
{
  TRACE_ENTRY_PAR(wavfile, volume);

  GstElement *play = nullptr;
  GstElement *sink = nullptr;

  if (!current_device.empty() && current_device != "default")
    {
      sink = gst_element_factory_make("autoaudiosink", "sink");
      if (sink != nullptr)
        {
          g_object_set(G_OBJECT(sink), "device", current_device.c_str(), NULL);
        }
    }
  else
    {
      sink = gst_element_factory_make("autoaudiosink", "sink");
    }

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

std::vector<workrave::audio::SoundDevice>
GstSoundPlayer::get_devices()
{
  if (!devices_cache.empty())
    {
      return devices_cache;
    }

  devices_cache.push_back({"default", "System Default", true});

  GstDeviceMonitor *monitor = gst_device_monitor_new();
  GstCaps *caps = gst_caps_new_empty_simple("audio/x-raw");
  gst_device_monitor_add_filter(monitor, "Audio/Sink", caps);
  gst_caps_unref(caps);

  if (gst_device_monitor_start(monitor))
    {
      GList *devices = gst_device_monitor_get_devices(monitor);
      for (GList *l = devices; l != nullptr; l = l->next)
        {
          auto *device = GST_DEVICE(l->data);
          auto *props = gst_device_get_properties(device);
          const char *display_name = gst_structure_get_string(props, "device.description");
          const char *device_name = gst_structure_get_string(props, "device.name");
          if (device_name && display_name)
            {
              devices_cache.push_back({device_name, display_name, false});
            }
          gst_structure_free(props);
        }
      g_list_free_full(devices, gst_object_unref);
      gst_device_monitor_stop(monitor);
    }

  gst_object_unref(monitor);
  return devices_cache;
}

void
GstSoundPlayer::set_device(const std::string &device_id)
{
  current_device = device_id;
}

std::string
GstSoundPlayer::get_device() const
{
  return current_device;
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
