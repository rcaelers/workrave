// MainWindow.hh --- Main info Window
//
// Copyright (C) 2001, 2002 Rob Caelers & Raymond Penners
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

#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#define NOMINMAX

#include <stdio.h>

#include "preinclude.h"

class GUI;
class ControlInterface;
class TimeBar;

#include <gtkmm.h>
#ifdef WIN32
#include <windows.h>
#endif

#include "ConfiguratorListener.hh"

using namespace std;

class MainWindow :
  public Gtk::Window,
  public ConfiguratorListener
{
public:  
  MainWindow(GUI *gui, ControlInterface *controller);
  ~MainWindow();

  void update();
  static bool get_always_on_top();
  static void set_always_on_top(bool b);


private:
  //! The controller that maintains the data and control over the breaks
  ControlInterface *core_control;

  //! Interface to the GUI.
  GUI *gui;
  
  //! Connection to the delete_event signal.
  SigC::Connection delete_connection;

  //! Connection to the timeout timer.
  SigC::Connection timer_connection;

  //! Table containing all timer information
  Gtk::Table *timers_box;

  //! Array of timer name labels
  Gtk::Widget **timer_names;

  //! Araay of timer value widgets.
  TimeBar **timer_times;

  //! The popup menu.
  Gtk::Menu *popup_menu;

  //! The popup mode menu items
  Gtk::RadioMenuItem *popup_mode_menus[3];
  
  //! Is the monitoring function suspended?
  bool monitor_suspended;
  
private:
  //
  void init();
  void setup();
  Gtk::Menu *create_menu(Gtk::RadioMenuItem *mode_menus[3]);
  void config_changed_notify(string key);

  // Events.
  bool on_delete_event(GdkEventAny*);
  bool on_button_event(GdkEventButton *event);
  void on_menu_about();
  void on_menu_quit();
  void on_menu_restbreak_now();
  void on_menu_preferences();
  void on_menu_statistics();
  void on_menu_normal();
  void on_menu_suspend();
  void on_menu_quiet();
#ifndef NDEBUG
  void on_test_me();
#endif

#ifdef WIN32
public:
  static void win32_set_start_in_tray(bool b);
  static bool win32_get_start_in_tray();

  static const string CFG_KEY_MAIN_WINDOW_START_IN_TRAY;
  static const string CFG_KEY_MAIN_WINDOW_X;
  static const string CFG_KEY_MAIN_WINDOW_Y;
  
private:
  void win32_sync_menu(int mode);
  void win32_show(bool b);
  void win32_init();
  void win32_exit();
  void win32_on_tray_open();
  void win32_remember_position();
  static void win32_get_start_position(int &x, int &y);
  static void win32_set_start_position(int x, int y);

  static LRESULT CALLBACK win32_window_proc(HWND hwnd, UINT uMsg,
                                            WPARAM wParam, LPARAM lParam);

  Gtk::Menu *win32_tray_menu;
  Gtk::RadioMenuItem *win32_tray_mode_menus[3];
  HWND win32_main_hwnd;
  NOTIFYICONDATA win32_tray_icon;
#endif
};

#endif // MAINWINDOW_HH
