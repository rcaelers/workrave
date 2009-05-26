// ConfiguratorFactory.cc
//
// Copyright (C) 2007, 2008 Rob Caelers
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

#include "Configurator.hh"
#include "ConfiguratorFactory.hh"

#ifdef HAVE_GLIB
#include "GlibIniConfigurator.hh"
#endif
#ifdef HAVE_QT
#include "QtIniConfigurator.hh"
#include "QtNativeConfigurator.hh"
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

//! Creates a configurator of the specified type.
Configurator *
ConfiguratorFactory::create(Format fmt)
{
  Configurator *c =  NULL;
  IConfigBackend *b = NULL;

#ifdef HAVE_GDOME
  if (fmt == FormatXml)
    {
      b = new XMLConfigurator();
    }
  else
#endif

#ifdef HAVE_GCONF
  if (fmt == FormatNative)
    {
      b = new GConfConfigurator();
    }
  else
#endif

#ifdef PLATFORM_OS_WIN32
  if (fmt == FormatNative)
    {
      b = new W32Configurator();
    }
  else
#endif

#ifdef PLATFORM_OS_OSX
  if (fmt == FormatNative)
    {
      b = new OSXConfigurator();
    }
  else
#endif
    
#ifdef HAVE_QT
  if (fmt == FormatNative)
    {
      b = new QtNativeConfigurator();
    }
  else
#endif

  if (fmt == FormatIni)
    {
#ifdef HAVE_GLIB
      b = new GlibIniConfigurator();
#elif defined(HAVE_QT)
      b = new QtIniConfigurator();
#else
#error Not ported
#endif
    }
  else
    {
      exit(1);
    }

  if (b != NULL)
    {
      c = new Configurator(b);
    }

  return c;
}
