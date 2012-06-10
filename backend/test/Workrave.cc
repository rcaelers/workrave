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

#include "Workrave.hh"
#include "ICore.hh"

Workrave::Ptr
Workrave::create()
{
  return Ptr(new Workrave());
}


Workrave::Workrave()
{
}


Workrave::~Workrave()
{
}


void
Workrave::init()
{
  core = ICore::create();

  char *argv[] = { (char *)"workrave" };
  
  core->init(1, argv, this, "");

  // for (int i = 0; i < BREAK_ID_SIZEOF; i++)
  //   {
  //     IBreak::Ptr b = core->get_break(BreakId(i));
  //     b->signal_break_event().connect(boost::bind(&GUI::on_break_event, this, BreakId(i), _1));
  //   }

  // core->signal_operation_mode_changed().connect(boost::bind(&GUI::on_operation_mode_changed, this, _1)); 
  // core->signal_usage_mode_changed().connect(boost::bind(&GUI::on_usage_mode_changed, this, _1));
  
  networking = Networking::create(core);
}

void
Workrave::connect(const std::string host, int port)
{
  networking->connect(host, port);
}

void
Workrave::heartbeat()
{
  networking->heartbeat();
}

