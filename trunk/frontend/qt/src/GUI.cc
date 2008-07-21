// GUI.cc
//
// Copyright (C) 2006, 2007 Raymond Penners <raymond@dotsphinx.com>
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
// $Id: IniConfigurator.hh 558 2006-02-23 19:42:12Z rcaelers $
//

#include <QApplication>
#include <QDialog>
#include <stdio.h>

#include "GUI.hh"
#include "CoreFactory.hh"

GUI::GUI(int argc, char *argv[])
  : QApplication(argc, argv, true)
{
  heartbeat_timer = new QTimer(this);
  connect(heartbeat_timer, SIGNAL(timeout()),
          this, SLOT(on_timer()));
  heartbeat_timer->start(1000);


  core = CoreFactory::get_core();
  char *display_name = 0;
  core->init(argc, argv, this, display_name);
  //core->set_core_events_listener(this);

  status_window = new StatusWindow();
  status_window->show();
  status_window->raise();
}

GUI::~GUI()
{
  delete status_window;
  delete heartbeat_timer;
}

void
GUI::on_timer()
{
  core->heartbeat();
  status_window->on_heartbeat();
}

void
GUI::set_break_response(IBreakResponse *rep)
{
}

void
GUI::start_prelude_window(BreakId break_id)
{
}

void
GUI::start_break_window(BreakId break_id, bool ignorable)
{
}

void
GUI::hide_break_window()
{
}

void
GUI::refresh_break_window()
{
}

void
GUI::set_break_progress(int value, int max_value)
{
}

void
GUI::set_prelude_stage(PreludeStage stage)
{
}

void
GUI::set_prelude_progress_text(PreludeProgressText text)
{
}

