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

#include <boost/test/unit_test.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signals2.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>

#include <iostream>
#include <fstream>
#include <map>

#include "ICore.hh"
#include "IApp.hh"
#include "IBreak.hh"

#include "utils/ITimeSource.hh"
#include "utils/TimeSource.hh"
#include "config/Config.hh"
#include "ICoreTestHooks.hh"

#include "CoreConfig.hh"
#include "SimulatedTime.hh"
#include "ActivityMonitorStub.hh"

using namespace std;
using namespace workrave::utils;
using namespace workrave::config;
using namespace workrave;

template<typename It> struct range {
   It begin_, end_;
   It begin() const { return begin_; }
   It end() const { return end_; }
};
template<typename It> range<It> as_range(const std::pair<It, It> &p) {
   return {p.first, p.second};
}

class Observation
{
public:

  Observation(int64_t time, string event, string params = "") : time(time), event(event), params(params), seen(false)
  {
  }

  bool operator==(const Observation &rhs) const
  {
    return (time == rhs.time
            && event == rhs.event
            && params == rhs.params);
  }

  friend ostream& operator<<(ostream &out, Observation &o);
  
  int64_t time;
  string event;
  string params;
  bool seen;
};

ostream& operator<<(ostream &out, Observation &o)
{
  out << "[time=" << o.time << " event=" << o.event << " arg=" << o.params << "]";
  return out;
}

class Backend : public workrave::IApp
{
public:
  Backend() : user_active(false), active_break(workrave::BREAK_ID_NONE), active_prelude(workrave::BREAK_ID_NONE)
  {
    string display_name = "";

    core = workrave::ICore::create();
    
    ICoreHooks::Ptr hooks = core->get_hooks();
    ICoreTestHooks::Ptr test_hooks = boost::dynamic_pointer_cast<ICoreTestHooks>(hooks);
    
    //test_hooks->hook_is_user_active() = boost::bind(&Backend::on_is_user_active, this, _1);
    test_hooks->hook_create_configurator() = boost::bind(&Backend::on_create_configurator, this);
    test_hooks->hook_create_monitor() = boost::bind(&Backend::on_create_monitor, this);
    test_hooks->hook_load_timer_state() = boost::bind(&Backend::on_load_timer_state, this, _1);
    
    core->init(this, "");

    for (int i = 0; i < workrave::BREAK_ID_SIZEOF; i++)
      {
        workrave::IBreak::Ptr b = core->get_break(workrave::BreakId(i));
        b->signal_break_event().connect(boost::bind(&Backend::on_break_event, this, workrave::BreakId(i), _1));
      }

    core->signal_operation_mode_changed().connect(boost::bind(&Backend::on_operation_mode_changed, this, _1)); 
    core->signal_usage_mode_changed().connect(boost::bind(&Backend::on_usage_mode_changed, this, _1)); 

    sim = SimulatedTime::create();
  }
  
  ~Backend()
  {
    BOOST_TEST_MESSAGE("Verifying");
    out.close();
   }

  void init()
  {
    sim->reset();

    string test_name = boost::unit_test::framework::current_test_case().p_name;

    boost::filesystem::path result_file_name;
    result_file_name /= "results";

    boost::filesystem::create_directory(result_file_name);
      
    result_file_name /= test_name + ".txt";

    out.open(result_file_name.string().c_str());
  }

  void tick()
  {
    tick(user_active, 1, [=](int) {});
  }

  void tick(bool active, int count = 1)
  {
    tick(active, count, [=](int) {});
  }

