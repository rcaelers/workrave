// CoreFactory.cc
//
// Copyright (C) 2001, 2002, 2003, 2006, 2007 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

static const char rcsid[] = "$Id$";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>

#include "CoreFactory.hh"
#include "Configurator.hh"
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
