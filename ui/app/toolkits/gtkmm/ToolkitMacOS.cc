// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#include "ToolkitMacOS.hh"

#include "utils/Paths.hh"
#include "debug.hh"

// #if defined(PLATFORM_OS_MACOS)
// #  include "MacOSAppletWindow.hh"
// #endif

using namespace workrave::utils;

ToolkitMacOS::ToolkitMacOS(int argc, char **argv)
  : Toolkit(argc, argv)
{
  locker = std::make_shared<MacOSLocker>();
  setup_environment();
}

void
ToolkitMacOS::init(std::shared_ptr<IApplicationContext> app)
{
  Toolkit::init(app);
}

auto
ToolkitMacOS::get_locker() -> std::shared_ptr<Locker>
{
  return locker;
}

void
ToolkitMacOS::setup_environment()
{
  TRACE_ENTRY();
  std::filesystem::path resources_dir = Paths::get_application_directory() / "Resources";

  auto etc_dir = resources_dir / "etc";
  auto bin_dir = resources_dir / "bin";
  auto lib_dir = resources_dir / "lib";
  auto share_dir = resources_dir / "share";

  Glib::setenv("DYLD_LIBRARY_PATH", lib_dir.string() + ":" + (lib_dir / "gdk-pixbuf-2.0" / "2.10.0" / "loaders").string());
  Glib::setenv("FONTCONFIG_PATH", etc_dir / "fonts");
  Glib::setenv("GDK_PIXBUF_MODULE_FILE", lib_dir / "gdk-pixbuf-2.0" / "2.10.0" / "loaders.cache");
  Glib::setenv("GIO_MODULE_DIR", lib_dir / "gio" / "modules");
  Glib::setenv("GTK_DATA_PREFIX", resources_dir);
  Glib::setenv("GTK_EXE_PREFIX", resources_dir);
  Glib::setenv("GTK_PATH", resources_dir);
  Glib::setenv("PATH", bin_dir.string() + ":" + Glib::getenv("PATH"));
  Glib::setenv("XDG_CONFIG_DIRS", etc_dir / "xdg");
  Glib::setenv("XDG_DATA_DIRS", share_dir);
}
