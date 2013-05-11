// Tookit.cc
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

#include <QDesktopWidget>
#include <QApplication>

#include "config/IConfigurator.hh"

#include "GUIConfig.hh"

#include "PreludeWindow.hh"
#include "MicroBreakWindow.hh"
#include "RestBreakWindow.hh"
#include "DailyLimitWindow.hh"
#include "PreferencesDialog.hh"

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

  PreludeWindow *prelude = new PreludeWindow(0, BREAK_ID_MICRO_BREAK);

  // prelude->set_progress(20, 30);
  // prelude->set_progress_text(IApp::PROGRESS_TEXT_BREAK_IN);
  // prelude->set_stage(IApp::STAGE_WARN);
  // prelude->start();
  // prelude->refresh();

  //MicroBreakWindow *mb = new MicroBreakWindow(0, IBreakWindow::BREAK_FLAGS_SKIPPABLE | IBreakWindow::BREAK_FLAGS_POSTPONABLE, GUIConfig::BLOCK_MODE_INPUT);
  RestBreakWindow *mb = new RestBreakWindow(0, IBreakWindow::BREAK_FLAGS_SKIPPABLE | IBreakWindow::BREAK_FLAGS_POSTPONABLE, GUIConfig::BLOCK_MODE_INPUT);
  //DailyLimitWindow *mb = new DailyLimitWindow(0, IBreakWindow::BREAK_FLAGS_SKIPPABLE | IBreakWindow::BREAK_FLAGS_POSTPONABLE, GUIConfig::BLOCK_MODE_INPUT);
  
  // mb->init();
  // mb->set_progress(40, 30);
  // mb->start();
  // mb->refresh();

  PreferencesDialog *pd = new PreferencesDialog();
  pd->show();
  
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
Toolkit::create_break_window(int screen, BreakId break_id, IBreakWindow::BreakFlags break_flags)
{
  IBreakWindow::Ptr ret;
  
  GUIConfig::BlockMode block_mode = GUIConfig::get_block_mode();
  if (break_id == BREAK_ID_MICRO_BREAK)
   {
     ret = MicroBreakWindow::create(screen, break_flags, block_mode);
   }
  else if (break_id == BREAK_ID_REST_BREAK)
   {
     ret = RestBreakWindow::create(screen, break_flags, block_mode);
   }
  else if (break_id == BREAK_ID_DAILY_LIMIT)
   {
     ret = DailyLimitWindow::create(screen, break_flags, block_mode);
   }

  return ret;
}


IPreludeWindow::Ptr
Toolkit::create_prelude_window(int screen, workrave::BreakId break_id)
{
  return PreludeWindow::create(screen, break_id);
}


int
Toolkit::get_screen_count() const
{
  QDesktopWidget *dw = QApplication::desktop();
  return dw->screenCount();
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
 
