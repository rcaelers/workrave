// MainWindow.cc --- Main info Window
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

static const char rcsid[] = "$Id$";

// TODO: only when needed.
#define NOMINMAX

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <unistd.h>
#include <iostream>

#ifdef HAVE_GNOME
#include <gnome.h>
#endif
#include "MainWindow.hh"
#include "PreferencesDialog.hh"
#include "StatisticsDialog.hh"
#include "WindowHints.hh"
#include "TimeBar.hh"
#include "GUI.hh"
#include "GUIControl.hh"
#include "Util.hh"
#include "Text.hh"

#include "Configurator.hh"
#include "TimerInterface.hh"
#include "ControlInterface.hh"
#include "ActivityMonitorInterface.hh"
#include "Statistics.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#include "NetworkJoinDialog.hh"
#include "NetworkLogDialog.hh"
#endif

#ifdef WIN32
#include <gdk/gdkwin32.h>
#endif

#include "Menus.hh"

const string MainWindow::CFG_KEY_MAIN_WINDOW_START_IN_TRAY
= "gui/main_window/start_in_tray";

#ifdef WIN32
const char *WIN32_MAIN_CLASS_NAME = "Workrave";
const UINT MYWM_TRAY_MESSAGE = WM_USER +0x100;

const string MainWindow::CFG_KEY_MAIN_WINDOW_X
= "gui/main_window/x";
const string MainWindow::CFG_KEY_MAIN_WINDOW_Y
= "gui/main_window/y";

#endif

//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
MainWindow::MainWindow(GUI *g, ControlInterface *c) :
  TimerWindow(g, c),
  timers_box(NULL),
  monitor_suspended(false)
{
  init();
}


//! Destructor.
MainWindow::~MainWindow()
{
  TRACE_ENTER("MainWindow::~MainWindow");

#ifdef WIN32
  win32_exit();
#endif
  
  TRACE_EXIT();
}


//! Initializes the main window.
void
MainWindow::init()
{
  TRACE_ENTER("MainWindow::init");

  set_title("Workrave");
  set_border_width(2);
  
  Configurator *config = gui->get_configurator();
  if (config != NULL)
    {
      config->add_listener(GUIControl::CFG_KEY_MAIN_WINDOW, this);
    }
  

  Menus *menus = Menus::get_instance();
  menus->set_main_window(this);
  popup_menu = menus->create_main_window_menu();
  
  timers_box = manage(new Gtk::Table(GUIControl::BREAK_ID_SIZEOF, 2, false));
  timers_box->set_spacings(2);
  
  init_widgets();
  
  for (int count = 0; count < GUIControl::BREAK_ID_SIZEOF; count++)
    {
      timers_box->attach(*timer_names[count], 0, 1, count, count + 1, Gtk::FILL);
      timers_box->attach(*timer_times[count], 1, 2, count, count + 1, Gtk::EXPAND | Gtk::FILL);
    }
  
  add(*timers_box);

  // Necessary for popup menu 
  set_events(get_events() | Gdk::BUTTON_PRESS_MASK);

  realize_if_needed();

  set_resizable(false);
  setup();
  stick();

  Glib::RefPtr<Gdk::Window> window = get_window();
#ifdef WIN32
  window->set_functions(Gdk::FUNC_CLOSE|Gdk::FUNC_MOVE);
#endif
  // WindowHints::set_tool_window(Gtk::Widget::gobj(), true);
  WindowHints::set_skip_winlist(Gtk::Widget::gobj(), true);

#ifdef WIN32
  win32_init();
  int x, y;
  win32_get_start_position(x, y);
  set_gravity(Gdk::GRAVITY_STATIC); 
  set_position(Gtk::WIN_POS_NONE);
  if (get_start_in_tray())
    {
      move(-1024, 0);
      show_all();
      win32_show(false);
      move(x, y);
    }
  else
    {
      move(x, y);
      show_all();
    }
#else
  show_all();
  if (get_start_in_tray())
    {
      iconify();
    }
#endif
  setup();
  TRACE_EXIT();
}




//! Setup configuration settings.
void
MainWindow::setup()
{
  TRACE_ENTER("MainWindow::setup");

  bool always_on_top = get_always_on_top();
  WindowHints::set_always_on_top(Gtk::Widget::gobj(), always_on_top);
  
  if (always_on_top)
    {
      raise();
    }

  TRACE_EXIT()
}


//! Updates the main window.
void
MainWindow::update()
{
  update_widgets();
}


//! User has closed the main window.
bool
MainWindow::on_delete_event(GdkEventAny *)
{
  TRACE_ENTER("MainWindow::on_delete_event");

#ifdef WIN32
  win32_show(false);
#else
  gui->terminate();
#endif
  
  TRACE_EXIT();
  return true;
}



//! Users pressed some mouse button in the main window.
bool
MainWindow::on_button_press_event(GdkEventButton *event)
{
  TRACE_ENTER("MainWindow::on_button_press_event");
  bool ret = false;

  if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
    {
      popup_menu->popup(event->button, event->time);
      ret = true;
    }

  TRACE_EXIT();
  return ret;
}


void
MainWindow::config_changed_notify(string key)
{
  TRACE_ENTER("MainWindow::config_changed_notify");
  (void) key;
  setup();
  TRACE_EXIT();
}

bool
MainWindow::get_always_on_top() 
{
  bool b;
  bool rc;
  b = GUIControl::get_instance()->get_configurator()
    ->get_value(GUIControl::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, &rc);
  if (! b)
    {
      rc = false;
    }
  return rc;
}


void
MainWindow::set_always_on_top(bool b)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(GUIControl::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, b);
}


