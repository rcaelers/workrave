// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#define BOOST_TEST_MODULE workrave_timer
#include <boost/test/unit_test.hpp>

#include <boost/signals2.hpp>
#include <boost/lexical_cast.hpp>

#include "utils/ITimeSource.hh"
#include "utils/TimeSource.hh"

#include "Timer.hh"
#include "TimePred.hh"
#include "SimulatedTime.hh"

using namespace std;
using namespace workrave::utils;
using namespace workrave;

class TestTimePred : public TimePred
{
public:
  void set(int64_t t)
  {
    next = t;
  }

  time_t get_next(time_t last_time)
  {
    if (next <= last_time)
      {
        next += 100;
      }
    return next;
  }

  std::string to_string() const
  {
    return "xx";
  }

  int64_t next{0};
};

class Fixture
{
public:
  Fixture()
  {
    sim = SimulatedTime::create();
  }

  ~Fixture() = default;

  void init()
  {
    sim->reset();
    TimeSource::sync();
    timer = make_timer();
  }

  Timer::Ptr make_timer()
  {
    timer = std::make_shared<Timer>("test");

    timer->set_limit(100);
    timer->set_limit_enabled(true);
    timer->set_auto_reset(20);
    timer->set_auto_reset_enabled(true);
    timer->set_snooze(50);
    timer->enable();
    return timer;
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
            BOOST_TEST_CONTEXT("Timer")
            {
              BOOST_TEST_INFO_SCOPE("Count:" << i);
              BOOST_TEST_INFO_SCOPE("Active: " << timer->get_elapsed_time());
              BOOST_TEST_INFO_SCOPE("Idle:   " << timer->get_elapsed_idle_time());
              BOOST_TEST_INFO_SCOPE("Overdue " << timer->get_total_overdue_time());
              check_func(i);
            }
            sim->current_time += 1000000;
          }
        catch (...)
          {
            BOOST_TEST_MESSAGE(string("error at:") + boost::lexical_cast<string>(i));
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
            BOOST_TEST_CONTEXT("Timer")
            {
              BOOST_TEST_INFO_SCOPE("Count:" << i);
              BOOST_TEST_INFO_SCOPE("Event:" << event);

              BOOST_TEST_INFO_SCOPE("Active: " << timer->get_elapsed_time());
              BOOST_TEST_INFO_SCOPE("Idle:   " << timer->get_elapsed_idle_time());
              BOOST_TEST_INFO_SCOPE("Overdue " << timer->get_total_overdue_time());
              check_func(i, event);
            }
            sim->current_time += 1000000;
          }
        catch (...)
          {
            BOOST_TEST_MESSAGE(string("error at:") + boost::lexical_cast<string>(i));
            std::cout << "error at:" << i << std::endl;
            throw;
          }
      }
  }

  SimulatedTime::Ptr sim;
  Timer::Ptr timer;
};

BOOST_FIXTURE_TEST_SUITE(timer, Fixture)

BOOST_AUTO_TEST_CASE(test_timer_get_id)
{
  init();
  BOOST_REQUIRE_EQUAL(timer->get_id(), "test");
}

BOOST_AUTO_TEST_CASE(test_timer_initial_settings)
{
  init();
  BOOST_REQUIRE_EQUAL(timer->get_limit(), 100);
  BOOST_REQUIRE_EQUAL(timer->is_limit_enabled(), true);
  BOOST_REQUIRE_EQUAL(timer->get_auto_reset(), 20);
  BOOST_REQUIRE_EQUAL(timer->is_auto_reset_enabled(), true);
  BOOST_REQUIRE_EQUAL(timer->get_snooze(), 50);
}

BOOST_AUTO_TEST_CASE(test_timer_elasped_time)
{
  init();

  tick(false, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0); });

  tick(true, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count); });

  tick(false, 20, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 200); });

  tick(false, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0); });
}

BOOST_AUTO_TEST_CASE(test_timer_elasped_time_when_timer_disabled)
{
  init();

  BOOST_REQUIRE_EQUAL(timer->is_enabled(), true);

  tick(false, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0); });

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count); });

  timer->disable();
  BOOST_REQUIRE_EQUAL(timer->is_enabled(), false);

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 99); });

  tick(false, 20, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 99); });

  tick(true, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 99); });
}

BOOST_AUTO_TEST_CASE(test_timer_elasped_idle_time)
{
  init();

  tick(false, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 20 + count); });

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0); });

  tick(false, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count); });

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0); });
}