  void tick(bool active, int seconds, const std::function<void(int)> &check_func)
  {
    for (int i = 0; i < seconds; i++)
      {
        try
          {
            if (user_active != active)
              {
                log(active ? "active" : "idle");
                user_active = active;
              }
            monitor->set_active(user_active);
            did_refresh = false;
            need_refresh = false;
            TimeSource::sync();
            core->heartbeat();
            check_func(i);

            if (active_break != workrave::BREAK_ID_NONE || active_prelude != workrave::BREAK_ID_NONE)
              {
                BOOST_REQUIRE(!need_refresh || did_refresh);
              }
            
            if (active_break != BREAK_ID_NONE)
              {
                check_break_progress();
              }

            if (active_prelude != BREAK_ID_NONE)
              {
                check_prelude_progress();
              }
            
            sim->current_time += 1000000;
            prelude_counter++;
          }
        catch (...)
          {
            BOOST_TEST_MESSAGE(string ("error at:") + boost::lexical_cast<string>(i)); 
            std::cout << "error at : " << (sim->current_time / 1000000) << " " << i << std::endl;
            throw;
          }
      }
  }
  
  void log_actual(const std::string &event, const std::string &param = "")
  {
    int64_t time = sim->get_monotonic_time_usec() / 1000000;
    
    out << time << ",Y,";
    out << event << ",";
    out << param << std::endl;

    actual_results.insert(make_pair(time, Observation(time, event, param)));
  }

  void log(const std::string &event, const std::string &param = "")
  {
    int64_t time = sim->get_monotonic_time_usec() / 1000000;
    
    out << time << ",N,";
    out << event << ",";
    out << param << std::endl;
  }
  
  void expect(int64_t time, const std::string &event, const std::string &param = "")
  {
    time += 1000;
    expected_results.insert(make_pair(time, Observation(time, event, param)));
  }
  
  void verify()
  {
    for (auto &expected : expected_results)
      {
        auto matching_actual_results = actual_results.equal_range(expected.first);
        for (auto &actual : as_range(matching_actual_results))
          {
            if (expected.second == actual.second && !actual.second.seen)
              {
                actual.second.seen = true;
                expected.second.seen = true;
                break;
              }
          }
        BOOST_CHECK_MESSAGE(expected.second.seen, boost::format("Observation %1% missing.") % expected.second);
      }
    
    for (auto &actual : actual_results)
      {
        BOOST_CHECK_MESSAGE(actual.second.seen, boost::format("Observation %1% extra.") % actual.second);
      }
  }

  void check_break_progress()
  {
    IBreak::Ptr b = core->get_break(active_break);
    BOOST_REQUIRE_EQUAL(last_max_value, b->get_auto_reset());
    BOOST_REQUIRE_EQUAL(last_value, b->get_elapsed_idle_time()); // FIXME: handle fake break.
  }

  void check_prelude_progress()
  {
    BOOST_REQUIRE_EQUAL(last_max_value, 29);
    if (prelude_counter == 0)
      {
        // FIXME: this is weird behaviour.
        BOOST_CHECK(last_value == 0 || last_value == 1);
      }
    else if (prelude_counter < 30)
      {
        BOOST_CHECK_EQUAL(last_value, prelude_counter + 1);
      }
    else
      {
        BOOST_CHECK_EQUAL(last_value, 30);
      }
  }
  
  virtual void create_prelude_window(workrave::BreakId break_id)
  {
    IBreak::Ptr b = core->get_break(break_id);

    BOOST_REQUIRE_GE(b->get_elapsed_time(), b->get_limit());
    BOOST_REQUIRE_EQUAL(active_break, workrave::BREAK_ID_NONE);
    BOOST_REQUIRE_EQUAL(active_prelude, workrave::BREAK_ID_NONE);

    active_prelude = break_id;
    prelude_counter = 0;
    prelude_stage_set = false;
    prelude_text_set = false;
    prelude_progress_set = false;
    log_actual("prelude", boost::str(boost::format("break_id=%1%") % CoreConfig::get_break_name(break_id)));
  }
  
  virtual void create_break_window(workrave::BreakId break_id, workrave::BreakHint break_hint)
  {
    IBreak::Ptr b = core->get_break(break_id);

    BOOST_REQUIRE_GE(b->get_elapsed_time(), b->get_limit());
    BOOST_REQUIRE_EQUAL(active_break, workrave::BREAK_ID_NONE);
    BOOST_REQUIRE_EQUAL(active_prelude, workrave::BREAK_ID_NONE);

    active_break = break_id;
    log_actual("break", boost::str(boost::format("break_id=%1% break_hint=%2%") % CoreConfig::get_break_name(break_id) % break_hint));
  }
  
