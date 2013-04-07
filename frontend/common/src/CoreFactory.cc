// CoreFactory.cc
//
// Copyright (C) 2001, 2002, 2003, 2006, 2007, 2012, 2013 Rob Caelers & Raymond Penners
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

#include <assert.h>

#include "CoreFactory.hh"

#include "config/IConfigurator.hh"
#include "ICore.hh"

using namespace workrave;
using namespace workrave::config;
using namespace workrave::dbus;

ICore::Ptr CoreFactory::core;

//! Returns the interface to the core.
ICore::Ptr
CoreFactory::get_core()
{
  if (!core)
    {
      core = ICore::create();
    }
  
  return core;
}


//! Returns the interface to the configurator
IConfigurator::Ptr
CoreFactory::get_configurator()
{
  return get_core()->get_configurator();
}


//! Returns the interface to the D-BUS facility
DBus::Ptr
CoreFactory::get_dbus()
{
  return get_core()->get_dbus();
}
