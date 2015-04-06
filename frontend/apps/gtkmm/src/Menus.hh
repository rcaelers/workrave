// Menus.hh --- Main info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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

#ifndef MENUS_HH
#define MENUS_HH

#include <gtkmm.h>

#include "ICore.hh"
#include "SoundTheme.hh"

class IGUI;
class StatisticsDialog;
class PreferencesDialog;
class AppletControl;
class ExercisesDialog;
class IMenu;

namespace Gtk
{
  class Menu;
}

using namespace workrave;

class Menus :
  public sigc::trackable
{
public:
  explicit Menus(SoundTheme::Ptr sound_theme);
  ~Menus();

  //! Menus items to be synced.
  enum MenuKind
    {
      MENU_NONE,
      MENU_MAINWINDOW,
      MENU_MAINAPPLET,
      MENU_APPLET_GNOME,
      MENU_APPLET_W32,
      MENU_APPLET_INDICATOR,
      MENU_APPLET_GENERICDBUS,
      MENU_SIZEOF,
    };

  void init(AppletControl *applet_control);
  void applet_command(short cmd);
  void resync();
  void locale_changed();
  void popup(const MenuKind kind,
             const guint button,
             const guint activate_time);

private:
  void set_usage_mode(workrave::UsageMode m);
  void on_menu_response(int response);
  void on_about_response(int response);

  void on_statistics_response(int response);
  void on_preferences_response(int response);
  void on_exercises_response(int response);

public:
  // Menu actions.
  void on_menu_open_main_window();
  void on_menu_restbreak_now();
  void on_menu_about();
  void on_menu_quit();
  void on_menu_preferences();
  void on_menu_exercises();
  void on_menu_statistics();
  void on_menu_normal();
  void on_menu_suspend();
  void on_menu_quiet();
  void on_menu_reading(bool reading);
  void on_menu_network_join();
  void on_menu_network_leave();
  void on_menu_network_reconnect();
  void on_menu_network_log(bool show);
  void on_set_operation_mode(workrave::OperationMode m);

#ifdef PLATFORM_OS_WIN32
  void on_about_link_activate(Gtk::AboutDialog &about, const Glib::ustring &link);
#endif

private:
  //! Interface to the GUI.
  IGUI *gui;

  // The Statistics dialog.
  StatisticsDialog *statistics_dialog;

  // The Statistics dialog.
  PreferencesDialog *preferences_dialog;

  // The exercises dialog.
  ExercisesDialog *exercises_dialog;

  //! Different kind of menus
  IMenu *menus[MENU_SIZEOF];

  Gtk::AboutDialog *about;

  SoundTheme::Ptr sound_theme;
};

#endif // MENUS_HH
