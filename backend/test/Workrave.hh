// Workrave.hh
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

#ifndef WORKRAVE_HH
#define WORKRAVE_HH

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/function.hpp>

#include "IApp.hh"
#include "ICore.hh"
#include "Networking.hh"

#include "utils/ITimeSource.hh"

using namespace workrave;
using namespace workrave::utils;

class Workrave : public IApp, public ITimeSource
{
public:
  typedef boost::shared_ptr<Workrave> Ptr;

public:
  static Ptr create(int id);

  Workrave(int id);
  virtual ~Workrave();

  void init(boost::shared_ptr<boost::barrier> barrier);
  void heartbeat();
  void connect(const std::string host, int port);

  void set_active(bool on);
  void log(const std::string &txt);
  
  ICore::Ptr get_core() const;
  IFog::Ptr get_fog() const;
  
  void create_prelude_window(BreakId break_id)  { }
  void create_break_window(BreakId break_id, BreakHint break_hint)  { }
  void hide_break_window()  { }
  void show_break_window()  { }
  void refresh_break_window()  { }
  void set_break_progress(int value, int max_value)  { }
  void set_prelude_stage(PreludeStage stage)  { }
  void set_prelude_progress_text(PreludeProgressText text)  { }
  void terminate();

  // ITimeSource
  gint64 get_real_time_usec();
  gint64 get_monotonic_time_usec();

  template<typename Functor>
  struct Closure
  {
    typedef typename boost::result_of<Functor()>::type result_type;
    typedef boost::packaged_task<result_type> task_type;
    typedef boost::shared_ptr<boost::packaged_task<result_type> > task_ptr_type;
 
    Functor functor;
    task_ptr_type task;
    boost::unique_future<result_type> future;
    
    Closure(Functor f) : functor(f), task(new task_type(f))
    {
      TRACE_ENTER("Closure::Closure");
      future = task->get_future();
      TRACE_EXIT();
    }
    
    static gboolean invoke(gpointer data)
    {
      TRACE_ENTER("Closure::invoke");
      Closure<Functor> *c = (Closure<Functor> *)data;
      (*(c->task.get()))();
      TRACE_EXIT();
      return FALSE;
    }
    
    static void destroy(gpointer data)
    {
      TRACE_ENTER("Closure::destroy");
      Closure *c = (Closure *)data;
      delete c;
      TRACE_EXIT();
    }
  };


  template<typename Functor>
  boost::unique_future<typename boost::result_of<Functor()>::type>
  invoke(Functor functor)
  {
    TRACE_ENTER("Workrave::invoke");
    typedef typename boost::result_of<Functor()>::type result_type;

    Closure<Functor> *c = new Closure<Functor>(functor);
  
    g_main_context_invoke_full(context,
                               G_PRIORITY_DEFAULT_IDLE,
                               Closure<Functor>::invoke,
                               c,
                               Closure<Functor>::destroy);

    TRACE_EXIT();
    return boost::move(c->future);
  }


  template<class Functor>
  typename boost::disable_if< boost::is_void<typename boost::result_of<Functor()>::type>, typename boost::result_of<Functor()>::type>::type
  invoke_sync(Functor functor)
  {
    TRACE_ENTER("Workrave::invoke_sync");
    typedef typename boost::result_of<Functor()>::type result_type;

    Closure<Functor> *c = new Closure<Functor>(functor);
  
    g_main_context_invoke_full(context,
                               G_PRIORITY_DEFAULT_IDLE,
                               Closure<Functor>::invoke,
                               c,
                               NULL);
    
    c->future.wait();
    TRACE_EXIT();
    result_type ret = c->future.get();
    delete c;
    return ret;
  }

  template<class Functor>
  typename boost::enable_if< boost::is_void<typename boost::result_of<Functor()>::type>, typename boost::result_of<Functor()>::type>::type
  invoke_sync(Functor functor)
  {
    TRACE_ENTER("Workrave::invoke_sync.void");
    typedef typename boost::result_of<Functor()>::type result_type;

    Closure<Functor> *c = new Closure<Functor>(functor);
  
    g_main_context_invoke_full(context,
                               G_PRIORITY_DEFAULT_IDLE,
                               Closure<Functor>::invoke,
                               c,
                               NULL);
    
    c->future.wait();
    delete c;
    TRACE_EXIT();
  }
  
private:
  ActivityState on_local_activity_state();
  IConfigurator::Ptr on_create_configurator();

  static gpointer static_workrave_thread(gpointer data);
  void run();
  
private:
  int id;
  
  ICore::Ptr core;

  Networking::Ptr networking;
  IFog::Ptr fog;

  boost::shared_ptr<boost::thread> thread;
  boost::shared_ptr<boost::barrier> barrier;

  //! The current wall clocl time.
  gint64 current_real_time;
  
  //! The current monotonic time.
  gint64 current_monotonic_time;

  ActivityState activity_state;
  
  GMainContext *context;
  GMainLoop *loop;
};

#endif // WORKRAVE_HH
