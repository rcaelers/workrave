// W32AppletMenu.cc --- Menus using W32Applet+
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "W32AppletMenu.hh"

#include <string>

#include "GUI.hh"
#include "Menus.hh"
#include "MainWindow.hh"
#include "W32AppletWindow.hh"

using namespace std;
using namespace workrave;


//! Constructor.
W32AppletMenu::W32AppletMenu(W32AppletWindow *applet_window)
  : applet_window(applet_window)
{
}


//! Destructor.
W32AppletMenu::~W32AppletMenu()
{
}

void
W32AppletMenu::resync(OperationMode mode, UsageMode usage)
{
  TRACE_ENTER_MSG("W32AppletMenu::resync", mode << " " << usage);

  IGUI *gui = GUI::get_instance();
  MainWindow *main_window = gui->get_main_window();
  
  if (applet_window != NULL)
    {
      TRACE_MSG("ok");
#ifndef PLATFORM_OS_WIN32_NATIVE
      HWND cmd_win = (HWND) GDK_WINDOW_HWND(gtk_widget_get_window(main_window->Gtk::Widget::gobj()));
#else
      HWND cmd_win = (HWND) GDK_WINDOW_HWND(((GtkWidget*)main_window->gobj())->window);
#endif

      W32AppletWindow *w32aw = applet_window;
      w32aw->init_menu(cmd_win);

      w32aw->add_menu(_("_Open"), Menus::MENU_COMMAND_OPEN, 0);
      w32aw->add_menu(_("Preferences"), Menus::MENU_COMMAND_PREFERENCES, 0);
      w32aw->add_menu(_("_Rest break"), Menus::MENU_COMMAND_REST_BREAK, 0);
      w32aw->add_menu(_("Exercises"), Menus::MENU_COMMAND_EXERCISES, 0);

      w32aw->add_menu(_("_Normal"), Menus::MENU_COMMAND_MODE_NORMAL,
                      W32AppletWindow::MENU_FLAG_TOGGLE
                      |W32AppletWindow::MENU_FLAG_POPUP
                      |(mode == OperationMode::Normal
                        ? W32AppletWindow::MENU_FLAG_SELECTED
                        : 0));
      w32aw->add_menu(_("Q_uiet"), Menus::MENU_COMMAND_MODE_QUIET,
                      W32AppletWindow::MENU_FLAG_TOGGLE
                      |W32AppletWindow::MENU_FLAG_POPUP
                      |(mode == OperationMode::Quiet
                        ? W32AppletWindow::MENU_FLAG_SELECTED
                        : 0));
      w32aw->add_menu(_("_Suspended"), Menus::MENU_COMMAND_MODE_SUSPENDED,
                      W32AppletWindow::MENU_FLAG_TOGGLE
                      |W32AppletWindow::MENU_FLAG_POPUP
                      |(mode == OperationMode::Suspended
                        ? W32AppletWindow::MENU_FLAG_SELECTED
                        : 0));

      w32aw->add_menu(_("_Mode"), 0, 0);

      w32aw->add_menu(_("Reading mode"), Menus::MENU_COMMAND_MODE_READING,
                      W32AppletWindow::MENU_FLAG_TOGGLE
                      |(usage == UsageMode::Reading
                        ? W32AppletWindow::MENU_FLAG_SELECTED
                        : 0));

      w32aw->add_menu(_("Statistics"), Menus::MENU_COMMAND_STATISTICS, 0);
      w32aw->add_menu(_("About..."), Menus::MENU_COMMAND_ABOUT, 0);
    }

  TRACE_EXIT();
}