  virtual void hide_break_window()
  {
    if (active_break != workrave::BREAK_ID_NONE || active_prelude  != workrave::BREAK_ID_NONE)
      {
        log_actual("hide");
      }
    else
      {
        log("hide");
      }
    active_break = workrave::BREAK_ID_NONE;
    active_prelude = workrave::BREAK_ID_NONE;
  }
  
  virtual void show_break_window()
  {
    BOOST_REQUIRE(active_break != workrave::BREAK_ID_NONE || active_prelude  != workrave::BREAK_ID_NONE);
    log_actual("show");
  }
  
  virtual void refresh_break_window()
  {
    BOOST_REQUIRE(active_break != workrave::BREAK_ID_NONE || active_prelude != workrave::BREAK_ID_NONE);

    if (active_prelude != workrave::BREAK_ID_NONE)
      {
        BOOST_REQUIRE(prelude_progress_set);
        BOOST_REQUIRE(prelude_stage_set);
        BOOST_REQUIRE(prelude_text_set);
      }

    if (active_break != workrave::BREAK_ID_NONE)
      {
        BOOST_REQUIRE(break_progress_set);
      }
    
    did_refresh = true;
    log("refresh");
  }
  
  virtual void set_break_progress(int value, int max_value)
  {
    BOOST_REQUIRE(active_break != workrave::BREAK_ID_NONE || active_prelude  != workrave::BREAK_ID_NONE);

    last_value = value;
    last_max_value = max_value;
    
    if (active_break != BREAK_ID_NONE)
      {
        check_break_progress();
        break_progress_set = true;
      }

    if (active_prelude != BREAK_ID_NONE)
      {
        check_prelude_progress();
        prelude_progress_set = true;
      }

    need_refresh = true;
    log("progress", boost::str(boost::format("value=%1% max_value=%2%") % value % max_value));
  }
  
  virtual void set_prelude_stage(PreludeStage stage)
  {
    BOOST_REQUIRE(active_break != workrave::BREAK_ID_NONE || active_prelude  != workrave::BREAK_ID_NONE);
    need_refresh = true;
    prelude_stage_set = true;
    log("stage", boost::str(boost::format("stage=%1%") % stage));
  }
  
  virtual void set_prelude_progress_text(PreludeProgressText text)
  {
    BOOST_REQUIRE(active_break != workrave::BREAK_ID_NONE || active_prelude  != workrave::BREAK_ID_NONE);
    need_refresh = true;
    prelude_text_set = true;
    log("text", boost::str(boost::format("text=%1%") % text));
  }

  virtual void on_break_event(workrave::BreakId break_id, workrave::BreakEvent event)
  {
    log_actual("break_event", boost::str(boost::format("break_id=%1% event=%2%") % CoreConfig::get_break_name(break_id) % (int)event));
  }
  
  virtual void on_operation_mode_changed(const workrave::OperationMode m)
  {
    log_actual("operationmode", boost::str(boost::format("mode=%1%") % (int)m));
  }

  virtual void on_usage_mode_changed(const workrave::UsageMode m)
  {
    log_actual("usagemode", boost::str(boost::format("mode=%1%") % (int)m));
  }

  bool on_is_user_active(bool dummy)
  {
    return user_active;
  }

  ActivityMonitor::Ptr on_create_monitor()
  {
    monitor = boost::make_shared<ActivityMonitorStub>();
    return monitor;
  }
  
