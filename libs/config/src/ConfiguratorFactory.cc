// ConfiguratorFactory.cc
//
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
#include "config.h"
#endif

#include <stdlib.h>

#include "IConfigurator.hh"
#include "ConfiguratorFactory.hh"
#include "Configurator.hh"

#ifdef HAVE_GLIB
#include "GlibIniConfigurator.hh"
#endif
#ifdef HAVE_GSETTINGS
#include "GSettingsConfigurator.hh"
#endif
#ifdef HAVE_GDOME
#include "XMLConfigurator.hh"
#endif
#ifdef HAVE_GCONF
#include "GConfConfigurator.hh"
#endif
#ifdef PLATFORM_OS_WIN32
#include "W32Configurator.hh"
#endif
#ifdef PLATFORM_OS_OSX
#include "OSXConfigurator.hh"
#endif
#ifdef HAVE_QT5
#include "QtSettingsConfigurator.hh"
#endif

//! Creates a configurator of the specified type.
IConfigurator::Ptr
ConfiguratorFactory::create(Format fmt)
{
  Configurator *c =  NULL;
  IConfigBackend *b = NULL;

  if (fmt == FormatXml)
    {
#ifdef HAVE_GDOME
      b = new XMLConfigurator();
#endif      
    }

  if (fmt == FormatNative)
    {
#if defined(PLATFORM_OS_WIN32)
      b = new W32Configurator();
#elif defined(PLATFORM_OS_OSX)
      b = new OSXConfigurator();
#elif defined(HAVE_QT5)
      b = new QtSettingsConfigurator();
#elif defined(HAVE_GSETTINGS)
      b = new GSettingsConfigurator();
#elif defined(HAVE_GCONF)
      b = new GConfConfigurator();
#endif
    }
  
  if (fmt == FormatIni)
    {
#ifdef HAVE_GLIB
      b = new GlibIniConfigurator();
#endif
    }

  if (b != NULL)
    {
      c = new Configurator(b);
    }

  return IConfigurator::Ptr(c);
}
