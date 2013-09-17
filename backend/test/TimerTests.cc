// Copyright (C) 2013 Rob Caelers
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
#include "config.h"
#endif

#define BOOST_TEST_MODULE workrave
#include <boost/test/included/unit_test.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signals2.hpp>
#include <boost/lexical_cast.hpp>

#include "utils/ITimeSource.hh"
#include "utils/TimeSource.hh"

#include "Timer.hh"

using namespace std;
using namespace workrave::utils;
using namespace workrave;

class SimulatedTime : public ITimeSource, public boost::enable_shared_from_this<SimulatedTime>
{
public:
  typedef boost::shared_ptr<SimulatedTime> Ptr;
  
  void init()
  {
    current_time = TimeSource::get_monotonic_time_usec(); 
    TimeSource::source = shared_from_this();
  }
  
  virtual int64_t get_real_time_usec()
  {
    return current_time;
  }
  
  virtual int64_t get_monotonic_time_usec()
  {
    return current_time;
  }

  int64_t current_time;
};

class Fixture
{
public:
  Fixture() 
  {
    sim = SimulatedTime::Ptr(new SimulatedTime);
    timer = Timer::create("test");
  }
  
  ~Fixture()
  {
  }

  void init()
  {
    sim->init();
    TimeSource::sync();

    timer->set_limit(100);
    timer->set_limit_enabled(true);
    timer->set_auto_reset(20);
    timer->set_auto_reset_enabled(true);
    timer->set_snooze(50);
    timer->enable();
  }

  void tick(bool active)
  {
    TimeSource::sync();
    timer->process(active);
    sim->current_time += 1000000;
  }

  void tick(bool active, int seconds, const std::function<void(int count)> &check_func)
  {
    for (int i = 0; i < seconds; i++)
      {
        try
          {
            TimeSource::sync();
            timer->process(active);
            check_func(i);
            sim->current_time += 1000000;
          }
        catch (...)
          {
            BOOST_TEST_MESSAGE(string ("error at:") + boost::lexical_cast<string>(i)); 
            std::cout << "error at:" << i << std::endl;
            throw;
          }
      }
  }

  void tick(bool active, int seconds, const std::function<void(int count, TimerEvent event)> &check_func)
  {
    for (int i = 0; i < seconds; i++)
      {
        try
          {
          TimeSource::sync();
          TimerEvent event = timer->process(active);
          check_func(i, event);
          sim->current_time += 1000000;
        }
        catch (...)
          {
            BOOST_TEST_MESSAGE(string ("error at:") + boost::lexical_cast<string>(i)); 
            std::cout << "error at:" << i << std::endl;
           throw;
          }
      }
  }
  
  SimulatedTime::Ptr sim;
  Timer::Ptr timer;
};

BOOST_FIXTURE_TEST_SUITE(s, Fixture)

BOOST_AUTO_TEST_CASE(test_timer_get_id)
{
  init();
  BOOST_REQUIRE_EQUAL(timer->get_id(), "test");
}

BOOST_AUTO_TEST_CASE(test_timer_elasped_time)
{
  init();

  tick(false, 200, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    });

  tick(true, 200, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    });

  tick(false, 20, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 200);
    });

  tick(false, 200, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    });
}


BOOST_AUTO_TEST_CASE(test_timer_elasped_idle_time)
{
  init();

  tick(false, 200, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 20 + count);
    });

  tick(true, 100, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    });

  tick(false, 200, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    });

  tick(true, 100, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_total_overdue_time)
{
  init();

  tick(false, 200, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    });

  tick(true, 100, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    });

  tick(true, 200, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
    });

  tick(false, 21, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200);
    });

  tick(true, 100, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200);
    });

  tick(true, 200, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200 + count);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_daily_reset_while_active)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  tick(true, 200, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), count);
    });

  tick(false, 21, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 200);
    });

  tick(true, 50, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 200);
    });

  timer->daily_reset_timer();
  
  tick(true, 50, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    });

  tick(true, 50, [=](int count) {
      // FIXME: '0' sounds more logical then 'count'
      // However, the current overdue time is not reset, only the total
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_daily_reset_while_idle)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  tick(true, 200, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), count);
    });

  tick(false, 21, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 200);
    });

  timer->daily_reset_timer();
  
  tick(true, 100, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    });

  tick(true, 200, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_stop_timer_while_active)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
    });

  timer->stop_timer();
  
  tick(true, 100, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 99 + count);
    });

}

