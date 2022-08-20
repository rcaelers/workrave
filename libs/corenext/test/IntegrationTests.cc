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

#define BOOST_TEST_MODULE workrave_integration
#include <boost/test/unit_test.hpp>

#include <boost/signals2.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <filesystem>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#if SPDLOG_VERSION >= 10600
#  include <spdlog/pattern_formatter.h>
#endif
#if SPDLOG_VERSION >= 10801
#  include <spdlog/cfg/env.h>
#endif

#include <iostream>
#include <fstream>
#include <map>

#include "core/CoreTypes.hh"
#include "core/CoreConfig.hh"
#include "core/ICore.hh"
#include "core/IApp.hh"
#include "core/IBreak.hh"

#include "config/Config.hh"
#include "config/SettingCache.hh"

#include "utils/ITimeSource.hh"
#include "utils/TimeSource.hh"
#include "debug.hh"

#include "Timer.hh"
#include "ICoreTestHooks.hh"

#include "SimulatedTime.hh"
#include "ActivityMonitorStub.hh"

using namespace std;
using namespace workrave::utils;
using namespace workrave::config;
using namespace workrave;

#if SPDLOG_VERSION >= 10600
class test_time_formatter_flag : public spdlog::custom_flag_formatter
{
public:
  void format(const spdlog::details::log_msg &, const std::tm &, spdlog::memory_buf_t &dest) override
  {
    auto timer_text = std::to_string(timer);
    timer_text = std::string(std::max(0, static_cast<int>(padinfo_.width_ - timer_text.size())), ' ') + timer_text;
    dest.append(timer_text.data(), timer_text.data() + timer_text.size());
  }

  std::unique_ptr<custom_flag_formatter> clone() const override
  {
    return spdlog::details::make_unique<test_time_formatter_flag>();
  }

  static int timer;
};

int test_time_formatter_flag::timer = 0;
#endif

class GlobalFixture
{
public:
  GlobalFixture() = default;
  ~GlobalFixture() = default;

  void setup()
  {
    const auto *log_file = "workrave-core-next-integration-test.log";

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, false);

    auto logger{std::make_shared<spdlog::logger>("workrave", file_sink)};
    spdlog::set_default_logger(logger);

    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%-5l%$] %v");

#if SPDLOG_VERSION >= 10600
    auto formatter = std::make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<test_time_formatter_flag>('*').set_pattern("[%Y-%m-%d %H:%M:%S.%e %4*] [%n] [%^%-5l%$] %v");
    spdlog::set_formatter(std::move(formatter));
#endif
#if SPDLOG_VERSION >= 10801
    spdlog::cfg::load_env_levels();
#endif
  }

  void teardown()
  {
  }
};

template<typename It>
struct range
{
  It begin_, end_;
  It begin() const
  {
    return begin_;
  }
  It end() const
  {
    return end_;
  }
};
template<typename It>
range<It>
as_range(const std::pair<It, It> &p)
{
  return {p.first, p.second};
}

class Observation
{
public:
  Observation(int64_t time, string event, string params = "")
    : time(time)
    , event(event)
    , params(params)
    , seen(false)
  {
  }

  bool operator==(const Observation &rhs) const
  {
    return (time == rhs.time && event == rhs.event && params == rhs.params);
  }

  friend ostream &operator<<(ostream &out, Observation &o);

  int64_t time;
  string event;
  string params;
  bool seen;
};

ostream &
operator<<(ostream &out, const Observation &o)
{
  out << "[time=" << o.time << " event=" << o.event << " arg=" << o.params << "]";
  return out;
}

class Backend : public workrave::IApp
{
public:
  Backend()
    : user_active(false)
    , start_time(0)
    , active_break(BREAK_ID_NONE)
    , active_prelude(BREAK_ID_NONE)
    , timer(0)
    , fake_break(false)
    , fake_break_delta(0)
    , forced_break(false)
    , max_preludes(3)
    , did_refresh(0)
    , need_refresh(0)
    , prelude_stage_set(0)
    , prelude_text_set(0)
    , prelude_progress_set(0)
    , break_progress_set(0)
    , last_value(0)
    , last_max_value(0)
  {
    for (int &i: prelude_count)
      {
        i = 0;
      }
  }

  ~Backend() override
  {
    out.close();
  }

  void init_log_file()
  {
    string test_name = boost::unit_test::framework::current_test_case().p_name;
    std::filesystem::path result_file_name;
    result_file_name /= "results";
    std::filesystem::create_directory(result_file_name);
    result_file_name /= test_name + ".txt";
    out.open(result_file_name.string().c_str());
  }

  void init_core()
  {
    TRACE_ENTRY();
    workrave::config::SettingCache::reset();
    core = workrave::CoreFactory::create();

    ICoreHooks::Ptr hooks = core->get_hooks();
    ICoreTestHooks::Ptr test_hooks = std::dynamic_pointer_cast<ICoreTestHooks>(hooks);

    test_hooks->hook_create_configurator() = std::bind(&Backend::on_create_configurator, this);
    test_hooks->hook_create_monitor() = std::bind(&Backend::on_create_monitor, this);
    test_hooks->hook_load_timer_state() = std::bind(&Backend::on_load_timer_state, this, std::placeholders::_1);

    core->init(this, "");

    for (int i = 0; i < BREAK_ID_SIZEOF; i++)
      {
        auto b = core->get_break(BreakId(i));
        b->signal_break_event().connect(std::bind(&Backend::on_break_event, this, BreakId(i), std::placeholders::_1));
        prelude_count[i] = 0;
      }

    TRACE_MSG("pretest");
    pretest_verify();

    core->set_operation_mode(OperationMode::Normal);
    core->set_usage_mode(UsageMode::Normal);

    core->signal_operation_mode_changed().connect(std::bind(&Backend::on_operation_mode_changed, this, std::placeholders::_1));
    core->signal_usage_mode_changed().connect(std::bind(&Backend::on_usage_mode_changed, this, std::placeholders::_1));
  }

  void init()
  {
    sim = SimulatedTime::create();
    sim->reset();

    TimeSource::sync();
    start_time = sim->get_real_time_usec();
#if SPDLOG_VERSION >= 10600
    test_time_formatter_flag::timer = timer;
#endif
    init_log_file();
    init_core();
  }

