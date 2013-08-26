// PulseMixer.hh
//
// Copyright (C) 2010, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef PULSEMIXER_HH
#define PULSEMIXER_HH

#include "IMixer.hh"

#ifdef HAVE_PULSE

#include <map>

#include <pulse/pulseaudio.h>
#include <pulse/stream.h>
#include <pulse/glib-mainloop.h>

class PulseMixer : public IMixer
{
public:
  PulseMixer();
  virtual ~PulseMixer();

  void init();
  bool set_mute(bool on);

private:
  static void context_state_cb(pa_context *c, void *user_data);
  static void subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *user_data);
  static void server_info_cb(pa_context *c, const pa_server_info *i, void *user_data);
  static void sink_cb(pa_context *c, const pa_sink_info *i, int eol, void *user_data);

  void set_default_sink_name(const char *name);
  void update_sink(const pa_sink_info &info);
  void remove_sink(uint32_t index);

  struct SinkInfo
  {
    bool mute;
    std::string description;
    std::string name;
    uint32_t index;
  };

  pa_glib_mainloop *pa_mainloop;
  pa_mainloop_api *pa_api;
  pa_context *context;

  std::map<uint32_t, SinkInfo*> sinks;
  SinkInfo *default_sink_info;
  std::string default_sink_name;
};

#endif

#endif // PULSEMIXER_HH
