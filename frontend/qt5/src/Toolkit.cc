// Toolkit.cc --- The WorkRave GUI Configuration
//
// Copyright (C) 2007 - 2013 Rob Caelers & Raymond Penners
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

#include "Toolkit.hh"

#include "config/IConfigurator.hh"

#include "GUIConfig.hh"

using namespace std;
using namespace workrave;
using namespace workrave::config;

IToolkit::Ptr
Toolkit::create(int argc, char **argv)
{
  return Ptr(new Toolkit(argc, argv));
}


Toolkit::Toolkit(int argc, char **argv)
  : QApplication(argc, argv),
    heartbeat_timer(new QTimer(this))
{
}


Toolkit::~Toolkit()
{
}

void
Toolkit::init()
{
  main_window =  boost::shared_ptr<MainWindow>(new MainWindow());
  connect(heartbeat_timer.get(), SIGNAL(timeout()), this, SLOT(on_timer()));
  heartbeat_timer->start(1000);

  main_window->show();
  main_window->raise();
}

void
Toolkit::run()
{
  exec();
}

void Toolkit::grab()
{
}

void Toolkit::ungrab()
{
}

std::string Toolkit::get_display_name()
{
  return "";
}

IBreakWindow::Ptr
Toolkit::create_break_window(HeadInfo &head, BreakId break_id, IBreakWindow::BreakFlags break_flags)
{
  IBreakWindow::Ptr ret;
  
  GUIConfig::BlockMode block_mode = GUIConfig::get_block_mode();
  //if (break_id == BREAK_ID_MICRO_BREAK)
  //  {
  //    ret = new MicroBreakWindow(head, break_flags, block_mode);
  //  }
  //else if (break_id == BREAK_ID_REST_BREAK)
  //  {
  //    ret = new RestBreakWindow(head, break_flags, block_mode);
  //  }
  //else if (break_id == BREAK_ID_DAILY_LIMIT)
  //  {
  //    ret = new DailyLimitWindow(head, break_flags, block_mode);
  //  }

  return ret;
}


IPreludeWindow::Ptr
Toolkit::create_prelude_window(HeadInfo &head, workrave::BreakId break_id)
{
  IPreludeWindow::Ptr ret;
  return ret;
}

void
Toolkit::on_timer()
{
  timer_signal();

  main_window->on_heartbeat();
}

boost::signals2::signal<void()> &
Toolkit::signal_timer()
{
  return timer_signal;
}
 
