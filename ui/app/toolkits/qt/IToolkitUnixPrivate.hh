// Copyright (C) 2025 Rob Caelers <robc@krandor.nl>
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

#ifndef ITOOLKIT_UNIX_PRIVATE_HH
#define ITOOLKIT_UNIX_PRIVATE_HH

#if defined(HAVE_WAYLAND)
#  include <memory>
#  include "WaylandWindowManager.hh"
#endif

class IToolkitUnixPrivate
{
public:
  virtual ~IToolkitUnixPrivate() = default;

#if defined(HAVE_WAYLAND)
  virtual auto get_wayland_window_manager() -> std::shared_ptr<WaylandWindowManager> = 0;
#endif
};

#endif // ITOOLKIT_UNIX_PRIVATE_HH
