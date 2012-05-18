// CoreFactory.cc
//
// Copyright (C) 2001, 2002, 2003, 2006, 2007, 2012 Rob Caelers & Raymond Penners
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

#include "IConfigurator.hh"
#include "CoreFactory.hh"
#include "Core.hh"

//! Returns the interface to the core.
ICore *
CoreFactory::get_core()
{
  return Core::get_instance();
}


//! Returns the interface to the configurator
IConfigurator *
CoreFactory::get_configurator()
{
  Core *core = Core::get_instance();
  assert(core != NULL);

  return core->get_configurator();
}


// //! Returns the interface to the networking facility
// INetwork *
// CoreFactory::get_networking()
// {
// #ifdef HAVE_DISTRIBUTION
//   Core *core = Core::get_instance();
//   assert(core != NULL);

//   return (INetwork *)core->get_networking();
// #else
//   return NULL;
// #endif
// }

//! Returns the interface to the D-BUS facility
DBus *
CoreFactory::get_dbus()
{
#ifdef HAVE_DBUS
  Core *core = Core::get_instance();
  assert(core != NULL);

  return core->get_dbus();
#else
  return NULL;
#endif
}
