//
// Copyright (C) 2001 - 2010, 2012 Rob Caelers <robc@krandor.nl>
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

#include "debug.hh"

#include <boost/lexical_cast.hpp>

#include "Workrave.hh"

#include "ICore.hh"
#include "ICoreHooks.hh"
#include "ICoreTestHooks.hh"

Workrave::Ptr
Workrave::create(int id)
{
  return Ptr(new Workrave(id));
}


Workrave::Workrave(int id) : id(id)
{
}


Workrave::~Workrave()
{
  TRACE_ENTER_MSG("Workrave::~Workrave", id);
  TRACE_EXIT();
}


ActivityState
Workrave::on_local_activity_state()
{
  return ACTIVITY_IDLE;
}


IConfigurator::Ptr
Workrave::on_create_configurator()
{
  IConfigurator::Ptr configurator = ConfiguratorFactory::create(ConfiguratorFactory::FormatIni);

  configurator->set_value("plugins/networking/user", "robc@workrave");
  configurator->set_value("plugins/networking/secret", "HelloWorld");
  configurator->set_value("plugins/networking/port", 2700 + id);

  return configurator;
}

void
Workrave::init(boost::shared_ptr<boost::barrier> barrier)
{
  this->barrier = barrier;
  thread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&Workrave::run, this)));
}

void
Workrave::terminate()
{
  g_main_loop_quit(loop);
  //barrier->wait();
}


void
Workrave::run()
{
#ifdef TRACING
  Debug::name(std::string("core-") + boost::lexical_cast<std::string>(id));
#endif

  TRACE_ENTER_MSG("Workrave::run", id);
  core = ICore::create(id);

  char *argv[] = { (char *)"workrave" };
  
  context = g_main_context_new();
  g_main_context_push_thread_default(context);
  loop = g_main_loop_new(context, FALSE);
  
  ICoreHooks::Ptr hooks = core->get_hooks();
  ICoreTestHooks::Ptr test_hooks = boost::dynamic_pointer_cast<ICoreTestHooks>(hooks);

  test_hooks->hook_local_activity_state() = boost::bind(&Workrave::on_local_activity_state, this);
  test_hooks->hook_create_configurator() = boost::bind(&Workrave::on_create_configurator, this);

  core->init(1, argv, this, "");
  
  networking = Networking::create(core);
  networking->init();

  GSource *source = g_timeout_source_new_seconds(1);
  g_source_set_callback(source, static_on_timer, this, NULL);
  g_source_attach(source, context);
  
  TRACE_MSG("Loop run");
  barrier->wait();
  g_main_loop_run(loop);
  TRACE_EXIT();
}

void
Workrave::connect(const std::string host, int port)
{
  TRACE_ENTER_MSG("Workrave::connect", host << " " << port);
  networking->connect(host, port);
  TRACE_EXIT();
}

void
Workrave::heartbeat()
{
  TRACE_ENTER_MSG("Workrave::heartbeat", id);
  networking->heartbeat();
  TRACE_EXIT();
}


ICore::Ptr
Workrave::get_core() const
{
  return core;
}


gpointer
Workrave::static_workrave_thread(gpointer data)
{
  Workrave *w = (Workrave *) data;
  if (w != NULL)
    {
      w->run();
    }

  return 0;
}

gboolean
Workrave::static_on_timer(gpointer data)
{
  Workrave *w = (Workrave *)data;
  
  w->heartbeat();
  
  return G_SOURCE_CONTINUE;
}
