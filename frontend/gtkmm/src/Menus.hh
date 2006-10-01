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

#include "ICore.hh"

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

class Menus :
  public SigC::Object
{
public:  
  Menus();
  ~Menus();

  //! Menus items to be synced.
  enum MenuSyncs
    {
      // Note: First 3 elements MUST be same as OperatingMode!!!
      MENUSYNC_MODE_NORMAL,
      MENUSYNC_MODE_SUSPENDED,
      MENUSYNC_MODE_QUIET,
      MENUSYNC_SHOW_LOG,
      MENUSYNC_SIZEOF
    };

  //! Menus items to be synced.
  enum MenuKind
    {
      MENU_MAINWINDOW,
      MENU_APPLET,
      MENU_SIZEOF
    };

  void create_menu(MenuKind kind);
  
  void popup(const MenuKind kind,
             const guint button,
             const guint activate_time);

  static Menus *get_instance();

  void set_main_window(MainWindow *main);
#if defined(HAVE_GNOME) || defined(WIN32)
  void set_applet_window(AppletWindow *applet);
#endif
#if defined(WIN32)
  void on_applet_command(short cmd);
#endif
  void resync_applet();
  
private:
  Gtk::Menu *create_menu(MenuKind kind, Gtk::CheckMenuItem *check_menus[MENUSYNC_SIZEOF]);

#ifdef WIN32
  void win32_popup_hack_connect(Gtk::Menu *menu);
  static gboolean win32_popup_hack_hide(gpointer data);
  static gboolean win32_popup_hack_leave_enter(GtkWidget *menu,
                                               GdkEventCrossing *event,
                                               void *data);
#endif  
  

  void sync_mode_menu(int mode);
  void sync_log_menu(bool active);
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

  //! 
  Gtk::Menu *menus[MENU_SIZEOF];
  
#if defined(HAVE_GNOME) || defined(WIN32)
  //! The applet windows
  AppletWindow *applet_window;
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
  Gtk::CheckMenuItem *sync_menus[MENU_SIZEOF][MENUSYNC_SIZEOF];
};


inline void
Menus::set_main_window(MainWindow *main)
{
  main_window = main;
}


#if defined(HAVE_GNOME) || defined(WIN32)
inline Menus *
Menus::get_instance()
{
  assert(instance != 0);
  return instance;
}
#endif

#endif // MENUS_HH