BOOST_AUTO_TEST_CASE(test_timer_total_overdue_time)
{
  init();

  tick(false, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0); });

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0); });

  tick(true, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count); });

  tick(false, 21, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200); });

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200); });

  tick(true, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200 + count); });
}

BOOST_AUTO_TEST_CASE(test_timer_total_overdue_time_daily_reset_while_active_and_not_overdue)
{
  init();

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0); });

  tick(true, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count); });

  tick(false, 21, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200); });

  tick(true, 50, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200); });

  timer->daily_reset();

  tick(true, 50, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0); });

  tick(true, 50, [=, this](int count) {
    //  the current overdue time is not reset, only the total
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_total_overdue_time_daily_reset_while_active_and_overdue)
{
  init();

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0); });

  tick(true, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count); });

  tick(false, 21, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200); });

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200); });

  tick(true, 50, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200 + count); });

  timer->daily_reset();

  tick(true, 50, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50 + count); });

  tick(true, 50, [=, this](int count) {
    // The current overdue time is not reset, only the total
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 100 + count);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_total_overdue_time_daily_reset_while_idle_and_not_overdue)
{
  init();

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0); });

  tick(true, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count); });

  tick(false, 21, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200); });

  timer->daily_reset();

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0); });

  tick(true, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count); });
}

BOOST_AUTO_TEST_CASE(test_timer_total_overdue_time_daily_reset_while_idle_and_overdue)
{
  init();

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0); });

  tick(true, 200, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count); });

  tick(false, 21, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200); });

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200); });

  tick(true, 70, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 200 + count); });

  tick(false, 10, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 270); });

  timer->daily_reset();

  tick(true, 100, [=, this](int count) { BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 70 + count); });
}

