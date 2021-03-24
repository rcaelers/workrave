// CoreFactory.cc
//
// Copyright (C) 2001, 2002, 2003, 2006, 2007 Rob Caelers & Raymond Penners
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
#  include "config.h"
#endif

#include <cassert>

#include "core/CoreFactory.hh"
#include "config/IConfigurator.hh"
#include "Core.hh"

ICore *
CoreFactory::get_core()
{
  return Core::get_instance();
}

IConfigurator *
CoreFactory::get_configurator()
{
  Core *core = Core::get_instance();
  assert(core != nullptr);

  return core->get_configurator();
}

#ifdef HAVE_DBUS
workrave::dbus::IDBus::Ptr
CoreFactory::get_dbus()
{
  Core *core = Core::get_instance();
  assert(core != nullptr);

  return core->get_dbus();
}
#endif