  void pretest_verify() const
  {
    for (int i = 0; i < BREAK_ID_SIZEOF; i++)
      {
        auto b = core->get_break(BreakId(i));

        if (i != BREAK_ID_DAILY_LIMIT)
          {
            BOOST_CHECK(b->is_auto_reset_enabled());
          }
        BOOST_CHECK(b->is_limit_enabled());
      }
    BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Quiet);
    BOOST_CHECK_EQUAL(core->get_usage_mode(), UsageMode::Reading);
  }

  void tick()
  {
    tick(user_active, 1, [](int) {});
  }

  void tick(bool active, int count = 1)
  {
    tick(active, count, [](int) {});
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

            for (int j = 0; j < BREAK_ID_SIZEOF; j++)
              {
                auto b = core->get_break(BreakId(j));
                spdlog::debug("{}: elapsed={} idle={}", j, b->get_elapsed_time(), b->get_elapsed_idle_time());
              }

            BOOST_TEST_CONTEXT("Timer")
            {
              BOOST_TEST_INFO_SCOPE("Count:" << i);

              if (active_break != BREAK_ID_NONE || active_prelude != BREAK_ID_NONE)
                {
                  BOOST_CHECK(!need_refresh || did_refresh);
                }

              if (active_break != BREAK_ID_NONE)
                {
                  check_break_progress();
                }

              for (int j = 0; j < BREAK_ID_SIZEOF; j++)
                {
                  auto b = core->get_break(BreakId(j));
                  BOOST_CHECK(j == active_break ? b->is_taking() : !b->is_taking());
                }

              if (active_prelude != BREAK_ID_NONE)
                {
                  check_prelude_progress();
                }

              check_func(i);
            }
            sim->current_time += 1000000;
            timer++;
#if SPDLOG_VERSION >= 10600
            test_time_formatter_flag::timer = (sim->current_time - start_time) / 1000000;
#endif
          }
        catch (std::exception &e)
          {
            BOOST_TEST_MESSAGE(string("error at:") + boost::lexical_cast<string>(i));
            std::cout << "error at : " << ((sim->current_time - start_time) / 1000000) << " " << i << "\n";
            std::cout << e.what() << "\n";
            throw;
          }
        catch (...)
          {
            BOOST_TEST_MESSAGE(string("error at:") + boost::lexical_cast<string>(i));
            std::cout << "error at : " << ((sim->current_time - start_time) / 1000000) << " " << i << "\n";
            throw;
          }
      }
  }

  void log_actual(const std::string &event, const std::string &param = "")
  {
    int64_t time = (sim->get_monotonic_time_usec() - start_time) / 1000000;

    out << time << ",Y,";
    out << event << ",";
    out << param << std::endl;

    auto observation = Observation(time, event, param);
    actual_results.insert(make_pair(time, observation));
    spdlog::debug("Observation: time={} event={} arg={}", observation.time, observation.event, observation.params);
  }

  void log(const std::string &event, const std::string &param = "")
  {
    int64_t time = (sim->get_monotonic_time_usec() - start_time) / 1000000;

    out << time << ",N,";
    out << event << ",";
    out << param << std::endl;
  }

  void expect(int64_t time, const std::string &event, const std::string &param = "")
  {
    expected_results.insert(make_pair(time, Observation(time, event, param)));
  }

  void verify()
  {
    for (auto &expected: expected_results)
      {
        auto matching_actual_results = actual_results.equal_range(expected.first);
        for (auto &actual: as_range(matching_actual_results))
          {
            if (expected.second == actual.second && !actual.second.seen)
              {
                actual.second.seen = true;
                expected.second.seen = true;
                break;
              }
          }
      }

    std::list<std::pair<int, Observation>> report;
    std::transform(expected_results.begin(), expected_results.end(), std::back_inserter(report), [](const auto &o) {
      return std::make_pair(0, o.second);
    });
    std::transform(actual_results.begin(), actual_results.end(), std::back_inserter(report), [](const auto &o) {
      return std::make_pair(1, o.second);
    });
    report.sort([](auto &a, auto &b) { return a.second.time < b.second.time; });

    for (auto [what, observation]: report)
      {
        if (what == 0)
          {
            BOOST_CHECK_MESSAGE(observation.seen, boost::format("Observation %1% missing.") % observation);
          }
        if (what == 1)
          {
            BOOST_CHECK_MESSAGE(observation.seen, boost::format("Observation %1% extra.") % observation);
          }
      }
  }

  void check_break_progress() const
  {
    auto b = core->get_break(active_break);
    if (fake_break)
      {
        BOOST_CHECK_EQUAL(last_max_value, b->get_auto_reset());
        BOOST_CHECK_EQUAL(last_value, timer + fake_break_delta);
      }
    else
      {
        BOOST_CHECK_EQUAL(last_max_value, b->get_auto_reset());

        if (active_break != BREAK_ID_DAILY_LIMIT)
          {
            // FIXME: check why this fails for daly limit.
            BOOST_CHECK_EQUAL(last_value, b->get_elapsed_idle_time());
          }
      }
  }

  void check_prelude_progress()
  {
    BOOST_CHECK_EQUAL(last_max_value, 29);
    if (timer == 0)
      {
        // FIXME: this is weird behaviour.
        BOOST_CHECK(last_value == 0 || last_value == 1);
      }
    else if (timer < 30)
      {
        BOOST_CHECK_EQUAL(last_value, timer + 1);
      }
    else
      {
        BOOST_CHECK_EQUAL(last_value, 30);
      }
  }

  void create_prelude_window(BreakId break_id) override
  {
    log_actual("prelude", boost::str(boost::format("break_id=%1%") % CoreConfig::get_break_name(break_id)));

    auto b = core->get_break(break_id);
    BOOST_CHECK_EQUAL(b->get_name(), CoreConfig::get_break_name(break_id));

    bool rest_break_advanced = false;
    if (break_id == BREAK_ID_REST_BREAK)
      {
        auto mb = core->get_break(BREAK_ID_MICRO_BREAK);

        if (mb->get_elapsed_time() >= mb->get_limit() && b->get_elapsed_time() + 30 >= b->get_limit())
          {
            rest_break_advanced = true;
          }
      }

    BOOST_CHECK(rest_break_advanced || b->get_elapsed_time() >= b->get_limit());
    BOOST_CHECK_EQUAL(active_break, BREAK_ID_NONE);
    BOOST_CHECK_EQUAL(active_prelude, BREAK_ID_NONE);

    active_prelude = break_id;
    prelude_count[break_id]++;
    timer = 0;
    prelude_stage_set = false;
    prelude_text_set = false;
    prelude_progress_set = false;
    last_value = -1;
    last_max_value = 0;
  }

  void create_break_window(BreakId break_id, workrave::utils::Flags<BreakHint> break_hint) override
  {
    log_actual("break",
               boost::str(boost::format("break_id=%1% break_hint=%2%") % CoreConfig::get_break_name(break_id) % break_hint));

    auto b = core->get_break(break_id);

    bool rest_break_advanced = false;
    if (break_id == BREAK_ID_REST_BREAK)
      {
        auto mb = core->get_break(BREAK_ID_MICRO_BREAK);

        if (mb->get_elapsed_time() >= mb->get_limit() && b->get_elapsed_time() + 30 >= b->get_limit())
          {
            rest_break_advanced = true;
          }
      }

    if (!fake_break && !forced_break)
      {
        BOOST_CHECK(rest_break_advanced || b->get_elapsed_time() >= b->get_limit());
      }
    BOOST_CHECK_EQUAL(active_break, BREAK_ID_NONE);
    BOOST_CHECK_EQUAL(active_prelude, BREAK_ID_NONE);

    active_break = break_id;
    timer = 0;
    last_value = -1;
    last_max_value = 0;
  }

  void hide_break_window() override
  {
    if (active_break != BREAK_ID_NONE || active_prelude != BREAK_ID_NONE)
      {
        log_actual("hide");
      }
    else
      {
        log("hide");
      }
    active_break = BREAK_ID_NONE;
    active_prelude = BREAK_ID_NONE;
  }

  void show_break_window() override
  {
    log_actual("show");
    BOOST_CHECK(active_break != BREAK_ID_NONE || active_prelude != BREAK_ID_NONE);
  }

  void refresh_break_window() override
  {
    log("refresh");

    // TODO: remove forced_break from check after fixing code.
    BOOST_CHECK(forced_break || active_break != BREAK_ID_NONE || active_prelude != BREAK_ID_NONE);

    if (active_prelude != BREAK_ID_NONE)
      {
        BOOST_CHECK(prelude_progress_set);
        BOOST_CHECK(prelude_stage_set);
        BOOST_CHECK(prelude_text_set);
      }

    if (active_break != BREAK_ID_NONE)
      {
        BOOST_CHECK(break_progress_set);
      }

    did_refresh = true;
  }

  void set_break_progress(int value, int max_value) override
  {
    log("progress", boost::str(boost::format("value=%1% max_value=%2%") % value % max_value));

    // TODO: remove forced_break from check after fixing code.
    BOOST_CHECK(forced_break || active_break != BREAK_ID_NONE || active_prelude != BREAK_ID_NONE);

    last_value = value;
    last_max_value = max_value;

    if (active_break != BREAK_ID_NONE)
      {
        break_progress_set = true;
      }

    if (active_prelude != BREAK_ID_NONE)
      {
        prelude_progress_set = true;
      }

    need_refresh = true;
  }

  void set_prelude_stage(PreludeStage stage) override
  {
    log("stage", boost::str(boost::format("stage=%1%") % stage));

    BOOST_CHECK(active_break != BREAK_ID_NONE || active_prelude != BREAK_ID_NONE);

    need_refresh = true;
    prelude_stage_set = true;
  }

  void set_prelude_progress_text(PreludeProgressText text) override
  {
    log("text", boost::str(boost::format("text=%1%") % text));

    BOOST_CHECK(active_break != BREAK_ID_NONE || active_prelude != BREAK_ID_NONE);

    if (prelude_count[active_prelude] < max_preludes)
      {
        BOOST_CHECK_EQUAL(text, IApp::PROGRESS_TEXT_DISAPPEARS_IN);
      }
    else
      {
        BOOST_CHECK_EQUAL(text, IApp::PROGRESS_TEXT_BREAK_IN);
      }

    need_refresh = true;
    prelude_text_set = true;
  }

  virtual void on_break_event(BreakId break_id, BreakEvent event)
  {
    log_actual("break_event", boost::str(boost::format("break_id=%1% event=%2%") % CoreConfig::get_break_name(break_id) % event));

    if (event == BreakEvent::BreakStop)
      {
        prelude_count[break_id] = 0;
      }
  }

  virtual void on_operation_mode_changed(const OperationMode m)
  {
    log_actual("operationmode", boost::str(boost::format("mode=%1%") % static_cast<int>(m)));
  }

  virtual void on_usage_mode_changed(const UsageMode m)
  {
    log_actual("usagemode", boost::str(boost::format("mode=%1%") % static_cast<int>(m)));
  }

  bool on_is_user_active(bool dummy) const
  {
    return user_active;
  }

  IActivityMonitor::Ptr on_create_monitor()
  {
    monitor = std::make_shared<ActivityMonitorStub>();
    return monitor;
  }

  IConfigurator::Ptr on_create_configurator()
  {
    config = ConfiguratorFactory::create(ConfigFileFormat::Ini);

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
    config->set_value("breaks/rest_break/max_preludes", 6);
    config->set_value("breaks/rest_break/enabled", true);
    config->set_value("breaks/daily_limit/max_preludes", 3);
    config->set_value("breaks/daily_limit/enabled", true);

    config->set_value("timers/daily_limit/use_microbreak_activity", false);
    config->set_value("general/usage-mode", 1);
    config->set_value("general/operation-mode", 2);

    return config;
  }

  bool on_load_timer_state(Timer::Ptr breaks[BREAK_ID_SIZEOF])
  {
    return true;
  }

  ofstream out;
  ICore::Ptr core;
  IConfigurator::Ptr config;
  SimulatedTime::Ptr sim;
  ActivityMonitorStub::Ptr monitor;
  bool user_active;
  uint64_t start_time;

  BreakId active_break;
  BreakId active_prelude;
  int timer;
  int prelude_count[BREAK_ID_SIZEOF];
  bool fake_break;
  int fake_break_delta;
  bool forced_break;
  int max_preludes;

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

