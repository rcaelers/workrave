// W32TrayMenu.hh --- Menu using W32Tray+
//
// Copyright (C) 2001 - 2009, 2012 Rob Caelers & Raymond Penners
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

#ifndef W32TRAYMENU_HH
#define W32TRAYMENU_HH

#include "config.h"

#include <string>

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <gtkmm/actiongroup.h>
#include <gtkmm/iconfactory.h>
#include <gtkmm/radioaction.h>
#include <gtkmm/uimanager.h>

#include "MainGtkMenu.hh"

class W32TrayMenu : public MainGtkMenu
{
public:
  W32TrayMenu();
  virtual ~W32TrayMenu();

  virtual void post_init();
  virtual void popup(const guint button, const guint activate_time);

private:
  void win32_popup_hack_connect(Gtk::Widget *menu);
  static gboolean win32_popup_hack_hide(gpointer data);
  static gboolean win32_popup_hack_leave_enter(GtkWidget *menu,
                                               GdkEventCrossing *event,
                                               void *data);
};

#endif // W32TRAYMENU_HH
