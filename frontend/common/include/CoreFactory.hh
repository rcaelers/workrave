// CoreFactory.hh --- The main access point to the Core
//
// Copyright (C) 2001 - 2007, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef COREFACTORY_HH
#define COREFACTORY_HH

#include "ICore.hh"
#include "config/IConfigurator.hh"
#include "dbus/IDBus.hh"

//! Main access points to the Core.
class CoreFactory
{
public:
  //! Returns the interface to the core.
  static workrave::ICore::Ptr get_core();
  
  //! Returns the interface to the core's configurator.
  static workrave::config::IConfigurator::Ptr get_configurator();
  
  //! Returns the interface to the DBUS facility.
  static workrave::dbus::IDBus::Ptr get_dbus();
  
  //! The one and only core instance
  static workrave::ICore::Ptr core;
};

#endif // COREFACTORY_HH
