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

#include "ToolkitPlatformWindows.hh"

class ToolkitPlatformWindows::Pimpl
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

ToolkitPlatformWindows::ToolkitPlatformWindows()
{
  pimpl = std::make_unique<Pimpl>();
}

ToolkitPlatformWindows::~ToolkitPlatformWindows()
{
}

QPixmap
ToolkitPlatformWindows::get_desktop_image()
{
  return pimpl->get_desktop_image();
}

void
ToolkitPlatformWindows::foreground()
{
  pimpl->foreground();
}

void
ToolkitPlatformWindows::restore_foreground()
{
  pimpl->restore_foreground();
}

void
ToolkitPlatformWindows::lock()
{
  pimpl->lock();
}

void
ToolkitPlatformWindows::unlock()
{
  pimpl->unlock();
}

QPixmap
ToolkitPlatformWindows::Pimpl::get_desktop_image()
{
  QPixmap pixmap;
  return pixmap;
}

void
ToolkitPlatformWindows::Pimpl::foreground()
{
}

void
ToolkitPlatformWindows::Pimpl::restore_foreground()
{
}

void
ToolkitPlatformWindows::Pimpl::lock()
{
}

void
ToolkitPlatformWindows::Pimpl::unlock()
{
}