BOOST_AUTO_TEST_CASE(test_timer_stop_timer_while_idle)
{
  init();

  tick(true, 200, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
    });

  tick(false, 20, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), 200);
    });

  timer->stop_timer();
  
  tick(false, 200, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_reset_timer_while_active)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  timer->reset_timer();
  
  tick(true, 100, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 1 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_reset_timer_while_idle)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  tick(false, 10, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), count);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });
  
  timer->reset_timer();
  
  tick(false, 100, [=](int count) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 21 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_freeze_timer_when_active)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  timer->freeze_timer(true);
  
  tick(false, 5, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 99);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      // FIXME: The limit reached is unexpected
      BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    });

  tick(true, 60, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 99);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 5);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  timer->freeze_timer(false);
  
  tick(true, 50, [=](int count, TimerEvent event) {
      // FIXME: why increased to 100?
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
      // FIXME: why two events?
      BOOST_REQUIRE_EQUAL(event, (count == 0 || count == 1) ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_freeze_timer_when_idle)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  tick(false, 5, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), count);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });
  
  timer->freeze_timer(true);
  
  tick(false, 5, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 5 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 60, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  timer->freeze_timer(false);
  
  tick(false, 5, [=](int count, TimerEvent event) {
      // FIXME: why did elapsed time increase to 101?
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 101);
      // FIXME: why did idle time reset to 0?
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
      // FIXME: why did overdue time increase to 1
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 1);
      // FIXME: why the limit reached?
      BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    });

  tick(true, 50, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 101 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 1 + count);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_raise_limit_while_idle)
{
  init();

  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  tick(false, 5, [=](int count, TimerEvent event) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), count);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });
  
  timer->set_limit(160);
  
  tick(false, 5, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 5 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 60, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 50, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 160 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
      // FIXME: why 2 events.
      BOOST_REQUIRE_EQUAL(event, (count == 0 || count == 1) ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_lower_limit_while_idle)
{
  init();

  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  tick(false, 10, [=](int count, TimerEvent event) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), count);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });
  
  timer->set_limit(40);
  
  tick(false, 10, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 60);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(false, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 20 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 60);
      BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
    });
}


BOOST_AUTO_TEST_CASE(test_timer_raise_limit_while_active)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  timer->set_limit(160);
  
  tick(true, 60, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 50, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 160 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
      // FIXME: why 2 events.
      BOOST_REQUIRE_EQUAL(event, (count == 0 || count == 1) ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_lower_limit_while_active)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  timer->set_limit(125);
  
  tick(true, 25, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 25, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 125 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
      // FIXME: why two events.
      BOOST_REQUIRE_EQUAL(event, (count == 0 || count == 1) ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_lower_limit_to_in_the_past_while_active)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  timer->set_limit(60);
  
  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 40 + count);
      BOOST_REQUIRE_EQUAL(event, (count == 0 || count == 50) ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_raise_auto_reset_while_idle)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  tick(false, 5, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), count);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });
  
  timer->set_auto_reset(40);
  
  tick(false, 35, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 5 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(false, 50, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 40 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_lower_auto_reset_while_idle)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  tick(false, 10, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), count);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });
  
  timer->set_auto_reset(15);
  
  tick(false, 5, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(false, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 15 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_lower_auto_reset_to_in_the_past_while_idle)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  tick(false, 10, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), count);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });
  
  timer->set_auto_reset(6);
  
  tick(false, 10, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
      // FIXME: why 6?
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 6 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
    });
}