BOOST_TEST_GLOBAL_FIXTURE(GlobalFixture);

BOOST_FIXTURE_TEST_SUITE(integration, Backend)

BOOST_AUTO_TEST_CASE(test_operation_mode)
{
  init();

  expect(0, "operationmode", "mode=2");
  core->set_operation_mode(OperationMode::Quiet);
  core->set_operation_mode(OperationMode::Quiet);
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Quiet);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Quiet);
  BOOST_CHECK(!core->is_operation_mode_an_override());

  expect(1, "operationmode", "mode=0");
  core->set_operation_mode(OperationMode::Normal);
  core->set_operation_mode(OperationMode::Normal);
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Normal);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(!core->is_operation_mode_an_override());

  expect(2, "operationmode", "mode=1");
  core->set_operation_mode(OperationMode::Suspended);
  core->set_operation_mode(OperationMode::Suspended);
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK(!core->is_operation_mode_an_override());

  expect(3, "operationmode", "mode=0");
  core->set_operation_mode(OperationMode::Normal);
  core->set_operation_mode(OperationMode::Normal);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Normal);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(!core->is_operation_mode_an_override());

  verify();
}

BOOST_AUTO_TEST_CASE(test_operation_mode_via_settings)
{
  init();

  expect(0, "operationmode", "mode=2");
  config->set_value("general/operation-mode", 2);
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Quiet);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Quiet);
  BOOST_CHECK(!core->is_operation_mode_an_override());

  expect(1, "operationmode", "mode=0");
  config->set_value("general/operation-mode", 0);
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Normal);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(!core->is_operation_mode_an_override());

  expect(2, "operationmode", "mode=1");
  config->set_value("general/operation-mode", 1);
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK(!core->is_operation_mode_an_override());

  expect(3, "operationmode", "mode=0");
  config->set_value("general/operation-mode", 0);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Normal);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(!core->is_operation_mode_an_override());

  verify();
}

BOOST_AUTO_TEST_CASE(test_operation_mode_quiet)
{
  init();

  expect(0, "operationmode", "mode=2");
  core->set_operation_mode(OperationMode::Quiet);
  tick(true, 300);

  expect(300, "operationmode", "mode=0");
  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  core->set_operation_mode(OperationMode::Normal);
  tick(true, 1);

  verify();
}

BOOST_AUTO_TEST_CASE(test_operation_mode_quiet_break_snoozed)
{
  init();

  expect(0, "operationmode", "mode=2");
  core->set_operation_mode(OperationMode::Quiet);
  tick(true, 302);

  expect(302, "operationmode", "mode=0");
  expect(450, "prelude", "break_id=micro_pause");
  expect(450, "show");
  expect(450, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(450, "break_event", "break_id=micro_pause event=BreakStart");
  core->set_operation_mode(OperationMode::Normal);
  tick(true, 150);

  verify();
}

BOOST_AUTO_TEST_CASE(test_operation_mode_suspended)
{
  init();

  expect(0, "operationmode", "mode=1");
  core->set_operation_mode(OperationMode::Suspended);
  tick(true, 300);

  expect(300, "operationmode", "mode=0");
  core->set_operation_mode(OperationMode::Normal);
  tick(true, 1);

  verify();
}

BOOST_AUTO_TEST_CASE(test_operation_mode_override)
{
  init();

  core->set_operation_mode_override(OperationMode::Suspended, "ov1");
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(core->is_operation_mode_an_override());

  core->set_operation_mode_override(OperationMode::Quiet, "ov2");
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(core->is_operation_mode_an_override());

  core->remove_operation_mode_override("ov2");
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(core->is_operation_mode_an_override());

  core->set_operation_mode_override(OperationMode::Quiet, "ov2");
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(core->is_operation_mode_an_override());

  core->remove_operation_mode_override("ov1");
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Quiet);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(core->is_operation_mode_an_override());

  core->remove_operation_mode_override("ov2");
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Normal);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(!core->is_operation_mode_an_override());

  core->set_operation_mode_override(OperationMode::Normal, "ov3");
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Normal);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(core->is_operation_mode_an_override());

  verify();
}

BOOST_AUTO_TEST_CASE(test_operation_mode_override_change_to_normal_while_overridden)
{
  init();

  expect(0, "operationmode", "mode=2");
  core->set_operation_mode(OperationMode::Quiet);
  BOOST_CHECK_EQUAL(CoreConfig::operation_mode()(), OperationMode::Quiet);
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Quiet);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Quiet);
  BOOST_CHECK(!core->is_operation_mode_an_override());
  BOOST_CHECK_EQUAL(CoreConfig::operation_mode()(), OperationMode::Quiet);

  core->set_operation_mode_override(OperationMode::Suspended, "ov1");
  BOOST_CHECK_EQUAL(CoreConfig::operation_mode()(), OperationMode::Quiet);
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Quiet);
  BOOST_CHECK(core->is_operation_mode_an_override());

  expect(2, "operationmode", "mode=0");
  core->set_operation_mode(OperationMode::Normal);
  BOOST_CHECK_EQUAL(CoreConfig::operation_mode()(), OperationMode::Normal);
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(core->is_operation_mode_an_override());

  core->remove_operation_mode_override("ov1");
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Normal);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(!core->is_operation_mode_an_override());

  core->remove_operation_mode_override("ov2");
  tick(false, 1);

  verify();
}

BOOST_AUTO_TEST_CASE(test_operation_mode_override_revert_while_overridden)
{
  init();

  expect(0, "operationmode", "mode=2");
  core->set_operation_mode(OperationMode::Quiet);
  BOOST_CHECK_EQUAL(CoreConfig::operation_mode()(), OperationMode::Quiet);
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Quiet);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Quiet);
  BOOST_CHECK(!core->is_operation_mode_an_override());

  core->set_operation_mode_override(OperationMode::Suspended, "ov1");
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Quiet);
  BOOST_CHECK(core->is_operation_mode_an_override());

  expect(2, "operationmode", "mode=1");
  core->set_operation_mode(OperationMode::Suspended);
  BOOST_CHECK_EQUAL(CoreConfig::operation_mode()(), OperationMode::Suspended);
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK(core->is_operation_mode_an_override());

  core->remove_operation_mode_override("ov1");
  tick(false, 1);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK_EQUAL(core->get_regular_operation_mode(), OperationMode::Suspended);
  BOOST_CHECK(!core->is_operation_mode_an_override());

  core->remove_operation_mode_override("ov2");
  tick(false, 1);

  verify();
}

