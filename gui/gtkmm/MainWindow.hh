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

class MainWindow :
  public Gtk::Window,
  public ConfiguratorListener
{
public:  
  MainWindow(GUI *gui, ControlInterface *controller);
  ~MainWindow();

  void update();

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

  //! Normal menu item.
  Gtk::CheckMenuItem *normal_menu_item;

  //! Suspend menu item.
  Gtk::CheckMenuItem *suspend_menu_item;

  //! Quiet menu item.
  Gtk::CheckMenuItem *quiet_menu_item;
  
  //! Is the monitoring function suspended?
  bool monitor_suspended;
  
  //! MainWindow always on-top ?
  bool always_on_top;
  
private:
  //
  void init();
  void setup();
  void create_menu();
  void load_config();
  void store_config();
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
private:
  void win32_show(bool b);
  void win32_init();
  void win32_exit();
  static LRESULT CALLBACK win32_window_proc(HWND hwnd, UINT uMsg,
                                            WPARAM wParam, LPARAM lParam);
  HWND win32_main_hwnd;
  NOTIFYICONDATA win32_tray_icon;
#endif
};

#endif // MAINWINDOW_HH
