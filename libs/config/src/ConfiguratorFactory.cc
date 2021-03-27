// Copyright (C) 2007, 2008, 2011, 2012, 2013 Rob Caelers
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

#include <cstdlib>

#include "IConfigurator.hh"
#include "ConfiguratorFactory.hh"
#include "Configurator.hh"

#include "IniConfigurator.hh"
#include "XmlConfigurator.hh"
#ifdef HAVE_GSETTINGS
#  include "GSettingsConfigurator.hh"
#endif
#ifdef PLATFORM_OS_WINDOWS
#  include "W32Configurator.hh"
#endif
#ifdef PLATFORM_OS_MACOS
#  include "MacOSConfigurator.hh"
#endif
#ifdef HAVE_QT5
#  include "QtSettingsConfigurator.hh"
#endif

using namespace workrave::config;

//! Creates a configurator of the specified type.
IConfigurator::Ptr
ConfiguratorFactory::create(ConfigFileFormat fmt)
{
  Configurator *c = nullptr;
  IConfigBackend *b = nullptr;

  if (fmt == ConfigFileFormat::Native)
    {
#if defined(PLATFORM_OS_WINDOWS)
      b = new W32Configurator();
#elif defined(PLATFORM_OS_MACOS)
      b = new MacOSConfigurator();
#elif defined(HAVE_QT5)
      b = new QtSettingsConfigurator();
#elif defined(HAVE_GSETTINGS)
      b = new GSettingsConfigurator();
#endif
    }

  if (fmt == ConfigFileFormat::Xml)
    {
      b = new XmlConfigurator();
    }

  if (fmt == ConfigFileFormat::Ini)
    {
      b = new IniConfigurator();
    }

  if (b != nullptr)
    {
      c = new Configurator(b);
    }

  return IConfigurator::Ptr(c);
}
