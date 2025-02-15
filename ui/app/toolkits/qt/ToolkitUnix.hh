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

#ifndef TOOLKIT_UNIX_HH
#define TOOLKIT_UNIX_HH

#include "IToolkitUnixPrivate.hh"
#include "Toolkit.hh"

#include "UnixLocker.hh"
#if defined(HAVE_WAYLAND)
#  include "WaylandWindowManager.hh"
#endif

class ToolkitUnix
  : public Toolkit
  , public IToolkitUnixPrivate
{
public:
  ToolkitUnix(int argc, char **argv);
  ~ToolkitUnix() override = default;

  // IToolkit
  void preinit(std::shared_ptr<workrave::config::IConfigurator> config) override;
  void init(std::shared_ptr<IApplicationContext> app) override;
  std::shared_ptr<Locker> get_locker() override;

  void show_notification(const std::string &id,
                         const std::string &title,
                         const std::string &balloon,
                         std::function<void()> func) override;
  auto get_desktop_image() -> QPixmap override;

// IToolkitUnixPrivate
#if defined(HAVE_WAYLAND)
  auto get_wayland_window_manager() -> std::shared_ptr<WaylandWindowManager> override;
#endif

private:
  std::shared_ptr<UnixLocker> locker;
#if defined(HAVE_WAYLAND)
  std::shared_ptr<WaylandWindowManager> wayland_window_manager;
#endif
};

#endif // TOOLKIT_UNIX_HH
