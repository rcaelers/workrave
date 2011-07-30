// GnomeAppletMenu.cc --- Menus using GnomeApplet+
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "GnomeAppletMenu.hh"

#include <string>

#include <gdkmm/pixbuf.h>
#include <gtkmm/action.h>
#include <gtkmm/iconset.h>
#include <gtkmm/iconsource.h>
#include <gtkmm/menu.h>
#include <gtkmm/stock.h>

#include "GnomeAppletWindow.hh"
#include "Menus.hh"
#include "Util.hh"

using namespace std;


//! Constructor.
GnomeAppletMenu::GnomeAppletMenu(GnomeAppletWindow *applet_window)
  :applet_window(applet_window)
{
}


//! Destructor.
GnomeAppletMenu::~GnomeAppletMenu()
{
}


void
GnomeAppletMenu::init()
{
}


void
GnomeAppletMenu::add_accel(Gtk::Window &)
{
}


void
GnomeAppletMenu::popup(const guint button, const guint activate_time)
{
  (void) button;
  (void) activate_time;
}


void
GnomeAppletMenu::resync(OperationMode mode, UsageMode usage, bool show_log)
{
  if (applet_window != NULL)
    {
      switch (mode)
        {
        case OPERATION_MODE_NORMAL:
          applet_window->set_menu_active(GnomeAppletWindow::MENUSYNC_MODE_NORMAL, true);
          break;

        case OPERATION_MODE_SUSPENDED:
          applet_window->set_menu_active(GnomeAppletWindow::MENUSYNC_MODE_SUSPENDED, true);
          break;

        case OPERATION_MODE_QUIET:
          applet_window->set_menu_active(GnomeAppletWindow::MENUSYNC_MODE_QUIET, true);
          break;

        default:
          break;
        }

      applet_window->set_menu_active(GnomeAppletWindow::MENUSYNC_SHOW_LOG, show_log);
      applet_window->set_menu_active(GnomeAppletWindow::MENUSYNC_MODE_READING, usage == USAGE_MODE_READING);
    }
}
