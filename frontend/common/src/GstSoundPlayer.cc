// GstSoundPlayer.cc --- Sound player
//
// Copyright (C) 2002, 2003, 2004, 2006, 2007, 2008 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id: GstSoundPlayer.cc 1470 2008-03-09 19:49:45Z rcaelers $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_GSTREAMER

#include "debug.hh"

#include "IConfigurator.hh"
#include "ICore.hh"
#include "CoreFactory.hh"

#include "GstSoundPlayer.hh"
#include "Sound.hh"
#include "Util.hh"
#include <debug.hh>

using namespace std;
using namespace workrave;

static gboolean bus_watch(GstBus *bus, GstMessage *msg, gpointer data);

GstSoundPlayer::GstSoundPlayer() :
  volume(0.9),
  gst_ok(false)
{
	GError *error = NULL;

  gst_ok = gst_init_check(NULL, NULL, &error);
  
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
  TRACE_EXIT();
}


bool
GstSoundPlayer::capability(SoundPlayer::SoundCapability cap)
{
  (void) cap;
  return false;
}

void
GstSoundPlayer::play_sound(SoundPlayer::SoundEvent snd)
{
  TRACE_ENTER_MSG("GstSoundPlayer::play_sound", snd);
  TRACE_EXIT();
}


void
GstSoundPlayer::play_sound(std::string wavfile)
{
  TRACE_ENTER_MSG("GstSoundPlayer::play_sound", wavfile);

	GstElement *sink = NULL;
	GstElement *play = NULL;
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
      bus = gst_pipeline_get_bus(GST_PIPELINE(play));
      gst_bus_add_watch(bus, bus_watch, play);
  
      char *uri = g_strdup_printf("file://%s", wavfile.c_str());

      int volume = 100;
      CoreFactory::get_configurator()->get_value(SoundPlayer::CFG_KEY_SOUND_VOLUME, volume);

      TRACE_MSG((float)volume);
      gst_element_set_state(play, GST_STATE_NULL);
      
      g_object_set(G_OBJECT(play),
                   "uri", uri,
                   "volume", (float)(volume / 100.0),
                   "audio-sink", sink, NULL);
      
      gst_element_set_state(play, GST_STATE_PLAYING);

      gst_object_unref(bus);
      g_free(uri);
    }

  TRACE_EXIT();
}


static gboolean
bus_watch(GstBus *bus, GstMessage *msg, gpointer data)
{
  TRACE_ENTER("GstSoundPlayer::bus_watch");
  GstElement *play = (GstElement *) data;
  GError *err = NULL;

  (void) bus;
  
  switch (GST_MESSAGE_TYPE (msg))
    {
    case GST_MESSAGE_ERROR:
      gst_message_parse_error(msg, &err, NULL);
      TRACE_MSG(err->message);
      g_error_free(err);
      /* FALLTHROUGH */
      
    case GST_MESSAGE_EOS:
      gst_element_set_state(play, GST_STATE_NULL);
      gst_object_unref(GST_OBJECT(play));
      break;
      
    case GST_MESSAGE_WARNING:
      gst_message_parse_warning(msg, &err, NULL);
      TRACE_MSG(err->message);
      g_error_free(err);
      break;

    default:
      break;
    }
  
  TRACE_EXIT();
  
  return TRUE;
}

bool
GstSoundPlayer::get_sound_enabled(SoundPlayer::SoundEvent snd, bool &enabled)
{
  (void) snd;
  (void) enabled;
  
  return false;
}

void
GstSoundPlayer::set_sound_enabled(SoundPlayer::SoundEvent snd, bool enabled)
{
  (void) snd;
  (void) enabled;
}

bool
GstSoundPlayer::get_sound_wav_file(SoundPlayer::SoundEvent snd, std::string &wav_file)
{
  (void) snd;
  (void) wav_file;
  return false;
}

void
GstSoundPlayer::set_sound_wav_file(SoundPlayer::SoundEvent snd, const std::string &wav_file)
{
  (void) snd;
  (void) wav_file
    ;
}

#endif
