// Timer.hh --- The Time
//
// Copyright (C) 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UTILS_TIMER_HH
#define WORKRAVE_UTILS_TIMER_HH

#include <string>
#include <map>

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>

#include <glib.h>
#include <glib-object.h>

#include "utils/ITimeSource.hh"
#include "utils/TimeSource.hh"

namespace workrave
{
  namespace utils
  {
#ifdef HAVE_TESTS
    class CallbackSyncer
    {
    public:
      CallbackSyncer(unsigned long count) : count(count) { }

      void notify()
      {
        TRACE_ENTER("Timer::notify");
        boost::mutex::scoped_lock lock(mutex);
        count--;
        condition.notify_one();
        TRACE_MSG(count);
        TRACE_EXIT();
      }

      void wait()
      {
        TRACE_ENTER("Timer::wait");
        boost::mutex::scoped_lock lock(mutex);
        while (count)
          {
            condition.wait(lock);
            TRACE_MSG(count);
          }
        TRACE_EXIT();
      }

    private:
      boost::mutex mutex;
      boost::condition_variable condition;
      unsigned long count;
    };
#endif

    //! A periodic timer.
    class Timer :
#ifdef HAVE_TESTS
      public ITimeSource,
#endif
      public std::enable_shared_from_this<Timer>
    {
    public:
      typedef std::shared_ptr<Timer> Ptr;
      typedef boost::function<void()> Callback;

      static Timer::Ptr get();
      Timer();

      //!
      void create(std::string name, gint64 interval, Callback callback);

      //!
      void destroy(std::string name);

#ifdef HAVE_TESTS
      void set_simulated(bool on);
      void simulate(gint64 usec, gint64 delay = 100);

      virtual gint64 get_real_time_usec();
      virtual gint64 get_monotonic_time_usec();
    private:
      static gboolean static_on_idle(gpointer data);
#endif

    private:
      static gboolean static_on_timer(gpointer data);

      class Info
      {
      public:
        Info() : interval(0), source(NULL)
#ifdef HAVE_TESTS
               , next(0), context(NULL), syncer(NULL)
#endif
        {}

        gint64 interval;
        GSource *source;
        Callback callback;

#ifdef HAVE_TESTS
        gint64 next;
        GMainContext *context;
        CallbackSyncer *syncer;
#endif
      };

      typedef std::map<std::string, Info*> TimerMap;
      typedef TimerMap::iterator TimerMapIter;

      static Timer::Ptr instance;

      TimerMap timers;
#ifdef HAVE_TESTS
      bool simulated;
      gint64 current_time;
#endif
    };
  }
}

#endif // WORKRAVE_UTILS_TIMER_HH
