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

#ifdef HAVE_QT5
#include <QtCore>
#endif

#include "ICore.hh"
#include "IApp.hh"
#include "IBreak.hh"

#include "utils/ITimeSource.hh"
#include "utils/TimeSource.hh"
#include "config/Config.hh"
#include "ICoreTestHooks.hh"

#include "Test.hh"

using namespace std;
using namespace workrave::utils;
using namespace workrave::config;
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

class Backend : public workrave::IApp
{
public:
  Backend() : user_active(false)
  {
    string display_name = "";

#ifdef HAVE_QT5
    int argc = 1;
    char argv1[] = "test";
    char* argv[] = {argv1, NULL};
    app = new QCoreApplication(argc, argv);
#endif

    core = workrave::ICore::create();
    
    ICoreHooks::Ptr hooks = core->get_hooks();
    ICoreTestHooks::Ptr test_hooks = boost::dynamic_pointer_cast<ICoreTestHooks>(hooks);
    
    test_hooks->hook_is_user_active() = boost::bind(&Backend::on_is_user_active, this, _1);
    test_hooks->hook_create_configurator() = boost::bind(&Backend::on_create_configurator, this);
    
    core->init(this, "");

    for (int i = 0; i < workrave::BREAK_ID_SIZEOF; i++)
      {
        workrave::IBreak::Ptr b = core->get_break(workrave::BreakId(i));
        b->signal_break_event().connect(boost::bind(&Backend::on_break_event, this, workrave::BreakId(i), _1));
      }

    core->signal_operation_mode_changed().connect(boost::bind(&Backend::on_operation_mode_changed, this, _1)); 

    sim = SimulatedTime::Ptr(new SimulatedTime());
  }
  
  ~Backend()
  {
#ifdef HAVE_QT5
    delete app;
#endif
    BOOST_TEST_MESSAGE("destructing...done");
   }

  void init()
  {
    sim->init();
  }

  void tick()
  {
    core->heartbeat();
    sim->current_time += 1000000;
  }
  
  virtual void create_prelude_window(workrave::BreakId break_id)
  {
  }
  
  virtual void create_break_window(workrave::BreakId break_id, workrave::BreakHint break_hint)
  {
  }
  
  virtual void hide_break_window()
  {
  }
  
  virtual void show_break_window()
  {
  }
  
  virtual void refresh_break_window()
  {
  }
  
  virtual void set_break_progress(int value, int max_value)
  {
  }
  
  virtual void set_prelude_stage(PreludeStage stage)
  {
  }
  
  virtual void set_prelude_progress_text(PreludeProgressText text)
  {
  }

  virtual void on_break_event(workrave::BreakId break_id, workrave::BreakEvent event)
  {
  }
  
  virtual void on_operation_mode_changed(const workrave::OperationMode m)
  {
  }

  bool on_is_user_active(bool dummy)
  {
    return user_active;
  }
  
  IConfigurator::Ptr on_create_configurator()
  {
    IConfigurator::Ptr config = ConfiguratorFactory::create(ConfiguratorFactory::FormatIni);
    
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
  
  workrave::ICore::Ptr core;
  SimulatedTime::Ptr sim;
  bool user_active;
#ifdef HAVE_QT5
  QCoreApplication *app;
#endif
};

BOOST_FIXTURE_TEST_SUITE(s, Backend)

BOOST_AUTO_TEST_CASE(test_microbreak_elasped_time)
{
  init();

  workrave::IBreak::Ptr mb = core->get_break(BREAK_ID_MICRO_BREAK);

  user_active = false;
  for (int i = 0; i < 100; i++)
    {
      tick();
      BOOST_REQUIRE_EQUAL(mb->get_elapsed_idle_time(), 20 + i);
      BOOST_REQUIRE_EQUAL(mb->get_elapsed_time(), 0);
    }

  user_active = true;
  for (int i = 0; i < 20; i++)
    {
      tick();
      BOOST_REQUIRE_EQUAL(mb->get_elapsed_idle_time(), 0);
      BOOST_REQUIRE_EQUAL(mb->get_elapsed_time(), i);
    }

  user_active = false;
  for (int i = 0; i < 100; i++)
    {
      tick();
      BOOST_REQUIRE_EQUAL(mb->get_elapsed_idle_time(), i);
      BOOST_REQUIRE_EQUAL(mb->get_elapsed_time(), i < 20 ? 20 : 0);
    }
}

// BOOST_AUTO_TEST_CASE(test_microbreak_limit_reached)
// {
//   init();

//   workrave::IBreak::Ptr mb = core->get_break(BREAK_ID_MICRO_BREAK);
//   for (int i = 0; i < 100; i++)
//     {
//       tick();

//       BOOST_REQUIRE_EQUAL(mb->get_elapsed_idle_time(), 20);
//       BOOST_REQUIRE_EQUAL(mb->get_elapsed_time(), 0);
//     }

//   user_active = true;
  
//   for (int i = 0; i < 300; i++)
//     {
//       tick();

//       std::cout << mb->get_elapsed_idle_time() << std::endl;
//       BOOST_REQUIRE_EQUAL(mb->get_elapsed_idle_time(), 0);
//       BOOST_REQUIRE_EQUAL(mb->get_elapsed_time(), i);
//     }
  
// }

BOOST_AUTO_TEST_SUITE_END()