  IConfigurator::Ptr on_create_configurator()
  {
    config = ConfiguratorFactory::create(ConfiguratorFactory::FormatIni);
    
    config->set_value("timers/micro_pause/limit", 300);
    config->set_value("timers/micro_pause/auto_reset", 20);
    config->set_value("timers/micro_pause/reset_pred", "");
    config->set_value("timers/micro_pause/snooze", 150);
    
    config->set_value("timers/rest_break/limit", 1500);
    config->set_value("timers/rest_break/auto_reset", 300);
    config->set_value("timers/rest_break/reset_pred", "");
    config->set_value("timers/rest_break/snooze", 180);
    
    config->set_value("timers/daily_limit/limit", 14400);
    config->set_value("timers/daily_limit/auto_reset", 0);
    config->set_value("timers/daily_limit/reset_pred", "day/4:00");
    config->set_value("timers/daily_limit/snooze", 1200);
    
    config->set_value("breaks/micro_pause/max_preludes", 3);
    config->set_value("breaks/micro_pause/enabled", true);
    config->set_value("breaks/rest_break/max_preludes", 3);
    config->set_value("breaks/rest_break/enabled", true);
    config->set_value("breaks/daily_limit/max_preludes", 3);
    config->set_value("breaks/daily_limit/enabled", true);
    config->set_value("general/usage-mode", 0);
    config->set_value("general/operation-mode", 0);

    return config;
  }

  bool on_load_timer_state(Timer::Ptr[workrave::BREAK_ID_SIZEOF])
  {
    return true;
  }
  
  ofstream out;
  workrave::ICore::Ptr core;
  IConfigurator::Ptr config;
  SimulatedTime::Ptr sim;
  ActivityMonitorStub::Ptr monitor;
  bool user_active;

  workrave::BreakId active_break;
  workrave::BreakId active_prelude;
  int prelude_counter;

  bool did_refresh;
  bool need_refresh;
  bool prelude_stage_set;
  bool prelude_text_set;
  bool prelude_progress_set;
  bool break_progress_set;
  int last_value;
  int last_max_value;
  
  multimap<int64_t, Observation> expected_results;
  multimap<int64_t, Observation> actual_results;
};

BOOST_FIXTURE_TEST_SUITE(integration, Backend)

BOOST_AUTO_TEST_CASE(test_operation_mode)
{
  init();

  expect(0, "operationmode", "mode=2");
  core->set_operation_mode(workrave::OPERATION_MODE_QUIET);
  core->set_operation_mode(workrave::OPERATION_MODE_QUIET);
  tick(false, 1);

  expect(1, "operationmode", "mode=0");
  core->set_operation_mode(workrave::OPERATION_MODE_NORMAL);
  core->set_operation_mode(workrave::OPERATION_MODE_NORMAL);
  tick(false, 1);

  expect(2, "operationmode", "mode=1");
  core->set_operation_mode(workrave::OPERATION_MODE_SUSPENDED);
  core->set_operation_mode(workrave::OPERATION_MODE_SUSPENDED);
  tick(false, 1);

  expect(3, "operationmode", "mode=0");
  core->set_operation_mode(workrave::OPERATION_MODE_NORMAL);
  core->set_operation_mode(workrave::OPERATION_MODE_NORMAL);

  verify();
}

BOOST_AUTO_TEST_CASE(test_operation_mode_quiet)
{
  init();
  
  expect(0,   "operationmode", "mode=2");
  core->set_operation_mode(workrave::OPERATION_MODE_QUIET);
  tick(true, 300);
  
  expect(300, "operationmode", "mode=0");
  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=0");
  core->set_operation_mode(workrave::OPERATION_MODE_NORMAL);
  tick(true, 1);

  verify();
}

BOOST_AUTO_TEST_CASE(test_operation_mode_quiet_break_snoozed)
{
  init();
  
  expect(0,   "operationmode", "mode=2");
  core->set_operation_mode(workrave::OPERATION_MODE_QUIET);
  tick(true, 302);

  expect(302, "operationmode", "mode=0");
  expect(451, "prelude", "break_id=micro_pause");
  expect(451, "show");
  expect(451, "break_event", "break_id=micro_pause event=0");
  core->set_operation_mode(workrave::OPERATION_MODE_NORMAL);
  tick(true, 150);

  verify();
}

BOOST_AUTO_TEST_CASE(test_operation_mode_suspended)
{
  init();

  expect(0,   "operationmode", "mode=1");
  core->set_operation_mode(workrave::OPERATION_MODE_SUSPENDED);
  tick(true, 300);

  expect(300, "operationmode", "mode=0");
  core->set_operation_mode(workrave::OPERATION_MODE_NORMAL);
  tick(true, 1);

  verify();
}

