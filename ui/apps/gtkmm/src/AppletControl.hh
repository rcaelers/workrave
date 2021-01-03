// AppletControl.hh --- Applet window
//
// Copyright (C) 2006, 2007, 2008, 2009, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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

#ifndef APPLETCONTROL_HH
#define APPLETCONTROL_HH

#include <map>
#include <memory>

#include "IAppletWindow.hh"

#include "utils/ScopedConnections.hh"

class AppletControl
{
public:
  enum class AppletType
    {
     Tray,
     GenericDBus,
     Windows,
     MacOS,
    };

  AppletControl() = default;
  ~AppletControl() = default;

  void init();
  void heartbeat();
  void set_tooltip(std::string& tip);
  std::shared_ptr<IAppletWindow> get_applet_window(AppletType type);

  bool is_visible() const;
  sigc::signal<void> &signal_visibility_changed();

private:
  std::map<AppletType, std::shared_ptr<IAppletWindow>> applets;

  bool visible { false };

  sigc::signal<void> visibility_changed_signal;

  scoped_connections connections;

private:
  void on_applet_visibility_changed(AppletType type, bool visible);
  void check_visible();
};

#endif // APPLETCONTROL_HH
