// MainWindow.cc --- Main info Window
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <unistd.h>
#include <iostream>

#ifdef HAVE_GNOME
#include <gnome.h>
#include "AppletWindow.hh"
#endif
#include "TimerBox.hh"
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

const string MainWindow::CFG_KEY_MAIN_WINDOW = "gui/main_window";
const string MainWindow::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP = "gui/main_window/always_on_top";

const string MainWindow::CFG_KEY_MAIN_WINDOW_START_IN_TRAY
= "gui/main_window/start_in_tray";

#ifdef WIN32
const char *WIN32_MAIN_CLASS_NAME = "Workrave";
const UINT MYWM_TRAY_MESSAGE = WM_USER +0x100;
#endif

const string MainWindow::CFG_KEY_MAIN_WINDOW_X
= "gui/main_window/x";
const string MainWindow::CFG_KEY_MAIN_WINDOW_Y
= "gui/main_window/y";

//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
MainWindow::MainWindow() :
  enabled(true),
  timers_box(NULL),
  monitor_suspended(false),
  iconified(false)
{
#ifdef HAVE_X
  leader = NULL;
#endif
  init();
}


//! Destructor.
MainWindow::~MainWindow()
{
  TRACE_ENTER("MainWindow::~MainWindow");

#ifdef WIN32
  win32_exit();
#endif
#ifdef HAVE_X
  delete leader;
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

  Configurator *config = GUIControl::get_instance()->get_configurator();
  config->add_listener(TimerBox::CFG_KEY_TIMERBOX + "main_window", this);

  enabled = TimerBox::is_enabled("main_window");

  Menus *menus = Menus::get_instance();
  menus->set_main_window(this);
  popup_menu = menus->create_main_window_menu();
  
  timers_box = manage(new TimerBox("main_window"));
  timers_box->set_geometry(true, -1);
  add(*timers_box);

  set_events(get_events() | Gdk::BUTTON_PRESS_MASK);

  
  // Necessary for popup menu 
  realize_if_needed();

  Glib::RefPtr<Gdk::Window> window = get_window();
  
#ifdef HAVE_X
  // HACK. this sets a different group leader in the WM_HINTS....
  // Without this hack, metacity makes ALL windows on-top.
  leader = new Gtk::Window(Gtk::WINDOW_POPUP);
  gtk_widget_realize(GTK_WIDGET(leader->gobj()));
  Glib::RefPtr<Gdk::Window> leader_window = leader->get_window();
  window->set_group(leader_window);
#endif
  
  set_resizable(false);
  stick();
  setup();

#ifdef WIN32
  window->set_functions(Gdk::FUNC_CLOSE|Gdk::FUNC_MOVE);

  win32_init();
  int x, y;
  get_start_position(x, y);
  set_gravity(Gdk::GRAVITY_STATIC); 
  set_position(Gtk::WIN_POS_NONE);
  if (!enabled) //  || get_start_in_tray())
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
  int x, y;
  get_start_position(x, y);
  set_gravity(Gdk::GRAVITY_STATIC); 
  set_position(Gtk::WIN_POS_NONE);
  show_all();
  move(x, y);
  TRACE_MSG(x << " " << y);
  
  if (!enabled) //  || get_start_in_tray())
    {
      close_window();
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

  bool new_enabled = TimerBox::is_enabled("main_window");

  TRACE_MSG("on top " << always_on_top);
  TRACE_MSG("enabled " << new_enabled);
  
  if (enabled != new_enabled)
    {
      enabled = new_enabled;
      if (enabled)
        {
          open_window();
        }
      else
        {
          close_window();
        }
    }
  if (always_on_top)
    {
      raise();
    }

  set_skipwinlist(false);
  
  TRACE_EXIT()
}


//! Updates the main window.
void
MainWindow::update()
{
  timers_box->update();
}



//! Opens the main window.
void
MainWindow::open_window()
{
  if (timers_box->get_visible_count() > 0)
    {
      deiconify();
      raise();
    }
}


//! Closes the main window.
void
MainWindow::close_window()
{
  iconify();
}


//! Toggles the main window.
void
MainWindow::toggle_window()
{
  if (iconified)
    {
      open_window();
    }
  else
    {
      close_window();
    }
}


//! User has closed the main window.
bool
MainWindow::on_delete_event(GdkEventAny *)
{
  TRACE_ENTER("MainWindow::on_delete_event");
  bool terminate = true;

#ifdef WIN32
  win32_show(false);
  terminate = false;
#else
  GUI *gui = GUI::get_instance(); 
  assert(gui != NULL);
  
#ifdef HAVE_GNOME
  AppletWindow *applet = gui->get_applet_window();
  if (applet != NULL)
    {
      AppletWindow::AppletMode mode = applet->get_applet_mode();
      terminate = mode == AppletWindow::APPLET_DISABLED;
    }
  if (terminate)
    {
      gui->terminate();
    }
  else
    {
      iconify();
    }
#else
  gui->terminate();
#endif // HAVE_GNOME
#endif // WIN32
  
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


bool
MainWindow::on_window_state_event(GdkEventWindowState *event)
{
  TRACE_ENTER("MainWindow::on_window_state_event");
  if (event != NULL)
    {
      if (event->changed_mask & GDK_WINDOW_STATE_ICONIFIED)
        {
          iconified = event->new_window_state & GDK_WINDOW_STATE_ICONIFIED;
          TimerBox::set_enabled("main_window", !iconified);
        }
    }

  TRACE_EXIT();
  return true;
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
    ->get_value(MainWindow::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, &rc);
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
    ->set_value(MainWindow::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, b);
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
  remember_position();

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


#endif

void
MainWindow::get_start_position(int &x, int &y)
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
MainWindow::set_start_position(int x, int y)
{
  Configurator *cfg = GUIControl::get_instance()->get_configurator();
  cfg->set_value(CFG_KEY_MAIN_WINDOW_X, x);
  cfg->set_value(CFG_KEY_MAIN_WINDOW_Y, y);
}


void
MainWindow::remember_position()
{
  int x, y;
  get_position(x, y);
  set_start_position(x, y);
}

void
MainWindow::set_skipwinlist(bool s)
{
  WindowHints::set_skip_winlist(Gtk::Widget::gobj(), s);
}
