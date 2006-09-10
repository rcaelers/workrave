// Menus.hh --- Main info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef MENUS_HH
#define MENUS_HH

#include "config.h"

#ifdef HAVE_GTKMM24
#include <sigc++/compatibility.h>
#endif

#ifdef HAVE_GNOME
#include <gnome.h>
#endif

#include "CoreInterface.hh"

class GUI;
class TimeBar;
class NetworkLogDialog;
class NetworkJoinDialog;
class StatisticsDialog;
class PreferencesDialog;
class MainWindow;
class AppletWindow;
class ExercisesDialog;
class TimerBoxAppletView;

#include <gtkmm/checkmenuitem.h>


#define MAX_CHECKMENUS 4

class Menus :
  public SigC::Object
{
public:  
  Menus();
  ~Menus();

  Gtk::Menu *create_menu(Gtk::CheckMenuItem *check_menus[4]);
  Gtk::Menu *create_main_window_menu();
  Gtk::Menu *create_tray_menu();
  
  static Menus *get_instance();

  void set_main_window(MainWindow *main);
#if defined(HAVE_GNOME)
  void set_applet_window(AppletWindow *applet);
#elif defined(WIN32)
  void set_applet_window(TimerBoxAppletView *applet);
  void on_applet_command(short cmd);
#endif
  void resync_applet();
  
private:
  void sync_mode_menu(int mode);
  void sync_tray_menu(bool active);
  void set_operation_mode(OperationMode m);
  
#ifdef HAVE_DISTRIBUTION
  void on_menu_network_log_main_window();
  void on_menu_network_log_tray();
  void on_network_log_response(int response);
  void on_network_join_response(int response);
#endif
  void on_statistics_response(int response);
  void on_preferences_response(int response);
#ifdef HAVE_EXERCISES
  void on_exercises_response(int response);
#endif  
  void on_menu_normal_menu(Gtk::CheckMenuItem *);
  void on_menu_suspend_menu(Gtk::CheckMenuItem *);
  void on_menu_quiet_menu(Gtk::CheckMenuItem *);
  
public:  
  // Menu actions.
  void on_menu_open_main_window();
  void on_menu_restbreak_now();
  void on_menu_about();
  void on_menu_quit();
  void on_menu_preferences();
#ifdef HAVE_EXERCISES  
  void on_menu_exercises();
#endif
  void on_menu_statistics();
  void on_menu_normal();
  void on_menu_suspend();
  void on_menu_quiet();
#ifndef NDEBUG
  void on_test_me();
#endif
  void on_menu_network_join();
  void on_menu_network_leave();
  void on_menu_network_reconnect();
  void on_menu_network_log(bool active);

protected:
  //! Interface to the GUI.
  GUI *gui;

private:
  //! The one and only instance
  static Menus *instance;
  
#if defined(HAVE_GNOME)
  //! The applet windows
  AppletWindow *applet_window;
#elif defined(WIN32)
  //! The applet windows
  TimerBoxAppletView *applet_window;
#endif

#ifdef HAVE_DISTRIBUTION
  NetworkLogDialog *network_log_dialog;
  NetworkJoinDialog *network_join_dialog;
#endif

  // The Statistics dialog.
  StatisticsDialog *statistics_dialog;
  
  // The Statistics dialog.
  PreferencesDialog *preferences_dialog;

#ifdef HAVE_EXERCISES
  // The exercises dialog.
  ExercisesDialog *exercises_dialog;
#endif
  //! The main window.
  MainWindow *main_window;

  //! The system tray popup menu items.
  Gtk::CheckMenuItem *tray_check_menus[MAX_CHECKMENUS];

  //! The popup mode menu items
  Gtk::CheckMenuItem *main_window_check_menus[MAX_CHECKMENUS];
};


inline void
Menus::set_main_window(MainWindow *main)
{
  main_window = main;
}


#if defined(HAVE_GNOME)
inline void
Menus::set_applet_window(AppletWindow *applet)
{
  applet_window = applet;
}
#elif defined(WIN32)
inline void
Menus::set_applet_window(TimerBoxAppletView *applet)
{
  applet_window = applet;
}
#endif


inline Menus *
Menus::get_instance()
{
  if (instance == NULL)
    {
      instance = new Menus();
    }
       
  return instance;
}

#endif // MENUS_HH