BOOST_AUTO_TEST_CASE(test_usage_mode)
{
  init();
  
  expect(0, "usagemode", "mode=1");
  core->set_usage_mode(workrave::USAGE_MODE_READING);
  core->set_usage_mode(workrave::USAGE_MODE_READING);

  expect(0, "usagemode", "mode=0");
  core->set_usage_mode(workrave::USAGE_MODE_NORMAL);
  core->set_usage_mode(workrave::USAGE_MODE_NORMAL);

  expect(0, "usagemode", "mode=1");
  core->set_usage_mode(workrave::USAGE_MODE_READING);
  core->set_usage_mode(workrave::USAGE_MODE_READING);

  verify();
}

BOOST_AUTO_TEST_CASE(test_reading_mode)
{
  init();
  
  expect(0, "usagemode", "mode=1");
  core->set_usage_mode(workrave::USAGE_MODE_READING);

  monitor->notify();
  tick(true, 2);
  
  tick(false, 2300);

  int64_t t = 300;
  for (int i = 0; i < 4; i++)
    {
      expect(t,      "prelude", "break_id=micro_pause");
      expect(t,      "show");
      expect(t,      "break_event", "break_id=micro_pause event=0");
      expect(t + 9,  "hide");
      expect(t + 9,  "break" , "break_id=micro_pause break_hint=0");
      expect(t + 9,  "show");
      expect(t + 9,  "break_event", "break_id=micro_pause event=3");
      expect(t + 20, "hide");
      expect(t + 20, "break_event", "break_id=micro_pause event=7");
      expect(t + 20, "break_event", "break_id=micro_pause event=10");

      t += 321;
    }

  t = 1584;
  expect(t,      "prelude", "break_id=rest_break");
  expect(t,      "show");
  expect(t,      "break_event", "break_id=rest_break event=0");
  expect(t + 9,  "hide");
  expect(t + 9,  "break" , "break_id=rest_break break_hint=0");
  expect(t + 9,  "show");
  expect(t + 9,  "break_event", "break_id=rest_break event=3");
  expect(t + 300, "hide");
  expect(t + 300, "break_event", "break_id=rest_break event=7");
  expect(t + 300, "break_event", "break_id=rest_break event=10");
  
  verify();
}

BOOST_AUTO_TEST_CASE(test_reading_mode_active_during_prelude)
{
  init();
  
  expect(0, "usagemode", "mode=1");
  core->set_usage_mode(workrave::USAGE_MODE_READING);

  monitor->notify();

  int64_t t = 300;
  for (int i = 0; i < 4; i++)
    {
      expect(t,      "prelude", "break_id=micro_pause");
      expect(t,      "show");
      expect(t,      "break_event", "break_id=micro_pause event=0");
      expect(t + 15, "hide");
      expect(t + 15, "break" , "break_id=micro_pause break_hint=0");
      expect(t + 15, "show");
      expect(t + 15, "break_event", "break_id=micro_pause event=3");
      expect(t + 35, "hide");
      expect(t + 35, "break_event", "break_id=micro_pause event=7");
      expect(t + 35, "break_event", "break_id=micro_pause event=10");

      tick(false, 300);
      tick(false, 5);
      tick(true, 10);
      tick(false, 21);
      
      t += 336;
    }
  
  verify();
}

BOOST_AUTO_TEST_CASE(test_reading_mode_active_while_no_break_or_prelude_active)
{
  init();
  
  expect(0, "usagemode", "mode=1");
  core->set_usage_mode(workrave::USAGE_MODE_READING);

  monitor->notify();

  int64_t t = 300;
  for (int i = 0; i < 4; i++)
    {
      expect(t,      "prelude", "break_id=micro_pause");
      expect(t,      "show");
      expect(t,      "break_event", "break_id=micro_pause event=0");
      expect(t + 9, "hide");
      expect(t + 9, "break" , "break_id=micro_pause break_hint=0");
      expect(t + 9, "show");
      expect(t + 9, "break_event", "break_id=micro_pause event=3");
      expect(t + 20, "hide");
      expect(t + 20, "break_event", "break_id=micro_pause event=7");
      expect(t + 20, "break_event", "break_id=micro_pause event=10");

      tick(false, 100);
      tick(true, 50);
      tick(false, 50);
      tick(true, 90);
      tick(false, 31);
      
      t += 321;
    }
  
  verify();
}

