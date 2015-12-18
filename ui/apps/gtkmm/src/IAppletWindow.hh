// IAppletWindow.hh --- Applet window
//
// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#ifndef IAPPLETWINDOW_HH
#define IAPPLETWINDOW_HH

#include <string>
#include <sigc++/sigc++.h>

class IAppletWindow
{
public:
  enum AppletState
    {
      APPLET_STATE_DISABLED,
      APPLET_STATE_PENDING,
      APPLET_STATE_VISIBLE,
      APPLET_STATE_ACTIVE,
    };

  virtual ~IAppletWindow() {}

  virtual sigc::signal<void, AppletState> &signal_state_changed() = 0;
  virtual sigc::signal<void> &signal_request_activate() = 0;

  virtual AppletState activate_applet() = 0;
  virtual void deactivate_applet() = 0;
  virtual void init_applet() = 0;
  virtual void update_applet() = 0;
  virtual void set_applet_tooltip(const std::string &tip) = 0;
};

#endif // IAPPLETWINDOW_HH
