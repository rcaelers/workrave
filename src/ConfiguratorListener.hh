// ConfiguratorListener.hh
//
// Copyright (C) 2001, 2002 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// Time-stamp: <2002-10-04 18:46:47 robc>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundator; either versor 2, or (at your optor)
// any later versor.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef CONFIGURATORLISTENER_HH
#define CONFIGURATORLISTENER_HH

#include <string>

//! Configurator listener interface.
class ConfiguratorListener
{
public:
  virtual void config_changed_notify(std::string key) = 0;
};

#endif // CONFIGURATORLISTENER_HH