BOOST_AUTO_TEST_CASE(test_timer_raise_auto_reset_while_active)
{
  init();

  tick(true, 50, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  timer->set_auto_reset(40);
  
  tick(true, 50, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(false, 40, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      // FIXME: why the limit reached, user is idle
      BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    });

  tick(false, 40, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 40 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_lower_auto_reset_while_active)
{
  init();

  tick(true, 100, [=](int count) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
    });

  timer->set_auto_reset(10);
  
  tick(true, 40, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
      // FIXME: why the limit reached, user is idle
      // FIXME: why two events
      BOOST_REQUIRE_EQUAL(event, (count == 0 || count == 1) ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    });

  tick(false, 10, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 140);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 40);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(false, 10, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 40);
      BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_limit_reached_event)
{
  init();

  tick(false, 10, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 1, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED);
    });

  // FIXME: why is the event raised twice??
  tick(true, 1, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED);
    });
  
  tick(true, 10, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_limit_reached_snoozed_event)
{
  init();

  tick(false, 10, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 1, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED);
    });

  // FIXME: why is the event raised twice??
  tick(true, 1, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED);
    });

  tick(true, 49, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 1, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED);
    });

  // FIXME: why 49?
  tick(true, 49, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 1, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED);
    });

  tick(true, 49, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_limit_reached_snoozed_event_when_snoozed)
{
  init();

  tick(false, 10, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 1, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED);
    });

  // FIXME: why is the event raised twice??
  tick(true, 1, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED);
    });

  tick(true, 40, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  timer->snooze_timer();
  
  tick(true, 49, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 1, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED);
    });

  tick(true, 30, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  timer->snooze_timer();
  
  // FIXME: why 49?
  tick(true, 49, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 1, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED);
    });

  tick(true, 49, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_limit_reached_inhibit_snooze_event)
{
  init();

  tick(false, 10, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 1, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED);
    });

  // FIXME: why is the event raised twice??
  tick(true, 1, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED);
    });

  timer->inhibit_snooze();
  
  tick(true, 200, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_when_not_over_limit_and_active_then_enable)
{
  init();

  tick(true, 50, [=](int count) {
    });

  timer->disable();
  
  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 49);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(false, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 49);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  timer->enable();
  
  tick(true, 50, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 49 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

}


BOOST_AUTO_TEST_CASE(test_timer_disable_when_over_limit_and_active_then_enable)
{
  init();

  tick(true, 150, [=](int count) {
    });

  timer->disable();
  
  tick(false, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 149);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 49);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 149);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 49);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  timer->enable();
  
  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 149 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 49 + count);
      BOOST_REQUIRE_EQUAL(event, (count == 0 || count == 50) ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    });

}


BOOST_AUTO_TEST_CASE(test_timer_disable_when_not_over_limit_and_idle_then_enable)
{
  init();

  tick(true, 50, [=](int count) {
    });

  tick(false, 5, [=](int count) {
    });
  
  timer->disable();
  
  tick(false, 5, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0); // FIXME: 5
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  timer->enable();
  
  tick(true, 50, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

}

BOOST_AUTO_TEST_CASE(test_timer_disable_when_over_limit_and_idle_then_enable)
{
  init();

  tick(true, 150, [=](int count) {
    });

  tick(false, 5, [=](int count) {
    });

  timer->disable();
  
  tick(false, 5, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(false, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  timer->enable();
  
  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50 + count);
      BOOST_REQUIRE_EQUAL(event, (count == 0 || count == 50) ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    });

}

BOOST_AUTO_TEST_CASE(test_timer_disable_limit_when_active)
{
  init();

  tick(true, 50, [=](int count) {
    });

  timer->set_limit_enabled(false);
  
  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });
}


BOOST_AUTO_TEST_CASE(test_timer_disable_limit_when_idle)
{
  init();

  tick(true, 50, [=](int count) {
    });

  tick(false, 50, [=](int count) {
    });

  timer->set_limit_enabled(false);

  tick(false, 50, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });
}


