//
// Copyright (C) 2012, 2013 Rob Caelers
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

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include "debug.hh"
#include "utils/Timer.hh"

using namespace workrave::utils;
using namespace std;

static Mutex g_timer_mutex;
Timer::Ptr Timer::instance;

Timer::Ptr
Timer::get()
{
  g_timer_mutex.lock();
  if (!instance)
    {
      instance = Timer::Ptr(new Timer());
    }
  g_timer_mutex.unlock();
  return instance;
}

Timer::Timer()
#ifdef HAVE_TESTS
  : simulated(false)
  , current_time(0)
#endif
{
}

void
Timer::create(string name, int64_t interval, Callback callback)
{
  TRACE_ENTER_MSG("Timer::create", name << " " << interval);
  g_timer_mutex.lock();

  string id = name + "." + boost::lexical_cast<std::string>(boost::this_thread::get_id());

  TRACE_MSG("id = " << id);
  if (timers.find(id) == timers.end())
    {
      timers[id] = new Info();
    }

  Info *info     = timers[id];
  info->callback = callback;

#ifdef HAVE_TESTS
  info->context = g_main_context_get_thread_default();
#endif

  if (info->interval != interval)
    {
      info->interval = interval;

      if (info->source != NULL)
        {
          g_source_destroy(info->source);
          info->source = NULL;
        }

      if (interval != 0
#ifdef HAVE_TESTS
          && !simulated
#endif
      )
        {
          info->source = g_timeout_source_new(interval);
          g_source_set_callback(info->source, static_on_timer, info, NULL);
          g_source_attach(info->source, g_main_context_get_thread_default());
        }
    }
  g_timer_mutex.unlock();
  TRACE_EXIT();
}

void
Timer::destroy(string name)
{
  TRACE_ENTER_MSG("Timer::destroy", name);
  g_timer_mutex.lock();
  string id = name + "." + boost::lexical_cast<std::string>(boost::this_thread::get_id());
  TRACE_MSG("id = " << id);

  if (timers.find(id) != timers.end())
    {
      Info *info = timers[name];

      if (info->source != NULL)
        {
          g_source_destroy(info->source);
          info->source = NULL;
        }

      timers.erase(name);
      delete info;
    }
  g_timer_mutex.unlock();
  TRACE_EXIT();
}

gboolean
Timer::static_on_timer(gpointer data)
{
  Info *info = (Info *)data;

  info->callback();

  return G_SOURCE_CONTINUE;
}

#ifdef HAVE_TESTS

gboolean
Timer::static_on_idle(gpointer data)
{
  Info *info = (Info *)data;

  info->callback();
  info->syncer->notify();
  return FALSE;
}

int64_t
Timer::get_real_time_usec()
{
  return current_time;
}

int64_t
Timer::get_monotonic_time_usec()
{
  return current_time;
}

void
Timer::set_simulated(bool on)
{
  TRACE_ENTER_MSG("Timer::set_simulated", on);
  g_timer_mutex.lock();
  simulated = on;
  if (on)
    {
      TimeSource::source = shared_from_this();
      current_time       = g_get_monotonic_time();

      for (TimerMapIter i = timers.begin(); i != timers.end(); i++)
        {
          if (i->second->source != NULL)
            {
              g_source_destroy(i->second->source);
              i->second->source = NULL;
            }
        }
    }
  else
    {
      TimeSource::source.reset();

      for (TimerMapIter i = timers.begin(); i != timers.end(); i++)
        {
          i->second->source = g_timeout_source_new(i->second->interval);
          g_source_set_callback(i->second->source, static_on_timer, i->second, NULL);
          g_source_attach(i->second->source, i->second->context);
        }
    }
  g_timer_mutex.unlock();
  TRACE_EXIT()
}

void
Timer::simulate(int64_t usec, int64_t delay)
{
  TRACE_ENTER("Timer::simulate");
  int64_t last = current_time + usec;

  int64_t smallest = -1;
  while (current_time < last)
    {
      g_timer_mutex.lock();
      list<Info *> timers_to_call;
      for (TimerMapIter i = timers.begin(); i != timers.end(); i++)
        {
          if (current_time >= i->second->next)
            {
              timers_to_call.push_back(i->second);
              i->second->next = current_time + i->second->interval;
            }

          if (smallest == -1 || (i->second->next - current_time) < smallest)
            {
              smallest = i->second->interval;
            }
        }
      g_timer_mutex.unlock();

      CallbackSyncer syncer(timers_to_call.size());

      for (list<Info *>::iterator i = timers_to_call.begin(); i != timers_to_call.end(); i++)
        {
          (*i)->syncer = &syncer;
          g_main_context_invoke_full((*i)->context, G_PRIORITY_DEFAULT_IDLE, static_on_idle, *i, NULL);
        }

      syncer.wait();
      current_time += smallest;
      g_usleep(delay);
    }
  TRACE_EXIT();
}

#endif
