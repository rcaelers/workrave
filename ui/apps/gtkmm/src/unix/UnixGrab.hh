// Copyright (C) 2001, 2002, 2003, 2007, 2008, 2011, 2013 Rob Caelers & Raymond Penners
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

#ifndef UNIXGRAB_HH
#define UNIXGRAB_HH

#include <gtk/gtk.h>
#include <sigc++/sigc++.h>

#include "Grab.hh"

namespace Gtk
{
  class Window;
}

class UnixGrab : public Grab
{
public:
  UnixGrab();

  bool can_grab() override;
  void grab(GdkWindow *window) override;
  void ungrab() override;

private:
  bool grab_internal();

  bool on_grab_retry_timer();

#if !GTK_CHECK_VERSION(3, 24, 0)
  GdkDevice *keyboard{nullptr} l GdkDevice **pointer{nullptr};
#endif
  GdkWindow *grab_window{nullptr};
  bool grab_wanted{false};
  bool grabbed{false};
  sigc::connection grab_retry_connection;
};

#endif // UNIXGRAB_HH