BOOST_AUTO_TEST_CASE(test_operation_mode_autoreset)
{
  init();

  expect(0, "operationmode", "mode=1");
  core->set_operation_mode(OperationMode::Suspended);
  tick(false, 10);
  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Suspended);

  // Revert to normal after 2 minutes
  expect(10, "operationmode", "mode=2");
  core->set_operation_mode_for(OperationMode::Quiet, std::chrono::minutes(2));
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 2min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == workrave::utils::TimeSource::get_real_time() + 2min);

  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Quiet);
  expect(130, "operationmode", "mode=0");
  tick(false, 190);
  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 0min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == std::chrono::system_clock::time_point{});

  // Option is not persistent
  expect(200, "operationmode", "mode=1");
  core->set_operation_mode(OperationMode::Suspended);
  tick(false, 200);

  // Change timed operation mode
  expect(400, "operationmode", "mode=2");
  expect(420, "operationmode", "mode=1");
  expect(540, "operationmode", "mode=0");
  core->set_operation_mode_for(OperationMode::Quiet, std::chrono::minutes(2));
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 2min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == workrave::utils::TimeSource::get_real_time() + 2min);
  tick(false, 20);
  core->set_operation_mode_for(OperationMode::Suspended, std::chrono::minutes(2));
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 2min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == workrave::utils::TimeSource::get_real_time() + 2min);
  tick(false, 180);
  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 0min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == std::chrono::system_clock::time_point{});

  // Reduce time
  expect(600, "operationmode", "mode=2");
  expect(680, "operationmode", "mode=0");
  core->set_operation_mode_for(OperationMode::Quiet, std::chrono::minutes(2));
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 2min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == workrave::utils::TimeSource::get_real_time() + 2min);
  tick(false, 20);
  core->set_operation_mode_for(OperationMode::Quiet, std::chrono::minutes(1));
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 1min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == workrave::utils::TimeSource::get_real_time() + 1min);
  tick(false, 180);
  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Normal);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 0min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == std::chrono::system_clock::time_point{});

  // Turn off
  expect(800, "operationmode", "mode=2");
  core->set_operation_mode_for(OperationMode::Quiet, std::chrono::minutes(2));
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 2min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == workrave::utils::TimeSource::get_real_time() + 2min);
  tick(false, 20);
  core->set_operation_mode(OperationMode::Quiet);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 0min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == std::chrono::system_clock::time_point{});
  tick(false, 170);
  expect(990, "operationmode", "mode=0");
  core->set_operation_mode(OperationMode::Normal);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 0min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == std::chrono::system_clock::time_point{});
  tick(false, 10);

  // Change to non-timed
  expect(1000, "operationmode", "mode=2");
  expect(1020, "operationmode", "mode=1");
  core->set_operation_mode_for(OperationMode::Quiet, std::chrono::minutes(2));
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 2min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == workrave::utils::TimeSource::get_real_time() + 2min);
  tick(false, 20);
  core->set_operation_mode(OperationMode::Suspended);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 0min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == std::chrono::system_clock::time_point{});
  tick(false, 180);

  // Change to non-timed
  expect(1200, "operationmode", "mode=2");
  expect(1220, "operationmode", "mode=0");
  core->set_operation_mode_for(OperationMode::Quiet, std::chrono::minutes(2));
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 2min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == workrave::utils::TimeSource::get_real_time() + 2min);
  tick(false, 20);
  core->set_operation_mode(OperationMode::Normal);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 0min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == std::chrono::system_clock::time_point{});
  tick(false, 180);

  verify();
}

BOOST_AUTO_TEST_CASE(test_operation_mode_autoreset_daily_reset)
{
  init();

  auto b = core->get_break(BREAK_ID_DAILY_LIMIT);

  config->set_value("breaks/micro_pause/enabled", false);
  config->set_value("breaks/rest_break/enabled", false);

  config->set_value("timers/daily_limit/reset_pred", "day/23:00");
  config->set_value("timers/daily_limit/limit", 7200);
  config->set_value("timers/daily_limit/snooze", 600);

  expect(0, "operationmode", "mode=1");
  core->set_operation_mode(OperationMode::Suspended);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 0min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == std::chrono::system_clock::time_point{});
  tick(true, 10);
  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Suspended);

  expect(10, "operationmode", "mode=2");
  core->set_operation_mode_for(OperationMode::Quiet, std::chrono::minutes(-1));
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == -1min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == std::chrono::system_clock::time_point{});
  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Quiet);
  expect(3600, "operationmode", "mode=0");

  tick(true, 90);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == -1min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == std::chrono::system_clock::time_point{});

  BOOST_CHECK_EQUAL(b->get_elapsed_time(), 89);

  tick(false, 3500);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == -1min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == std::chrono::system_clock::time_point{});

  BOOST_CHECK_EQUAL(b->get_elapsed_time(), 90);
  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Quiet);

  tick(false, 1);

  BOOST_CHECK_EQUAL(b->get_elapsed_time(), 0);
  BOOST_CHECK_EQUAL(core->get_active_operation_mode(), OperationMode::Normal);

  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_duration()() == 0min);
  BOOST_CHECK(CoreConfig::operation_mode_auto_reset_time()() == std::chrono::system_clock::time_point{});

  verify();

  // Revert to normal after 2 minutes
}

BOOST_AUTO_TEST_CASE(test_usage_mode)
{
  init();

  expect(0, "usagemode", "mode=1");
  core->set_usage_mode(UsageMode::Reading);
  core->set_usage_mode(UsageMode::Reading);

  BOOST_CHECK_EQUAL(core->get_usage_mode(), UsageMode::Reading);

  expect(0, "usagemode", "mode=0");
  core->set_usage_mode(UsageMode::Normal);
  core->set_usage_mode(UsageMode::Normal);

  BOOST_CHECK_EQUAL(core->get_usage_mode(), UsageMode::Normal);

  expect(0, "usagemode", "mode=1");
  core->set_usage_mode(UsageMode::Reading);
  core->set_usage_mode(UsageMode::Reading);

  BOOST_CHECK_EQUAL(core->get_usage_mode(), UsageMode::Reading);

  verify();
}

BOOST_AUTO_TEST_CASE(test_usage_mode_via_settings)
{
  init();

  expect(0, "usagemode", "mode=1");
  config->set_value("general/usage-mode", 1);

  BOOST_CHECK_EQUAL(core->get_usage_mode(), UsageMode::Reading);

  expect(0, "usagemode", "mode=0");
  config->set_value("general/usage-mode", 0);

  BOOST_CHECK_EQUAL(core->get_usage_mode(), UsageMode::Normal);

  expect(0, "usagemode", "mode=1");
  config->set_value("general/usage-mode", 1);

  BOOST_CHECK_EQUAL(core->get_usage_mode(), UsageMode::Reading);

  verify();
}

BOOST_AUTO_TEST_CASE(test_reading_mode)
{
  init();

  expect(0, "usagemode", "mode=1");
  core->set_usage_mode(UsageMode::Reading);

  monitor->notify();
  tick(true, 2);

  tick(false, 2300);

  int64_t t = 300;
  for (int i = 0; i < 4; i++)
    {
      expect(t, "prelude", "break_id=micro_pause");
      expect(t, "show");
      expect(t, "break_event", "break_id=micro_pause event=ShowPrelude");
      expect(t, "break_event", "break_id=micro_pause event=BreakStart");
      expect(t + 9, "hide");
      expect(t + 9, "break", "break_id=micro_pause break_hint=normal");
      expect(t + 9, "show");
      expect(t + 9, "break_event", "break_id=micro_pause event=ShowBreak");
      expect(t + 20, "hide");
      expect(t + 20, "break_event", "break_id=micro_pause event=BreakTaken");
      expect(t + 20, "break_event", "break_id=micro_pause event=BreakIdle");
      expect(t + 20, "break_event", "break_id=micro_pause event=BreakStop");

      t += 321;
    }

  t = 1584;
  expect(t, "prelude", "break_id=rest_break");
  expect(t, "show");
  expect(t, "break_event", "break_id=rest_break event=ShowPrelude");
  expect(t, "break_event", "break_id=rest_break event=BreakStart");
  expect(t + 9, "hide");
  expect(t + 9, "break", "break_id=rest_break break_hint=normal");
  expect(t + 9, "show");
  expect(t + 9, "break_event", "break_id=rest_break event=ShowBreak");
  expect(t + 300, "hide");
  expect(t + 300, "break_event", "break_id=rest_break event=BreakTaken");
  expect(t + 300, "break_event", "break_id=rest_break event=BreakIdle");
  expect(t + 300, "break_event", "break_id=rest_break event=BreakStop");

  verify();
}

