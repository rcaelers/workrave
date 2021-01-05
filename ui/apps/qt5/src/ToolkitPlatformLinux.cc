// Copyright (C) 2007 - 2013 Rob Caelers & Raymond Penners
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

#include "ToolkitPlatformLinux.hh"

class ToolkitPlatformLinux::Pimpl
{
public:
  Pimpl() = default;

  QPixmap get_desktop_image();
  void foreground();
  void restore_foreground();
  void lock();
  void unlock();

private:
};

ToolkitPlatformLinux::ToolkitPlatformLinux()
{
  pimpl = std::make_unique<Pimpl>();
}

ToolkitPlatformLinux::~ToolkitPlatformLinux() {}

QPixmap
ToolkitPlatformLinux::get_desktop_image()
{
  return pimpl->get_desktop_image();
}

void
ToolkitPlatformLinux::foreground()
{
  pimpl->foreground();
}

void
ToolkitPlatformLinux::restore_foreground()
{
  pimpl->restore_foreground();
}

void
ToolkitPlatformLinux::lock()
{
  pimpl->lock();
}

void
ToolkitPlatformLinux::unlock()
{
  pimpl->unlock();
}

QPixmap
ToolkitPlatformLinux::Pimpl::get_desktop_image()
{
  QPixmap pixmap;
  return pixmap;
}

void
ToolkitPlatformLinux::Pimpl::foreground()
{
}

void
ToolkitPlatformLinux::Pimpl::restore_foreground()
{
}

void
ToolkitPlatformLinux::Pimpl::lock()
{
}

void
ToolkitPlatformLinux::Pimpl::unlock()
{
}