bool
MainWindow::get_start_in_tray() 
{
  bool b;
  bool rc;
  b = GUIControl::get_instance()->get_configurator()
    ->get_value(CFG_KEY_MAIN_WINDOW_START_IN_TRAY, &rc);
  if (! b)
    {
      rc = false;
    }
  return rc;
}


void
MainWindow::set_start_in_tray(bool b)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_MAIN_WINDOW_START_IN_TRAY, b);
}


#ifdef WIN32
void
MainWindow::win32_show(bool b)
{
  // Gtk's hide() seems to quit the program.
  GtkWidget *window = Gtk::Widget::gobj();
  GdkWindow *gdk_window = window->window;
  HWND hwnd = (HWND) GDK_WINDOW_HWND(gdk_window);
  ShowWindow(hwnd, b ? SW_SHOWNORMAL : SW_HIDE);
  if (b)
    {
      deiconify();
      raise();
    }
}

void
MainWindow::win32_init()
{
  TRACE_ENTER("MainWindow::win32_init");
  
  HINSTANCE hinstance = (HINSTANCE) GetModuleHandle(NULL);
  
  WNDCLASSEX wclass =
    {
      sizeof(WNDCLASSEX),
      0,
      win32_window_proc,
      0,
      0,
      hinstance,
      NULL,
      NULL,
      NULL,
      NULL,
      WIN32_MAIN_CLASS_NAME,
      NULL
    };
  ATOM atom = RegisterClassEx(&wclass);

  win32_main_hwnd = CreateWindowEx(WS_EX_TOOLWINDOW,
                                   WIN32_MAIN_CLASS_NAME,
                                   "Workrave",
                                   WS_OVERLAPPED,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   (HWND)NULL,
                                   (HMENU)NULL,
                                   hinstance,
                                   (LPSTR)NULL);
  ShowWindow(win32_main_hwnd, SW_HIDE);
  
  // User data
  SetWindowLong(win32_main_hwnd, GWL_USERDATA, (LONG) this);
  
  // Reassign ownership
  GtkWidget *window = Gtk::Widget::gobj();
  GdkWindow *gdk_window = window->window;
  HWND hwnd = (HWND) GDK_WINDOW_HWND(gdk_window);
  SetWindowLong(hwnd, GWL_HWNDPARENT, (LONG) win32_main_hwnd);

  // Tray icon
  win32_tray_icon.cbSize = sizeof(NOTIFYICONDATA);
  win32_tray_icon.hWnd = win32_main_hwnd;
  win32_tray_icon.uID = 1;
  win32_tray_icon.uFlags = NIF_ICON|NIF_TIP|NIF_MESSAGE;
  win32_tray_icon.uCallbackMessage = MYWM_TRAY_MESSAGE;
  win32_tray_icon.hIcon = LoadIcon(hinstance, "workrave");
  strcpy(win32_tray_icon.szTip, "Workrave");
  Shell_NotifyIcon(NIM_ADD, &win32_tray_icon);

  // Tray menu
  Menus *menus = Menus::get_instance();
  win32_tray_menu = menus->create_tray_menu();

  Gtk::Menu::MenuList &menulist = win32_tray_menu->items();

  menulist.push_front(Gtk::Menu_Helpers::StockMenuElem
                      (Gtk::Stock::OPEN,
                       SigC::slot(*this, &MainWindow::win32_on_tray_open)));
  TRACE_EXIT();
}

void
MainWindow::win32_exit()
{
  // Remember position
  win32_remember_position();

  // Destroy tray
  Shell_NotifyIcon(NIM_DELETE, &win32_tray_icon);
  DestroyWindow(win32_main_hwnd);
  UnregisterClass(WIN32_MAIN_CLASS_NAME, GetModuleHandle(NULL));
}

LRESULT CALLBACK
MainWindow::win32_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam,
                              LPARAM lParam)
{
  switch (uMsg)
    {
    case MYWM_TRAY_MESSAGE:
      {
        MainWindow *win;
        win = (MainWindow *) GetWindowLong(hwnd, GWL_USERDATA);
        switch (lParam)
          {
          case WM_RBUTTONUP:
            {
              GtkWidget *window = (GtkWidget*) win->win32_tray_menu->gobj();
              GdkWindow *gdk_window = window->window;
              HWND phwnd = (HWND) GDK_WINDOW_HWND(gdk_window);
              SetForegroundWindow(phwnd);
              win->win32_tray_menu->popup(0, GetTickCount());
            }
            break;
          case WM_LBUTTONDBLCLK:
            win->win32_show(true);
            break;
          }
      }
      break;
    }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//! User requested immediate restbreak.
void
MainWindow::win32_on_tray_open()
{
  win32_show(true);
}




void
MainWindow::win32_get_start_position(int &x, int &y)
{
  bool b;
  // FIXME: Default to right-bottom instead of 256x256
  Configurator *cfg = GUIControl::get_instance()->get_configurator();
  b = cfg->get_value(CFG_KEY_MAIN_WINDOW_X, &x);
  if (! b)
    {
      x = 256;
    }
  b = cfg->get_value(CFG_KEY_MAIN_WINDOW_Y, &y);
  if (! b)
    {
      y = 256;
    }
}

void
MainWindow::win32_set_start_position(int x, int y)
{
  Configurator *cfg = GUIControl::get_instance()->get_configurator();
  cfg->set_value(CFG_KEY_MAIN_WINDOW_X, x);
  cfg->set_value(CFG_KEY_MAIN_WINDOW_Y, y);
}


void
MainWindow::win32_remember_position()
{
  GtkWidget *window = Gtk::Widget::gobj();
  GdkWindow *gdk_window = window->window;
  HWND hwnd = (HWND) GDK_WINDOW_HWND(gdk_window);
  RECT rect;
  if (GetWindowRect(hwnd, &rect))
    {
      win32_set_start_position(rect.left, rect.top);
    }
}

#endif
