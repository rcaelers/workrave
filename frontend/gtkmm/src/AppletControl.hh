// AppletControl.hh --- Applet window
//
// Copyright (C) 2006, 2007, 2008, 2009, 2011, 2012 Rob Caelers & Raymond Penners
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

#include "preinclude.h"

#include "config/IConfiguratorListener.hh"

#include "IAppletWindow.hh"

using namespace workrave;
using namespace workrave::config;

class AppletControl :
  public IConfiguratorListener
{
public:
  enum AppletType
    {
      APPLET_NONE = -1,
      APPLET_FIRST = 0,
      APPLET_TRAY = APPLET_FIRST,
      APPLET_GNOME,
      APPLET_GENERIC_DBUS,
      APPLET_W32,
      APPLET_OSX,
      APPLET_SIZE
    };

  AppletControl();
  virtual ~AppletControl();

  void init();
  void heartbeat();
  void set_tooltip(std::string& tip);
  bool is_visible();
  IAppletWindow *get_applet_window(AppletType type);

  sigc::signal<void> &signal_visibility_changed();

private:
  //! All known applets
  IAppletWindow *applets[APPLET_SIZE];

  //! Did applet acknowledge visibility?
  IAppletWindow::AppletState applet_state[APPLET_SIZE];

  //!
  bool visible;
  
  //!
  bool enabled;

  //!
  int delayed_show;

  //!
  sigc::signal<void> visibility_changed_signal;
  
private:
  typedef IAppletWindow::AppletState AppletState;

  AppletState activate_applet(AppletType type);
  void deactivate_applet(AppletType type);

  void on_applet_state_changed(AppletType type, IAppletWindow::AppletState state);
  void on_applet_request_activate(AppletType type);
  
  void config_changed_notify(const std::string &key);
  void read_configuration();
  void check_visible();
  void show();
  void show(AppletType type);
  void hide();
  bool is_visible(AppletType type);
};


//! Return the specified applet.
inline IAppletWindow *
AppletControl::get_applet_window(AppletType type)
{
  return applets[type];
}

#endif // APPLETCONTROL_HH