BOOST_AUTO_TEST_CASE(test_timer_stop_timer_while_active_and_not_overdue)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  timer->stop_timer();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 99 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count < 1 ? 0 : count - 1);
    BOOST_REQUIRE_EQUAL(event, (count - 1) % 50 == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_stop_timer_while_active_and_overdue)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(true, 40, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
  });

  timer->stop_timer();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 139 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count < 1 ? 39 : count + 39);
    BOOST_REQUIRE_EQUAL(event, (count - 11) % 50 == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_stop_timer_while_idle_and_overdue)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count % 50 == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });

  tick(false, 20, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 200);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 100);
  });

  timer->stop_timer();

  tick(false, 200, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 20 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 100);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_stop_timer_while_idle_and_not_overdue)
{
  init();

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 20, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->stop_timer();

  tick(false, 200, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 20 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_reset_timer_while_active_and_not_overdue)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->reset_timer();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 99 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 1 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_reset_timer_while_active_and_overdue)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count % 50 == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });

  timer->reset_timer();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 99 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 1 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 99);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_reset_timer_while_idle_and_not_overdue)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->reset_timer();

  tick(false, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 21 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_reset_timer_while_idle__overdue)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count % 50 == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 200);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 100);
  });

  timer->reset_timer();

  tick(false, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 21 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 100);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_freeze_timer_when_active)
{
  init();

  tick(true, 100, [=, this](int count) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 99);
  timer->freeze_timer(true);
  BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 99);

  tick(false, 5, [=, this](int count, TimerEvent event) {
    timer->freeze_timer(true);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 99);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(true, 60, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 99);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 5);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  timer->freeze_timer(false);

  tick(true, 50, [=, this](int count, TimerEvent event) {
    timer->freeze_timer(false);
    // FIXME: why increased to 100?
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_freeze_timer_when_idle_defrost_when_active)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->is_running(), true);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->is_running(), false);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  timer->freeze_timer(true);

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 5 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->is_running(), false);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(true, 60, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->is_running(), true);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
  BOOST_REQUIRE_EQUAL(timer->is_running(), true);
  timer->freeze_timer(false);
  BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 101);
    // FIXME: idle time reset to 0? -> need to process unfreeze in process
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 1);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(true, 40, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 101 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 1 + count);
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_freeze_timer_when_idle_defrost_when_idle)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->is_running(), true);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->is_running(), false);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  timer->freeze_timer(true);

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 5 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->is_running(), false);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(true, 60, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->is_running(), true);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
  BOOST_REQUIRE_EQUAL(timer->is_running(), true);

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
  BOOST_REQUIRE_EQUAL(timer->is_running(), false);

  timer->freeze_timer(false);
  BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 15 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(true, 40, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_lower_limit_while_idle)
{
  init();

  tick(true, 30, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 30);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_limit(40);

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 30);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(true, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 30 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 40 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_lower_limit_while_idle_in_the_past)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_limit(40);

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 60);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 60 + count);
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_lower_limit_while_active)
{
  init();

  tick(true, 30, [=, this](int count) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_limit(40);

  tick(true, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 30 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 40 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_lower_limit_to_in_the_past_while_active)
{
  init();

  tick(true, 100, [=, this](int count) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_limit(60);

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 40 + count);
    BOOST_REQUIRE_EQUAL(event, (count == 0 || count == 50) ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_raise_limit_while_idle)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_limit(160);

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 5 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 60, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 160 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_raise_limit_while_active)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_limit(160);

  tick(true, 60, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 160 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_lower_auto_reset_while_idle)
{
  init();

  tick(true, 100, [=, this](int count) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 10, [=, this](int count) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_auto_reset(15);

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(false, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 15 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_lower_auto_reset_to_in_the_past_while_idle)
{
  init();

  tick(true, 100, [=, this](int count) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 10, [=, this](int count) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_auto_reset(6);

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    // FIXME: why 6?
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 6 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_lower_auto_reset_while_active)
{
  init();

  tick(true, 100, [=, this](int count) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_auto_reset(10);

  tick(true, 40, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
  });

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 140);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 40);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 40);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_raise_auto_reset_while_idle)
{
  init();

  tick(true, 100, [=, this](int count) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 5, [=, this](int count) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_auto_reset(40);

  tick(false, 35, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 5 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 40 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_raise_auto_reset_while_active)
{
  init();

  tick(true, 50, [=, this](int count) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_auto_reset(40);

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(false, 40, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(false, 40, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 40 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_limit_reached_event)
{
  init();

  tick(false, 10, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  tick(true, 100, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  tick(true, 1, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED); });

  tick(true, 10, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });
}

BOOST_AUTO_TEST_CASE(test_timer_limit_reached_snoozed_event)
{
  init();

  tick(false, 10, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  tick(true, 100, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  tick(true, 1, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED); });

  tick(true, 49, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  tick(true, 1, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED); });

  // FIXME: why 49?
  tick(true, 49, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  tick(true, 1, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED); });

  tick(true, 49, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });
}

BOOST_AUTO_TEST_CASE(test_timer_limit_reached_snoozed_event_when_snoozed)
{
  init();

  tick(false, 10, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  tick(true, 100, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  tick(true, 1, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED); });

  tick(true, 40, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  timer->snooze_timer();

  tick(true, 49, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  tick(true, 1, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED); });

  tick(true, 30, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  timer->snooze_timer();

  // FIXME: why 49?
  tick(true, 49, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  tick(true, 1, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED); });

  tick(true, 49, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });
}

BOOST_AUTO_TEST_CASE(test_timer_limit_reached_inhibit_snooze_event)
{
  init();

  tick(false, 10, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  tick(true, 100, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  tick(true, 1, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_LIMIT_REACHED); });

  tick(true, 1, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });

  timer->inhibit_snooze();

  tick(true, 200, [](int count, TimerEvent event) { BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE); });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_when_not_over_limit_and_active_then_enable_and_active)
{
  init();

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->disable();
  timer->enable();
  timer->disable();
  timer->disable();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 49);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(false, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 49);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  timer->enable();
  timer->disable();
  timer->enable();
  timer->enable();

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 49 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_when_not_over_limit_and_active_then_enable_and_idle)
{
  init();

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->disable();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 49);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(false, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 49);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  timer->enable();

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 19 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count < 19 ? 49 : 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 1 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_when_not_over_limit_and_idle_then_enable_and_active)
{
  init();

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->disable();

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0); // FIXME: 5
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  timer->enable();

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_when_not_over_limit_and_idle_then_enable_and_idle)
{
  init();

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->disable();

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0); // FIXME: 5
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->enable();

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 19 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count < 19 ? 50 : 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 1 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_when_over_limit_and_active_then_enable_and_active)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 100 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });

  timer->disable();

  tick(false, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 149);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 49);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 149);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 49);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  timer->enable();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 149 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 49 + count);
    BOOST_REQUIRE_EQUAL(event, (count == 0 || count == 50) ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_when_over_limit_and_active_then_enable_and_idle)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 100 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });

  timer->disable();

  tick(false, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 149);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 49);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 149);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 49);
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
  });

  timer->enable();

  tick(false, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 19 ? TIMER_EVENT_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count < 19 ? 149 : 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 1 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 49);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_when_over_limit_and_idle_then_enable_and_active)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 100 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  timer->disable();

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  tick(false, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  timer->enable();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, (count == 0 || count == 50) ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50 + count);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_when_over_limit_and_idle_then_enable_and_idle)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 100 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  timer->disable();

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0); // FIXME: why reset?
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  tick(false, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  timer->enable();

  tick(false, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 19 ? TIMER_EVENT_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count < 19 ? 150 : 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 1 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_enable_limit_when_active_and_not_overdue)
{
  init();

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 20 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_limit_enabled(false);

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  timer->set_limit_enabled(true);
  BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_enable_limit_when_active_and_overdue)
{
  init();

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 20 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_limit_enabled(false);

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  timer->set_limit_enabled(true);
  BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 49);

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, (count == 0 || count == 50) ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50 + count);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_enable_limit_when_idle)
{
  init();

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_limit_enabled(false);

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 20 ? TIMER_EVENT_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count < 20 ? 50 : 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_limit_enabled(true);

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 50 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_limit_while_active_and_not_overdue)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 100 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 101, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count % 50 == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });

  BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 100);

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 20 ? TIMER_EVENT_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count < 20 ? 201 : 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 101);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 100 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 101);
  });

  timer->set_limit_enabled(false);

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 101);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_limit_when_idle)
{
  init();

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 20 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count < 20 ? 50 : 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_limit_enabled(false);

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 50 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_limit_when_active)
{
  init();

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_limit_enabled(false);

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_limit_while_active_and_overdue)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 100 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 101, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count % 50 == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });

  BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 100);

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 20 ? TIMER_EVENT_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count < 20 ? 201 : 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 101);
  });

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 100 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 101);
  });

  tick(true, 101, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count % 50 == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 101 + count);
  });

  // FIXME: first loop of 201 step incresaes overdue time by 100.
  // FIXME: second loop incresaes overdue time by 101. Why?
  BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 201);

  timer->set_limit_enabled(false);

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 201 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 101);
  });

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 301);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 101);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_auto_reset)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 100 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  timer->set_auto_reset_enabled(false);

  tick(false, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_disable_limit_when_timer_is_disabled)
{
  init();

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 20 ? TIMER_EVENT_NATURAL_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count < 20 ? 50 : 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->disable();

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 20);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  timer->set_limit_enabled(false);

  timer->enable();

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 21 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_toggle_auto_reset__enable_after_reset)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 100 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  timer->set_auto_reset_enabled(false);

  tick(false, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  timer->set_auto_reset_enabled(true);

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    // FIXME: why is this set to 20 instead of 60?
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 20 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_toggle_auto_reset_before_reset)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 100 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  timer->set_auto_reset_enabled(false);

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 10 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  timer->set_auto_reset_enabled(true);

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 15 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 20 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_toggle_auto_reset_when_timer_is_disabled)
{
  init();

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 100 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_LIMIT_REACHED : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), count);
  });

  tick(false, 10, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  timer->disable();

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  timer->set_auto_reset_enabled(false);
  timer->enable();

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 1 + count); // FIXME: should 10+count
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  timer->set_auto_reset_enabled(true);

  tick(false, 14, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 150);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 6 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 20 + count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 50);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_daily_reset_with_predicate)
{
  init();

  TestTimePred *p = new TestTimePred;
  p->set(TimeSource::get_real_time_sec_sync() + 371);
  timer->set_limit(4 * 3600);
  timer->set_daily_reset(p);
  timer->set_auto_reset(0);
  timer->set_auto_reset_enabled(false);

  tick(true, 100, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 200, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 100 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 21, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 300);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 300 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  // reset

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50 + count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, count == 0 ? TIMER_EVENT_RESET : TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });
}

BOOST_AUTO_TEST_CASE(test_timer_serialize)
{
  init();

  tick(true, 50, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), 0);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  tick(false, 5, [=, this](int count, TimerEvent event) {
    BOOST_REQUIRE_EQUAL(event, TIMER_EVENT_NONE);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_time(), 50);
    BOOST_REQUIRE_EQUAL(timer->get_elapsed_idle_time(), count);
    BOOST_REQUIRE_EQUAL(timer->get_total_overdue_time(), 0);
  });

  std::string s1 = timer->serialize_state();
  s1 = s1.substr(s1.find_first_of(" ") + 1);

  Timer::Ptr t = make_timer();

  bool b = t->deserialize_state(s1, 3);
  BOOST_REQUIRE_EQUAL(b, true);

  std::string s2 = t->serialize_state();
  s2 = s2.substr(s2.find_first_of(" ") + 1);
  BOOST_REQUIRE_EQUAL(s1, s2);
}

BOOST_AUTO_TEST_SUITE_END()
