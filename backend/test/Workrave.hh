// Workrave.hh
//
// Copyright (C) 2012 Rob Caelers <robc@krandor.nl>
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

using namespace workrave;

class Workrave : public IApp
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

  ICore::Ptr get_core() const;
  
  void create_prelude_window(BreakId break_id)  { }
  void create_break_window(BreakId break_id, BreakHint break_hint)  { }
  void hide_break_window()  { }
  void show_break_window()  { }
  void refresh_break_window()  { }
  void set_break_progress(int value, int max_value)  { }
  void set_prelude_stage(PreludeStage stage)  { }
  void set_prelude_progress_text(PreludeProgressText text)  { }
  void terminate()  { }

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
      future = task->get_future();
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
      Closure *c = (Closure *)data;
      delete c;
    }
  };
  
  template<typename Functor>
  boost::unique_future<typename boost::result_of<Functor()>::type>
  invoke(Functor functor)
  {
    typedef typename boost::result_of<Functor()>::type result_type;

    Closure<Functor> *c = new Closure<Functor>(functor);
  
    g_main_context_invoke_full(context,
                               G_PRIORITY_DEFAULT_IDLE,
                               Closure<Functor>::invoke,
                               c,
                               Closure<Functor>::destroy);
    
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
    return c->future.get();
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
    TRACE_EXIT();
  }
  
  // template<typename Functor>
  // typename boost::result_of<Functor()>::type
  // invoke_sync(Functor functor)
  // {
  //   typedef typename boost::result_of<Functor()>::type result_type;

  //   Closure<Functor> *c = new Closure<Functor>(functor);
  
  //   g_main_context_invoke_full(context,
  //                              G_PRIORITY_DEFAULT_IDLE,
  //                              Closure<Functor>::invoke,
  //                              c,
  //                              NULL);
    
  //   c->future.wait();
  //   return c->get();
  // }
  
private:
  ActivityState on_local_activity_state();
  IConfigurator::Ptr on_create_configurator();

  static gpointer static_workrave_thread(gpointer data);
  static gboolean static_on_timer(gpointer data);
  void run();
  
private:
  int id;
  
  ICore::Ptr core;

  //! Current state
  Networking::Ptr networking;

  boost::shared_ptr<boost::thread> thread;
  boost::shared_ptr<boost::barrier> barrier;
  
  GMainContext *context;
  GMainLoop *loop;
  //GThread *thread;
};

#endif // WORKRAVE_HH
