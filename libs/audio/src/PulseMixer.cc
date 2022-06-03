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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "debug.hh"

#include "PulseMixer.hh"

using namespace std;

PulseMixer::~PulseMixer()
{
  pa_context_unref(context);
  pa_glib_mainloop_free(pa_mainloop);
}

bool
PulseMixer::set_mute(bool on)
{
  TRACE_ENTRY_PAR(on);

  bool was_muted = false;

  if (default_sink_info != nullptr)
    {
      was_muted = default_sink_info->mute;
      TRACE_MSG("Was muted {}", was_muted);

      if (was_muted != on)
        {
          pa_operation *o;
          if (!(o = pa_context_set_sink_mute_by_index(context, default_sink_info->index, on, nullptr, nullptr)))
            {
              TRACE_MSG("pa_context_set_sink_mute_by_index failed");
            }
          else
            {
              pa_operation_unref(o);
            }
        }
    }
  return was_muted;
}

void
PulseMixer::init()
{
  TRACE_ENTRY();
  pa_mainloop = pa_glib_mainloop_new(g_main_context_default());
  g_assert(pa_mainloop);

  pa_api = pa_glib_mainloop_get_api(pa_mainloop);
  g_assert(pa_api);

  pa_proplist *pa_proplist = pa_proplist_new();

  pa_proplist_sets(pa_proplist, PA_PROP_APPLICATION_NAME, "Workrave");
  pa_proplist_sets(pa_proplist, PA_PROP_APPLICATION_ID, "org.workrave.Workrave");
  pa_proplist_sets(pa_proplist, PA_PROP_APPLICATION_ICON_NAME, "workrave");
  pa_proplist_sets(pa_proplist, PA_PROP_APPLICATION_VERSION, WORKRAVE_VERSION);

  context = pa_context_new_with_proplist(pa_api, nullptr, pa_proplist);
  g_assert(context);

  pa_proplist_free(pa_proplist);

  pa_context_set_state_callback(context, context_state_cb, this);

  pa_context_connect(context, nullptr, (pa_context_flags_t)0, nullptr);
}

void
PulseMixer::subscribe_cb(pa_context *c, pa_subscription_event_type_t t, uint32_t index, void *user_data)
{
  TRACE_ENTRY();
  auto *pulse = (PulseMixer *)user_data;

  switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
    {
    case PA_SUBSCRIPTION_EVENT_SINK:
      if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
        {
          pulse->remove_sink(index);
        }
      else
        {
          pa_operation *o;
          if (!(o = pa_context_get_sink_info_by_index(c, index, sink_cb, pulse)))
            {
              TRACE_MSG("pa_context_get_sink_info_by_index failed");
              return;
            }
          pa_operation_unref(o);
        }
      break;
    case PA_SUBSCRIPTION_EVENT_SERVER:
      {
        pa_operation *o;
        if (!(o = pa_context_get_server_info(c, server_info_cb, pulse)))
          {
            TRACE_MSG("pa_context_get_server_info failed");
            return;
          }
        pa_operation_unref(o);
      }
    }
}

void
PulseMixer::context_state_cb(pa_context *c, void *user_data)
{
  TRACE_ENTRY();
  auto *pulse = (PulseMixer *)user_data;

  switch (pa_context_get_state(c))
    {
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
      break;

    case PA_CONTEXT_READY:
      {
        pa_operation *o;

        pa_context_set_subscribe_callback(c, subscribe_cb, pulse);

        if (!(o = pa_context_subscribe(c,
                                       (pa_subscription_mask_t)(PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE
                                                                | PA_SUBSCRIPTION_MASK_SINK_INPUT
                                                                | PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT | PA_SUBSCRIPTION_MASK_CLIENT
                                                                | PA_SUBSCRIPTION_MASK_SERVER),
                                       nullptr,
                                       nullptr)))
          {
            TRACE_MSG("pa_context_subscribe failed");
            return;
          }
        pa_operation_unref(o);

        if (!(o = pa_context_get_server_info(c, server_info_cb, pulse)))
          {
            TRACE_MSG("pa_context_get_server_info failed");
            return;
          }
        pa_operation_unref(o);

        if (!(o = pa_context_get_sink_info_list(c, sink_cb, pulse)))
          {
            TRACE_MSG("pa_context_get_sink_info_list failed");
            return;
          }
        pa_operation_unref(o);

        break;
      }
    case PA_CONTEXT_TERMINATED:
      break;
    case PA_CONTEXT_FAILED:
    default:
      TRACE_MSG("Connection failure: {}", pa_strerror(pa_context_errno(c)));
    }
}

void
PulseMixer::server_info_cb(pa_context *, const pa_server_info *i, void *user_data)
{
  TRACE_ENTRY();
  auto *pulse = (PulseMixer *)user_data;
  pulse->set_default_sink_name(i->default_sink_name ? i->default_sink_name : "");
}

void
PulseMixer::sink_cb(pa_context *, const pa_sink_info *i, int eol, void *user_data)
{
  TRACE_ENTRY();
  auto *pulse = (PulseMixer *)user_data;

  if (eol == 0)
    {
      pulse->update_sink(*i);
    }
}

void
PulseMixer::set_default_sink_name(const char *name)
{
  TRACE_ENTRY_PAR(name);

  default_sink_name = name;

  for (auto &sink: sinks)
    {
      SinkInfo *sink_info = sink.second;

      if (sink_info != nullptr && sink_info->name == default_sink_name)
        {
          TRACE_MSG("New default sink");
          default_sink_info = sink_info;
        }
    }
}

void
PulseMixer::remove_sink(uint32_t index)
{
  TRACE_ENTRY_PAR(index);
  if (sinks.count(index))
    {
      if (sinks[index] == default_sink_info)
        {
          TRACE_MSG("Lost default sink");
          default_sink_info = nullptr;
        }
      delete sinks[index];
      sinks.erase(index);
    }
}

void
PulseMixer::update_sink(const pa_sink_info &info)
{
  TRACE_ENTRY();
  SinkInfo *sink_info = nullptr;

  if (sinks.count(info.index))
    {
      sink_info = sinks[info.index];
    }
  else
    {
      sink_info = new SinkInfo();
      sinks[info.index] = sink_info;
    }

  sink_info->index = info.index;
  sink_info->name = info.name;
  sink_info->description = info.description;
  sink_info->mute = info.mute;

  TRACE_VAR(info.name, info.mute, info.index);

  if (sink_info->name == default_sink_name)
    {
      default_sink_info = sink_info;
    }
}
