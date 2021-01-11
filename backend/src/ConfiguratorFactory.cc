// ConfiguratorFactory.cc
//
// Copyright (C) 2007, 2008, 2011 Rob Caelers
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

#include "Configurator.hh"
#include "ConfiguratorFactory.hh"

#ifdef HAVE_GLIB
#  include "GlibIniConfigurator.hh"
#endif
#ifdef HAVE_GSETTINGS
#  include "GSettingsConfigurator.hh"
#endif
#ifdef HAVE_GDOME
#  include "XMLConfigurator.hh"
#endif
#ifdef HAVE_GCONF
#  include "GConfConfigurator.hh"
#endif
#ifdef PLATFORM_OS_WINDOWS
#  include "W32Configurator.hh"
#endif
#ifdef PLATFORM_OS_MACOS
#  include "MacOSConfigurator.hh"
#endif

//! Creates a configurator of the specified type.
Configurator *
ConfiguratorFactory::create(Format fmt)
{
  Configurator *c = nullptr;
  IConfigBackend *b = nullptr;

#ifdef HAVE_GDOME
  if (fmt == FormatXml)
    {
      b = new XMLConfigurator();
    }
  else
#endif

#if HAVE_GSETTINGS
    if (fmt == FormatNative)
    {
      b = new GSettingsConfigurator();
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

#ifdef PLATFORM_OS_WINDOWS
    if (fmt == FormatNative)
    {
      b = new W32Configurator();
    }
  else
#endif

#ifdef PLATFORM_OS_MACOS
    if (fmt == FormatNative)
    {
      b = new MacOSConfigurator();
    }
  else
#endif

    if (fmt == FormatIni)
    {
#ifdef HAVE_GLIB
      b = new GlibIniConfigurator();
#else
#  error Not ported
#endif
    }

  if (b != nullptr)
    {
      c = new Configurator(b);
    }

  return c;
}