BOOST_AUTO_TEST_CASE(test_reading_mode_active_during_prelude)
{
  init();

  expect(0, "usagemode", "mode=1");
  core->set_usage_mode(UsageMode::Reading);

  monitor->notify();

  int64_t t = 300;
  for (int i = 0; i < 4; i++)
    {
      expect(t, "prelude", "break_id=micro_pause");
      expect(t, "show");
      expect(t, "break_event", "break_id=micro_pause event=ShowPrelude");
      expect(t, "break_event", "break_id=micro_pause event=BreakStart");
      expect(t + 15, "hide");
      expect(t + 15, "break", "break_id=micro_pause break_hint=normal");
      expect(t + 15, "show");
      expect(t + 15, "break_event", "break_id=micro_pause event=ShowBreak");
      expect(t + 35, "hide");
      expect(t + 35, "break_event", "break_id=micro_pause event=BreakTaken");
      expect(t + 35, "break_event", "break_id=micro_pause event=BreakIdle");
      expect(t + 35, "break_event", "break_id=micro_pause event=BreakStop");

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
  core->set_usage_mode(UsageMode::Reading);

  monitor->notify();

  int64_t t = 300;
  for (int i = 0; i < 4; i++)
    {
      expect(t, "prelude", "break_id=micro_pause");
      expect(t, "show");
      expect(t, "break_event", "break_id=micro_pause event=ShowPrelude");
      expect(t, "break_event", "break_id=micro_pause event=BreakStart");
      expect(t + 9, "hide");
      expect(t + 9, "break", "break_id=micro_pause break_hint=normal");
      expect(t + 9, "show");
      expect(t + 9, "break_event", "break_id=micro_pause event=ShowBreak");
      expect(t + 20, "hide");
      expect(t + 20, "break_event", "break_id=micro_pause event=BreakTaken");
      expect(t + 20, "break_event", "break_id=micro_pause event=BreakIdle");
      expect(t + 20, "break_event", "break_id=micro_pause event=BreakStop");

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
  core->set_usage_mode(UsageMode::Reading);

  monitor->notify();
  tick(true, 2);

  tick(false, 1584);

  int64_t t = 300;
  for (int i = 0; i < 4; i++)
    {
      expect(t, "prelude", "break_id=micro_pause");
      expect(t, "show");
      expect(t, "break_event", "break_id=micro_pause event=ShowPrelude");
      expect(t, "break_event", "break_id=micro_pause event=BreakStart");
      expect(t + 9, "hide");
      expect(t + 9, "break", "break_id=micro_pause break_hint=normal");
      expect(t + 9, "show");
      expect(t + 9, "break_event", "break_id=micro_pause event=ShowBreak");
      expect(t + 20, "hide");
      expect(t + 20, "break_event", "break_id=micro_pause event=BreakTaken");
      expect(t + 20, "break_event", "break_id=micro_pause event=BreakIdle");
      expect(t + 20, "break_event", "break_id=micro_pause event=BreakStop");

      t += 321;
    }

  tick(false, 20);
  tick(true, 20);
  tick(false, 400);

  t = 1584;
  expect(t, "prelude", "break_id=rest_break");
  expect(t, "show");
  expect(t, "break_event", "break_id=rest_break event=ShowPrelude");
  expect(t, "break_event", "break_id=rest_break event=BreakStart");
  expect(t + 9, "hide");
  expect(t + 9, "break", "break_id=rest_break break_hint=normal");
  expect(t + 9, "show");
  expect(t + 9, "break_event", "break_id=rest_break event=ShowBreak");
  expect(t + 320, "hide");
  expect(t + 320, "break_event", "break_id=rest_break event=BreakTaken");
  expect(t + 320, "break_event", "break_id=rest_break event=BreakIdle");
  expect(t + 320, "break_event", "break_id=rest_break event=BreakStop");

  verify();
}

BOOST_AUTO_TEST_CASE(test_reading_mode_suspend)
{
  init();

  expect(0, "usagemode", "mode=1");
  core->set_usage_mode(UsageMode::Reading);

  monitor->notify();
  tick(true, 2);

  tick(false, 1580);

  int64_t t = 300;
  for (int i = 0; i < 4; i++)
    {
      expect(t, "prelude", "break_id=micro_pause");
      expect(t, "show");
      expect(t, "break_event", "break_id=micro_pause event=ShowPrelude");
      expect(t, "break_event", "break_id=micro_pause event=BreakStart");
      expect(t + 9, "hide");
      expect(t + 9, "break", "break_id=micro_pause break_hint=normal");
      expect(t + 9, "show");
      expect(t + 9, "break_event", "break_id=micro_pause event=ShowBreak");
      expect(t + 20, "hide");
      expect(t + 20, "break_event", "break_id=micro_pause event=BreakTaken");
      expect(t + 20, "break_event", "break_id=micro_pause event=BreakIdle");
      expect(t + 20, "break_event", "break_id=micro_pause event=BreakStop");

      t += 321;
    }

  expect(1582, "operationmode", "mode=1");
  core->set_operation_mode(OperationMode::Suspended);
  tick(true, 100);
  expect(1682, "operationmode", "mode=0");
  core->set_operation_mode(OperationMode::Normal);
  tick(false, 400);

  t = 1684;
  expect(t, "prelude", "break_id=rest_break");
  expect(t, "show");
  expect(t, "break_event", "break_id=rest_break event=ShowPrelude");
  expect(t, "break_event", "break_id=rest_break event=BreakStart");
  expect(t + 9, "hide");
  expect(t + 9, "break", "break_id=rest_break break_hint=normal");
  expect(t + 9, "show");
  expect(t + 9, "break_event", "break_id=rest_break event=ShowBreak");
  expect(t + 300, "hide");
  expect(t + 300, "break_event", "break_id=rest_break event=BreakTaken");
  expect(t + 300, "break_event", "break_id=rest_break event=BreakIdle");
  expect(t + 300, "break_event", "break_id=rest_break event=BreakStop");

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_idle)
{
  init();

  tick(false, 50, [=, this](int) {
    for (int i = 0; i < BREAK_ID_SIZEOF; i++)
      {
        auto b = core->get_break(BreakId(i));
        BOOST_CHECK(!b->is_running());
      }
  });

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_idle_just_before_first_prelude)
{
  init();

  tick(true, 299);

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_active)
{
  init();

  tick(true, 50, [=, this](int) {
    for (int i = 0; i < BREAK_ID_SIZEOF; i++)
      {
        auto b = core->get_break(BreakId(i));
        BOOST_CHECK(b->is_running());
      }
  });

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_ignores_first_prelude)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  expect(335, "break_event", "break_id=micro_pause event=BreakIgnored");
  expect(335, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(335, "hide");
  tick(true, 336);

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_takes_break_immediately_after_start_of_first_prelude)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  tick(true, 301);

  expect(309, "hide");
  expect(309, "break", "break_id=micro_pause break_hint=normal");
  expect(309, "show");
  expect(309, "break_event", "break_id=micro_pause event=ShowBreak");

  expect(320, "hide");
  expect(320, "break_event", "break_id=micro_pause event=BreakTaken");
  expect(320, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(320, "break_event", "break_id=micro_pause event=BreakStop");
  tick(false, 40);

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_takes_break_halfway_first_prelude)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  tick(true, 315);

  expect(315, "hide");
  expect(315, "break", "break_id=micro_pause break_hint=normal");
  expect(315, "show");
  expect(315, "break_event", "break_id=micro_pause event=ShowBreak");
  tick(false, 10);

  expect(335, "hide");
  expect(335, "break_event", "break_id=micro_pause event=BreakTaken");
  expect(335, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(335, "break_event", "break_id=micro_pause event=BreakStop");
  tick(false, 30);

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_takes_break_at_end_of_first_prelude)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  tick(true, 329);

  expect(329, "hide");
  expect(329, "break", "break_id=micro_pause break_hint=normal");
  expect(329, "show");
  expect(329, "break_event", "break_id=micro_pause event=ShowBreak");
  expect(349, "hide");
  expect(349, "break_event", "break_id=micro_pause event=BreakTaken");
  expect(349, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(349, "break_event", "break_id=micro_pause event=BreakStop");
  tick(false, 40);

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_takes_break_at_end_of_first_prelude__idle_detect_delayed)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  tick(true, 334);

  expect(334, "hide");
  expect(334, "break", "break_id=micro_pause break_hint=normal");
  expect(334, "show");
  expect(334, "break_event", "break_id=micro_pause event=ShowBreak");

  expect(354, "hide");
  expect(354, "break_event", "break_id=micro_pause event=BreakTaken");
  expect(354, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(354, "break_event", "break_id=micro_pause event=BreakStop");
  tick(false, 40);

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_ignores_first_prelude__idle_detect_delayed)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  tick(true, 334);
  monitor->notify();

  expect(334, "hide");
  expect(334, "break_event", "break_id=micro_pause event=BreakIgnored");
  expect(334, "break_event", "break_id=micro_pause event=BreakIdle");

  tick(true, 1);

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_takes_break_after_first_prelude_active_during_break)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  tick(true, 310);

  expect(310, "hide");
  expect(310, "break", "break_id=micro_pause break_hint=normal");
  expect(310, "show");
  expect(310, "break_event", "break_id=micro_pause event=ShowBreak");
  tick(false, 10);

  tick(true, 20);

  expect(350, "hide");
  expect(350, "break_event", "break_id=micro_pause event=BreakTaken");
  expect(350, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(350, "break_event", "break_id=micro_pause event=BreakStop");
  tick(false, 30);

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_active_during_prelude)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  tick(true, 310);

  expect(310, "hide");
  expect(310, "break", "break_id=micro_pause break_hint=normal");
  expect(310, "show");
  expect(310, "break_event", "break_id=micro_pause event=ShowBreak");
  tick(false, 10);
  tick(true, 10);

  expect(340, "hide");
  expect(340, "break_event", "break_id=micro_pause event=BreakTaken");
  expect(340, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(340, "break_event", "break_id=micro_pause event=BreakStop");
  tick(false, 30);

  verify();
}

BOOST_AUTO_TEST_CASE(test_forced_break)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  expect(335, "hide");
  expect(335, "break_event", "break_id=micro_pause event=BreakIgnored");
  expect(335, "break_event", "break_id=micro_pause event=BreakIdle");

  expect(451, "prelude", "break_id=micro_pause");
  expect(451, "show");
  expect(451, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(486, "hide");
  expect(486, "break_event", "break_id=micro_pause event=BreakIgnored");
  expect(486, "break_event", "break_id=micro_pause event=BreakIdle");

  expect(602, "prelude", "break_id=micro_pause");
  expect(602, "show");
  expect(602, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(631, "hide");
  expect(631, "break", "break_id=micro_pause break_hint=normal");
  expect(631, "show");
  expect(631, "break_event", "break_id=micro_pause event=ShowBreak");
  tick(true, 760);

  verify();
}

BOOST_AUTO_TEST_CASE(test_overdue_time)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  expect(315, "hide");
  expect(315, "break", "break_id=micro_pause break_hint=normal");
  expect(315, "show");
  expect(315, "break_event", "break_id=micro_pause event=ShowBreak");
  expect(335, "hide");
  expect(335, "break_event", "break_id=micro_pause event=BreakTaken");
  expect(335, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(335, "break_event", "break_id=micro_pause event=BreakStop");
  tick(true, 315);
  tick(false, 40);

  auto b = core->get_break(BREAK_ID_MICRO_BREAK);
  BOOST_CHECK_EQUAL(b->get_total_overdue_time(), 14);

  expect(655, "prelude", "break_id=micro_pause");
  expect(655, "show");
  expect(655, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(655, "break_event", "break_id=micro_pause event=BreakStart");
  expect(670, "hide");
  expect(670, "break", "break_id=micro_pause break_hint=normal");
  expect(670, "show");
  expect(670, "break_event", "break_id=micro_pause event=ShowBreak");
  expect(690, "hide");
  expect(690, "break_event", "break_id=micro_pause event=BreakTaken");
  expect(690, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(690, "break_event", "break_id=micro_pause event=BreakStop");
  tick(true, 315);
  tick(false, 40);

  BOOST_CHECK_EQUAL(b->get_total_overdue_time(), 28);

  verify();
}

BOOST_AUTO_TEST_CASE(test_insist_policy_halt)
{
  init();

  config->set_value("breaks/micro_pause/enabled", false);

  expect(1500, "prelude", "break_id=rest_break");
  expect(1500, "show");
  expect(1500, "break_event", "break_id=rest_break event=ShowPrelude");
  expect(1500, "break_event", "break_id=rest_break event=BreakStart");
  tick(true, 1501);

  expect(1509, "hide");
  expect(1509, "break", "break_id=rest_break break_hint=normal");
  expect(1509, "show");
  expect(1509, "break_event", "break_id=rest_break event=ShowBreak");

  auto rb = core->get_break(BREAK_ID_REST_BREAK);

  tick(false, 50);

  core->set_insist_policy(InsistPolicy::Halt);

  int elapsed = rb->get_elapsed_idle_time();
  tick(true, 100, [=](int) { BOOST_CHECK_EQUAL(rb->get_elapsed_idle_time(), elapsed + 1); });
  tick(false, 400);

  expect(1900, "hide");
  expect(1900, "break_event", "break_id=rest_break event=BreakTaken");
  expect(1900, "break_event", "break_id=rest_break event=BreakIdle");
  expect(1900, "break_event", "break_id=rest_break event=BreakStop");

  verify();
}

BOOST_AUTO_TEST_CASE(test_insist_policy_reset)
{
  init();

  config->set_value("breaks/micro_pause/enabled", false);

  expect(1500, "prelude", "break_id=rest_break");
  expect(1500, "show");
  expect(1500, "break_event", "break_id=rest_break event=ShowPrelude");
  expect(1500, "break_event", "break_id=rest_break event=BreakStart");
  tick(true, 1501);

  expect(1509, "hide");
  expect(1509, "break", "break_id=rest_break break_hint=normal");
  expect(1509, "show");
  expect(1509, "break_event", "break_id=rest_break event=ShowBreak");

  auto rb = core->get_break(BREAK_ID_REST_BREAK);

  tick(false, 50);

  core->set_insist_policy(InsistPolicy::Reset);

  tick(true, 100, [=](int) { BOOST_CHECK_EQUAL(rb->get_elapsed_idle_time(), 0); });
  tick(false, 400);

  expect(1951, "hide");
  expect(1951, "break_event", "break_id=rest_break event=BreakTaken");
  expect(1951, "break_event", "break_id=rest_break event=BreakIdle");
  expect(1951, "break_event", "break_id=rest_break event=BreakStop");

  verify();
}

BOOST_AUTO_TEST_CASE(test_insist_policy_ignore)
{
  init();

  config->set_value("breaks/micro_pause/enabled", false);

  expect(1500, "prelude", "break_id=rest_break");
  expect(1500, "show");
  expect(1500, "break_event", "break_id=rest_break event=ShowPrelude");
  expect(1500, "break_event", "break_id=rest_break event=BreakStart");
  tick(true, 1501);

  expect(1509, "hide");
  expect(1509, "break", "break_id=rest_break break_hint=normal");
  expect(1509, "show");
  expect(1509, "break_event", "break_id=rest_break event=ShowBreak");

  tick(false, 50);

  core->set_insist_policy(InsistPolicy::Ignore);

  tick(true, 100);
  tick(false, 400);

  expect(1800, "hide");
  expect(1800, "break_event", "break_id=rest_break event=BreakTaken");
  expect(1800, "break_event", "break_id=rest_break event=BreakIdle");
  expect(1800, "break_event", "break_id=rest_break event=BreakStop");

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_postpones_rest_break)
{
  init();

  config->set_value("breaks/micro_pause/enabled", false);

  expect(1500, "prelude", "break_id=rest_break");
  expect(1500, "show");
  expect(1500, "break_event", "break_id=rest_break event=ShowPrelude");
  expect(1500, "break_event", "break_id=rest_break event=BreakStart");
  tick(true, 1501);

  expect(1509, "hide");
  expect(1509, "break", "break_id=rest_break break_hint=normal");
  expect(1509, "show");
  expect(1509, "break_event", "break_id=rest_break event=ShowBreak");

  expect(1551, "hide");
  expect(1551, "break_event", "break_id=rest_break event=BreakPostponed");
  expect(1551, "break_event", "break_id=rest_break event=BreakIdle");
  expect(1551, "break_event", "break_id=rest_break event=BreakStop");
  tick(false, 50);
  auto b = core->get_break(BREAK_ID_REST_BREAK);
  b->postpone_break();
  tick(false, 1);

  expect(1732, "prelude", "break_id=rest_break");
  expect(1732, "show");
  expect(1732, "break_event", "break_id=rest_break event=ShowPrelude");
  expect(1732, "break_event", "break_id=rest_break event=BreakStart");
  tick(true, 200);

  verify();
}

BOOST_AUTO_TEST_CASE(test_user_skips_rest_break)
{
  init();

  config->set_value("breaks/micro_pause/enabled", false);

  expect(1500, "prelude", "break_id=rest_break");
  expect(1500, "show");
  expect(1500, "break_event", "break_id=rest_break event=ShowPrelude");
  expect(1500, "break_event", "break_id=rest_break event=BreakStart");
  tick(true, 1501);

  expect(1509, "hide");
  expect(1509, "break", "break_id=rest_break break_hint=normal");
  expect(1509, "show");
  expect(1509, "break_event", "break_id=rest_break event=ShowBreak");

  expect(1551, "hide");
  expect(1551, "break_event", "break_id=rest_break event=BreakSkipped");
  expect(1551, "break_event", "break_id=rest_break event=BreakIdle");
  expect(1551, "break_event", "break_id=rest_break event=BreakStop");
  tick(false, 50);
  auto b = core->get_break(BREAK_ID_REST_BREAK);
  b->skip_break();
  tick(false, 1);

  expect(3052, "prelude", "break_id=rest_break");
  expect(3052, "show");
  expect(3052, "break_event", "break_id=rest_break event=ShowPrelude");
  expect(3052, "break_event", "break_id=rest_break event=BreakStart");
  tick(true, 1510);

  verify();
}

BOOST_AUTO_TEST_CASE(test_quiet_during_prelude)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  tick(true, 315);

  expect(315, "operationmode", "mode=2");
  expect(315, "break_event", "break_id=micro_pause event=BreakIgnored");
  expect(315, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(315, "break_event", "break_id=micro_pause event=BreakStop");
  core->set_operation_mode(OperationMode::Quiet);

  expect(315, "hide");
  tick(false, 40);

  verify();
}

BOOST_AUTO_TEST_CASE(test_suspended_during_prelude)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  tick(true, 315);

  expect(315, "operationmode", "mode=1");
  expect(315, "break_event", "break_id=micro_pause event=BreakIgnored"); // TODO: why ignored?
  expect(315, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(315, "break_event", "break_id=micro_pause event=BreakStop");
  core->set_operation_mode(OperationMode::Suspended);

  expect(315, "hide");
  tick(false, 40);

  verify();
}

BOOST_AUTO_TEST_CASE(test_suspended_during_break)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  tick(true, 310);

  expect(310, "hide");
  expect(310, "break", "break_id=micro_pause break_hint=normal");
  expect(310, "show");
  expect(310, "break_event", "break_id=micro_pause event=ShowBreak");

  expect(320, "operationmode", "mode=1");
  expect(320, "hide");
  expect(320, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(320, "break_event", "break_id=micro_pause event=BreakStop");
  tick(false, 10);
  core->set_operation_mode(OperationMode::Suspended);

  tick(false, 30);

  verify();
}

BOOST_AUTO_TEST_CASE(test_quiet_during_break)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  tick(true, 310);

  expect(310, "hide");
  expect(310, "break", "break_id=micro_pause break_hint=normal");
  expect(310, "show");
  expect(310, "break_event", "break_id=micro_pause event=ShowBreak");

  expect(320, "operationmode", "mode=2");
  expect(320, "hide");
  expect(320, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(320, "break_event", "break_id=micro_pause event=BreakStop");
  tick(false, 10);
  core->set_operation_mode(OperationMode::Quiet);

  tick(false, 30);

  verify();
}

BOOST_AUTO_TEST_CASE(test_rest_break_now)
{
  init();

  tick(true, 10);
  tick(false, 10);

  forced_break = true;

  core->force_break(BREAK_ID_REST_BREAK, BreakHint::UserInitiated);
  expect(20, "break", "break_id=rest_break break_hint=userinitiated");
  expect(20, "show");
  expect(20, "break_event", "break_id=rest_break event=ShowBreakForced");
  // expect(20, "break_event", "break_id=rest_break event=BreakStart");

  tick(false, 50);
  tick(false, 260);

  expect(310, "hide");
  expect(310, "break_event", "break_id=rest_break event=BreakTaken");
  expect(310, "break_event", "break_id=rest_break event=BreakIdle");
  expect(310, "break_event", "break_id=rest_break event=BreakStop");

  verify();
}

BOOST_AUTO_TEST_CASE(test_rest_break_now_active_during_break)
{
  init();

  tick(true, 10);
  tick(false, 10);

  forced_break = true;

  core->force_break(BREAK_ID_REST_BREAK, BreakHint::UserInitiated);
  expect(20, "break", "break_id=rest_break break_hint=userinitiated");
  expect(20, "show");
  expect(20, "break_event", "break_id=rest_break event=ShowBreakForced");
  // expect(20, "break_event", "break_id=rest_break event=BreakStart");

  tick(false, 50);
  tick(true, 20);
  tick(false, 260);

  expect(330, "hide");
  expect(330, "break_event", "break_id=rest_break event=BreakTaken");
  expect(330, "break_event", "break_id=rest_break event=BreakIdle");
  expect(330, "break_event", "break_id=rest_break event=BreakStop");

  verify();
}

BOOST_AUTO_TEST_CASE(test_rest_break_now_during_microbreak_prelude)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  tick(true, 305);

  forced_break = true;

  core->force_break(BREAK_ID_REST_BREAK, BreakHint::UserInitiated);
  expect(305, "hide");
  expect(305, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(305, "break_event", "break_id=micro_pause event=BreakIgnored"); // TODO: why
  expect(305, "break_event", "break_id=micro_pause event=BreakStop");

  expect(305, "break", "break_id=rest_break break_hint=userinitiated");
  expect(305, "show");
  expect(305, "break_event", "break_id=rest_break event=ShowBreakForced");

  tick(false, 50);
  tick(false, 260);

  expect(604, "hide");
  expect(604, "break_event", "break_id=rest_break event=BreakTaken");
  expect(604, "break_event", "break_id=rest_break event=BreakIdle");
  expect(604, "break_event", "break_id=rest_break event=BreakStop");

  verify();
}

BOOST_AUTO_TEST_CASE(test_rest_break_now_during_microbreak)
{
  init();

  expect(300, "prelude", "break_id=micro_pause");
  expect(300, "show");
  expect(300, "break_event", "break_id=micro_pause event=ShowPrelude");
  expect(300, "break_event", "break_id=micro_pause event=BreakStart");
  tick(true, 310);

  expect(310, "hide");
  expect(310, "break", "break_id=micro_pause break_hint=normal");
  expect(310, "show");
  expect(310, "break_event", "break_id=micro_pause event=ShowBreak");

  tick(false, 10);

  forced_break = true;

  core->force_break(BREAK_ID_REST_BREAK, BreakHint::UserInitiated);
  expect(320, "hide");
  expect(320, "break_event", "break_id=micro_pause event=BreakIdle");
  expect(320, "break_event", "break_id=micro_pause event=BreakStop");

  expect(320, "break", "break_id=rest_break break_hint=userinitiated");
  expect(320, "show");
  expect(320, "break_event", "break_id=rest_break event=ShowBreakForced");
  // expect(320, "break_event", "break_id=rest_break event=BreakStart");

  tick(false, 50);
  tick(false, 260);

  expect(610, "hide");
  expect(610, "break_event", "break_id=rest_break event=BreakTaken");
  expect(610, "break_event", "break_id=rest_break event=BreakIdle");
  expect(610, "break_event", "break_id=rest_break event=BreakStop");

  verify();
}

BOOST_AUTO_TEST_CASE(test_rest_break_now_when_timer_is_disabled)
{
  init();

  config->set_value("breaks/rest_break/enabled", false);

  tick(true, 10);
  tick(false, 10);

  forced_break = true;
  fake_break = true;
  fake_break_delta = 1;

  core->force_break(BREAK_ID_REST_BREAK, BreakHint::UserInitiated);
  expect(20, "break", "break_id=rest_break break_hint=userinitiated");
  expect(20, "show");
  expect(20, "break_event", "break_id=rest_break event=ShowBreakForced");
  // expect(20, "break_event", "break_id=rest_break event=BreakStart");

  tick(false, 50);
  tick(false, 260);

  expect(319, "hide");
  // expect(319, "break_event", "break_id=rest_break event=BreakTaken");
  expect(319, "break_event", "break_id=rest_break event=BreakIdle");
  expect(319, "break_event", "break_id=rest_break event=BreakStop");

  verify();
}

BOOST_AUTO_TEST_CASE(test_rest_break_now_when_break_is_idle)
{
  init();

  tick(false, 500);

  forced_break = true;
  fake_break = true;
  fake_break_delta = 1;

  core->force_break(BREAK_ID_REST_BREAK, BreakHint::UserInitiated);
  expect(500, "break", "break_id=rest_break break_hint=userinitiated");
  expect(500, "show");
  expect(500, "break_event", "break_id=rest_break event=ShowBreakForced");
  // expect(500, "break_event", "break_id=rest_break event=BreakStart");

  tick(false, 50);
  tick(false, 260);

  expect(799, "hide");
  // expect(789, "break_event", "break_id=rest_break event=BreakTaken");
  expect(799, "break_event", "break_id=rest_break event=BreakIdle");
  expect(799, "break_event", "break_id=rest_break event=BreakStop");

  verify();
}

BOOST_AUTO_TEST_CASE(test_advance_imminent_rest_break)
{
  init();

  config->set_value("timers/micro_pause/limit", 300);
  config->set_value("timers/micro_pause/auto_reset", 20);

  config->set_value("timers/rest_break/limit", 330);
  config->set_value("timers/rest_break/auto_reset", 300);

  expect(300, "prelude", "break_id=rest_break");
  expect(300, "show");
  expect(300, "break_event", "break_id=rest_break event=BreakStart");
  expect(300, "break_event", "break_id=rest_break event=ShowPrelude");
  expect(309, "hide");
  expect(309, "break", "break_id=rest_break break_hint=normal");
  expect(309, "show");
  expect(309, "break_event", "break_id=rest_break event=ShowBreak");

  tick(true, 305);
  tick(false, 10);

  verify();
}

BOOST_AUTO_TEST_CASE(test_advance_imminent_rest_break_max_prelude_count_taken_from_micro_break)
{
  init();

  config->set_value("timers/micro_pause/limit", 300);
  config->set_value("timers/micro_pause/auto_reset", 20);
  config->set_value("breaks/micro_pause/max_preludes", 1);

  config->set_value("timers/rest_break/limit", 330);
  config->set_value("timers/rest_break/auto_reset", 300);
  config->set_value("breaks/rest_break/max_preludes", 6);

  max_preludes = 1;
  expect(300, "prelude", "break_id=rest_break");
  expect(300, "show");
  expect(300, "break_event", "break_id=rest_break event=BreakStart");
  expect(300, "break_event", "break_id=rest_break event=ShowPrelude");
  expect(329, "hide");
  expect(329, "break", "break_id=rest_break break_hint=normal");
  expect(329, "show");
  expect(329, "break_event", "break_id=rest_break event=ShowBreak");

  tick(true, 350);

  verify();
}

BOOST_AUTO_TEST_CASE(test_two_breaks_at_the_same_time)
{
  init();

  config->set_value("timers/micro_pause/limit", 300);
  config->set_value("timers/rest_break/limit", 300);

  expect(300, "prelude", "break_id=rest_break");
  expect(300, "show");
  expect(300, "break_event", "break_id=rest_break event=BreakStart");
  expect(300, "break_event", "break_id=rest_break event=ShowPrelude");
  expect(309, "hide");
  expect(309, "break", "break_id=rest_break break_hint=normal");
  expect(309, "show");
  expect(309, "break_event", "break_id=rest_break event=ShowBreak");

  tick(true, 305);
  tick(false, 50);

  verify();
}

BOOST_AUTO_TEST_CASE(test_daily_limit_postpone)
{
  init();

  config->set_value("breaks/micro_pause/enabled", false);
  config->set_value("breaks/rest_break/enabled", false);
  config->set_value("timers/daily_limit/reset_pred", "");
  config->set_value("timers/daily_limit/limit", 7200);
  config->set_value("timers/daily_limit/snooze", 600);

  expect(7200, "prelude", "break_id=daily_limit");
  expect(7200, "show");
  expect(7200, "break_event", "break_id=daily_limit event=BreakStart");
  expect(7200, "break_event", "break_id=daily_limit event=ShowPrelude");
  tick(true, 7201);

  expect(7209, "hide");
  expect(7209, "break", "break_id=daily_limit break_hint=normal");
  expect(7209, "show");
  expect(7209, "break_event", "break_id=daily_limit event=ShowBreak");
  tick(false, 20);

  auto b = core->get_break(BREAK_ID_DAILY_LIMIT);
  b->postpone_break();

  expect(7221, "hide");
  expect(7221, "break_event", "break_id=daily_limit event=BreakPostponed");
  expect(7221, "break_event", "break_id=daily_limit event=BreakIdle");
  expect(7221, "break_event", "break_id=daily_limit event=BreakStop");
  tick(false, 1);

  expect(7822, "prelude", "break_id=daily_limit");
  expect(7822, "show");
  expect(7822, "break_event", "break_id=daily_limit event=ShowPrelude");
  expect(7822, "break_event", "break_id=daily_limit event=BreakStart");
  tick(true, 620);

  verify();
}

BOOST_AUTO_TEST_CASE(test_daily_limit_skip)
{
  init();

  config->set_value("breaks/micro_pause/enabled", false);
  config->set_value("breaks/rest_break/enabled", false);
  config->set_value("timers/daily_limit/reset_pred", "");
  config->set_value("timers/daily_limit/limit", 7200);
  config->set_value("timers/daily_limit/snooze", 600);

  expect(7200, "prelude", "break_id=daily_limit");
  expect(7200, "show");
  expect(7200, "break_event", "break_id=daily_limit event=BreakStart");
  expect(7200, "break_event", "break_id=daily_limit event=ShowPrelude");
  tick(true, 7201);

  expect(7209, "hide");
  expect(7209, "break", "break_id=daily_limit break_hint=normal");
  expect(7209, "show");
  expect(7209, "break_event", "break_id=daily_limit event=ShowBreak");
  tick(false, 20);

  auto b = core->get_break(BREAK_ID_DAILY_LIMIT);
  b->skip_break();

  expect(7221, "hide");
  expect(7221, "break_event", "break_id=daily_limit event=BreakSkipped");
  expect(7221, "break_event", "break_id=daily_limit event=BreakIdle");
  expect(7221, "break_event", "break_id=daily_limit event=BreakStop");
  tick(false, 1);
  tick(true, 3600);

  verify();
}

BOOST_AUTO_TEST_CASE(test_daily_limit_regard_micro_break_as_activity)
{
  init();

  config->set_value("breaks/rest_break/enabled", false);
  config->set_value("timers/daily_limit/use_microbreak_activity", true);

  config->set_value("timers/micro_pause/limit", 300);
  config->set_value("timers/micro_pause/auto_reset", 8000);
  config->set_value("timers/micro_pause/reset_pred", "");
  config->set_value("timers/micro_pause/snooze", 150);

  config->set_value("timers/daily_limit/limit", 7200);
  config->set_value("timers/daily_limit/auto_reset", 0);
  config->set_value("timers/daily_limit/reset_pred", "");
  config->set_value("timers/daily_limit/snooze", 1200);

  tick(true, 1);

  expect(7201, "prelude", "break_id=daily_limit");
  expect(7201, "show");
  expect(7201, "break_event", "break_id=daily_limit event=BreakStart");
  expect(7201, "break_event", "break_id=daily_limit event=ShowPrelude");
  tick(false, 7200);

  expect(7210, "hide");
  expect(7210, "break", "break_id=daily_limit break_hint=normal");
  expect(7210, "show");
  expect(7210, "break_event", "break_id=daily_limit event=ShowBreak");
  tick(false, 20);

  auto b = core->get_break(BREAK_ID_DAILY_LIMIT);
  b->skip_break();

  expect(7221, "hide");
  expect(7221, "break_event", "break_id=daily_limit event=BreakSkipped");
  expect(7221, "break_event", "break_id=daily_limit event=BreakIdle");
  expect(7221, "break_event", "break_id=daily_limit event=BreakStop");
  tick(false, 1);
  tick(false, 3600);

  verify();
}

BOOST_AUTO_TEST_CASE(test_daily_limit_regard_micro_break_as_activity_off)
{
  init();

  config->set_value("breaks/rest_break/enabled", false);
  config->set_value("timers/daily_limit/use_microbreak_activity", false);

  config->set_value("timers/micro_pause/limit", 300);
  config->set_value("timers/micro_pause/auto_reset", 8000);
  config->set_value("timers/micro_pause/reset_pred", "");
  config->set_value("timers/micro_pause/snooze", 150);

  config->set_value("timers/daily_limit/limit", 7200);
  config->set_value("timers/daily_limit/auto_reset", 0);
  config->set_value("timers/daily_limit/reset_pred", "");
  config->set_value("timers/daily_limit/snooze", 1200);

  tick(true, 1);
  tick(false, 8000);

  verify();
}

BOOST_AUTO_TEST_CASE(test_daily_limit_reset)
{
  init();

  auto b = core->get_break(BREAK_ID_DAILY_LIMIT);

  config->set_value("breaks/micro_pause/enabled", false);
  config->set_value("breaks/rest_break/enabled", false);

  config->set_value("timers/daily_limit/reset_pred", "day/23:00");
  config->set_value("timers/daily_limit/limit", 7200);
  config->set_value("timers/daily_limit/snooze", 600);

  tick(true, 100);

  BOOST_CHECK_EQUAL(b->get_elapsed_time(), 99);

  tick(false, 3500);

  BOOST_CHECK_EQUAL(b->get_elapsed_time(), 100);

  tick(false, 1);

  BOOST_CHECK_EQUAL(b->get_elapsed_time(), 0);

  verify();
}

// TODO: daily limit + change limit
// TODO: daily limit + statistics reset
// TODO: forced restbreak in reading mode (active state)

BOOST_AUTO_TEST_SUITE_END()