BOOST_AUTO_TEST_CASE(test_reading_mode_active_during_micro_break)
{
  init();
  
  expect(0, "usagemode", "mode=1");
  core->set_usage_mode(workrave::USAGE_MODE_READING);

  monitor->notify();
  tick(true, 2);
  
  tick(false, 1584);

  int64_t t = 300;
  for (int i = 0; i < 4; i++)
    {
      expect(t,      "prelude", "break_id=micro_pause");
      expect(t,      "show");
      expect(t,      "break_event", "break_id=micro_pause event=0");
      expect(t + 9,  "hide");
      expect(t + 9,  "break" , "break_id=micro_pause break_hint=0");
      expect(t + 9,  "show");
      expect(t + 9,  "break_event", "break_id=micro_pause event=3");
      expect(t + 20, "hide");
      expect(t + 20, "break_event", "break_id=micro_pause event=7");
      expect(t + 20, "break_event", "break_id=micro_pause event=10");

      t += 321;
    }

  tick(false, 20);
  tick(true, 20);
  tick(false, 400); 
 
  t = 1584;
  expect(t,      "prelude", "break_id=rest_break");
  expect(t,      "show");
  expect(t,      "break_event", "break_id=rest_break event=0");
  expect(t + 9,  "hide");
  expect(t + 9,  "break" , "break_id=rest_break break_hint=0");
  expect(t + 9,  "show");
  expect(t + 9,  "break_event", "break_id=rest_break event=3");
  expect(t + 320, "hide");
  expect(t + 320, "break_event", "break_id=rest_break event=7");
  expect(t + 320, "break_event", "break_id=rest_break event=10");
  
  verify();
}

BOOST_AUTO_TEST_CASE(test_user_idle)
{
  init();

  tick(false, 50);

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_active)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=0");
  tick(true, 310);

  expect(310, "hide");
  expect(310, "break", "break_id=micro_pause break_hint=0");
  expect(310, "show");
  expect(310, "break_event", "break_id=micro_pause event=3");
  tick(false, 10);

  expect(330, "hide");
  expect(330, "break_event", "break_id=micro_pause event=7");
  expect(330, "break_event", "break_id=micro_pause event=10");
  tick(false, 30);

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_active_during_prelude)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=0");
  tick(true, 310);
  
  expect(310, "hide");
  expect(310, "break", "break_id=micro_pause break_hint=0");
  expect(310, "show");
  expect(310, "break_event", "break_id=micro_pause event=3");
  tick(false, 10);
  tick(true, 10);
  
  expect(340, "hide");
  expect(340, "break_event", "break_id=micro_pause event=7");
  expect(340, "break_event", "break_id=micro_pause event=10");
  tick(false, 30);

  verify();
}

BOOST_AUTO_TEST_CASE(test_forced_break)
{
  init();

  expect(300,"prelude","break_id=micro_pause");
  expect(300,"show");
  expect(300,"break_event","break_id=micro_pause event=0");
  expect(335,"hide");
  expect(335,"break_event","break_id=micro_pause event=4");
  expect(335,"break_event","break_id=micro_pause event=10");
  expect(452,"prelude","break_id=micro_pause");
  expect(452,"show");
  expect(452,"break_event","break_id=micro_pause event=0");
  expect(481,"hide");
  expect(481,"break","break_id=micro_pause break_hint=0");
  expect(481,"show");
  expect(481,"break_event","break_id=micro_pause event=3");
  tick(true, 760);

  verify();
}

BOOST_AUTO_TEST_SUITE_END()
