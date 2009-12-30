// W32AppletMenu.hh --- Menu using W32Applet+
//
// Copyright (C) 2001 - 2009 Rob Caelers & Raymond Penners
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

#ifndef W32APPLETMENU_HH
#define W32APPLETMENU_HH

#include "config.h"

#include <string>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/iconfactory.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/uimanager.h>

#include "Menu.hh"

class MainWindow;
class W32AppletWindow;

class W32AppletMenu : public Menu
{
public:
  W32AppletMenu(MainWindow *main_window, W32AppletWindow *applet_window);
  virtual ~W32AppletMenu();

  virtual void init();
  virtual void add_accel(Gtk::Window &window);
  virtual void popup(const guint button, const guint activate_time);
  virtual void resync(workrave::OperationMode mode, workrave::UsageMode usage, bool show_log);
  
private:
  MainWindow *main_window;
  W32AppletWindow *applet_window;
};

#endif // W32APPLETMENU_HH