BOOST_AUTO_TEST_CASE(test_timer_enable_limit_when_active_and_not_overdue)
{
  init();

  tick(false, 50, [=](int count) {
    });

  timer->set_limit_enabled(false);
  
  tick(true, 50, [=](int count, TimerEvent event) {
      BOOST_CHECK_EQUAL(timer->get_elapsed_time(), count);
      BOOST_CHECK_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  timer->set_limit_enabled(true);
  
  tick(true, 50, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_enable_limit_when_active_and_overdue)
{
  init();

  tick(false, 50, [=](int count) {
    });

  timer->set_limit_enabled(false);
  
  tick(true, 150, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  timer->set_limit_enabled(true);
  
  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50 + count);
      BOOST_REQUIRE_EQUAL(event, (count == 0 || count == 50) ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_enable_limit_when_idle)
{
  init();

  tick(true, 50, [=](int count) {
    });

  timer->set_limit_enabled(false);

  tick(false, 50, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(event, count == 20 ? TIMER_EVENT_RESET : TIMER_EVENT_NONE);
    });

  timer->set_limit_enabled(true);

  tick(false, 50, [=](int count, TimerEvent event) {
    });

  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_limit_when_overdue)
{
  init();

  tick(true, 201, [=](int count) {
    });

  BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 100);

  tick(false, 50, [=](int count) {
    });
    
  tick(true, 201, [=](int count) {
    });

  // FIXME: first loop of 201 step incresaes overdue time by 100.
  // FIXME: second loop incresaes overdue time by 101.
  // FIXME: Why?
  BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 201);
  
  timer->set_limit_enabled(false);
  
  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 201 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);

      // FIXME: Expected 201. But over due that is 'in progress' is not counted.
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 101);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_limit_when_not_overdue)
{
  init();

  tick(true, 201, [=](int count) {
    });

  BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 100);

  tick(false, 50, [=](int count) {
    });
    
  tick(true, 50, [=](int count) {
    });

  timer->set_limit_enabled(false);
  
  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 101);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_while_active)
{
  init();

  tick(true, 201, [=](int count) {
    });

  BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 100);

  tick(false, 50, [=](int count) {
    });
    
  tick(true, 201, [=](int count) {
    });

  // FIXME: first loop of 201 step incresaes overdue time by 100.
  // FIXME: second loop incresaes overdue time by 101.
  // FIXME: Why?
  BOOST_CHECK_EQUAL(timer->get_total_overdue_time(), 201);
  
  timer->set_limit_enabled(false);
  
  tick(true, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 201 + count);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);

      // FIXME: Expected 201. But over due that is 'in progress' is not counted.
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 101);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_reset)
{
  init();

  tick(true, 150, [=](int count) {
    });

  tick(false, 10, [=](int count) {
    });

  timer->set_auto_reset_enabled(false);
  
  tick(false, 100, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_enable_reset_when_reset_passed)
{
  init();

  tick(true, 150, [=](int count) {
    });

  tick(false, 10, [=](int count) {
    });

  timer->set_auto_reset_enabled(false);

  tick(false, 50, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  timer->set_auto_reset_enabled(true);
  
  tick(false, 5, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
      // FIXME: why is this set to 20 instead of 60?
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 20 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
      BOOST_REQUIRE_EQUAL(event,  count == 0 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_CASE(test_timer_enable_reset)
{
  init();

  tick(true, 150, [=](int count) {
    });

  tick(false, 10, [=](int count) {
    });

  timer->set_auto_reset_enabled(false);

  tick(false, 5, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  timer->set_auto_reset_enabled(true);
  
  tick(false, 5, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 15 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
      BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    });

  tick(false, 5, [=](int count, TimerEvent event) {
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
      BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 20 + count);
      BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
      // FIXME: whyis this a natural break?
      BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
    });
}

BOOST_AUTO_TEST_SUITE_END()
