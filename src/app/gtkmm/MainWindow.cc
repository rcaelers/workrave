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
#include "Util.hh"
#include "Text.hh"

#include "CoreFactory.hh"
#include "ConfiguratorInterface.hh"
#include "TimerInterface.hh"
#include "StatisticsInterface.hh"

#ifdef HAVE_DISTRIBUTION
#include "NetworkJoinDialog.hh"
#include "NetworkLogDialog.hh"
#endif

#ifdef WIN32
#include <gdk/gdkwin32.h>
#include <pbt.h>
#endif

#include "Menus.hh"

#include <gtkmm/menu.h>
#include <gtkmm/stock.h>

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
const string MainWindow::CFG_KEY_MAIN_WINDOW_HEAD_X
= "gui/main_window/head_x";
const string MainWindow::CFG_KEY_MAIN_WINDOW_HEAD_Y
= "gui/main_window/head_y";

//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
MainWindow::MainWindow() :
  enabled(true),
  timers_box(NULL),
  monitor_suspended(false),
  visible(true),
  applet_active(false)
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

  list<Glib::RefPtr<Gdk::Pixbuf> > icons;

  char *icon_files[] = { "workrave-icon-small.png",
                         "workrave-icon-medium.png",
                         "workrave-icon-large.png" };
  
  for (unsigned int i = 0; i < sizeof(icon_files) / sizeof(char *); i++)
    {
      string file = Util::complete_directory(icon_files[i], Util::SEARCH_PATH_IMAGES);

      try
        {
          Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(file);
          icons.push_back(pixbuf);
        }
      catch (...)
        {
        }
    }
  
  Glib::ListHandle<Glib::RefPtr<Gdk::Pixbuf> > icon_list(icons);
  set_icon_list(icon_list);
    
  ConfiguratorInterface *config = CoreFactory::get_configurator();
  config->add_listener(TimerBox::CFG_KEY_TIMERBOX + "main_window", this);

  enabled = TimerBox::is_enabled("main_window");

  Menus *menus = Menus::get_instance();
  menus->set_main_window(this);
  popup_menu = menus->create_main_window_menu();
  
  timers_box = manage(new TimerBox("main_window"));
  timers_box->set_geometry(true, -1);
  add(*timers_box);

  set_events(get_events() | Gdk::BUTTON_PRESS_MASK | Gdk::SUBSTRUCTURE_MASK);
  
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
  TRACE_MSG("moving to " << x << " " << y);
  move(x, y);
  
  if (!enabled) //  || get_start_in_tray())
    {
      iconify();
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
  TRACE_ENTER("MainWindow::open_window");
  if (timers_box->get_visible_count() > 0)
    {
      int x, y;
      set_position(Gtk::WIN_POS_NONE);
      set_gravity(Gdk::GRAVITY_STATIC); 
      get_start_position(x, y);
      TRACE_MSG("moving to " << x << " " << y);
      move(x, y);

#ifdef WIN32
      win32_show(true);
#else
      show_all();
      deiconify();
#endif
    }
  TRACE_EXIT();
}



//! Closes the main window.
void
MainWindow::close_window()
{
  // Remember position
  remember_position();

#ifdef WIN32
  win32_show(false);
#else
  if (applet_active)
    {
      hide_all();
    }
  else
    {
      iconify();
    }
#endif
}


//! Toggles the main window.
void
MainWindow::toggle_window()
{
  TimerBox::set_enabled("main_window", !enabled);
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
      close_window();
      TimerBox::set_enabled("main_window", false);
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
          bool iconified = event->new_window_state & GDK_WINDOW_STATE_ICONIFIED;
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
  b = CoreFactory::get_configurator()
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
  CoreFactory::get_configurator()
    ->set_value(MainWindow::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, b);
}


bool
MainWindow::get_start_in_tray() 
{
  bool b;
  bool rc;
  b = CoreFactory::get_configurator()
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
  CoreFactory::get_configurator()
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
      WindowHints::attach_thread_input(TRUE);
      present();
      WindowHints::attach_thread_input(FALSE);
    }
}

void
MainWindow::win32_init()
{
  TRACE_ENTER("MainWindow::win32_init");
  
  win32_hinstance = (HINSTANCE) GetModuleHandle(NULL);
  
  WNDCLASSEX wclass =
    {
      sizeof(WNDCLASSEX),
      0,
      win32_window_proc,
      0,
      0,
      win32_hinstance,
      NULL,
      NULL,
      NULL,
      NULL,
      WIN32_MAIN_CLASS_NAME,
      NULL
    };
  /* ATOM atom = */ RegisterClassEx(&wclass);

  win32_main_hwnd = CreateWindowEx(WS_EX_TOOLWINDOW,
                                   WIN32_MAIN_CLASS_NAME,
                                   "Workrave",
                                   WS_OVERLAPPED,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   (HWND)NULL,
                                   (HMENU)NULL,
                                   win32_hinstance,
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
  wm_taskbarcreated = RegisterWindowMessage("TaskbarCreated");
  win32_add_tray_icon();

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

void
MainWindow::win32_add_tray_icon()
{
  win32_tray_icon.cbSize = sizeof(NOTIFYICONDATA);
  win32_tray_icon.hWnd = win32_main_hwnd;
  win32_tray_icon.uID = 1;
  win32_tray_icon.uFlags = NIF_ICON|NIF_TIP|NIF_MESSAGE;
  win32_tray_icon.uCallbackMessage = MYWM_TRAY_MESSAGE;
  win32_tray_icon.hIcon = LoadIcon(win32_hinstance, "workrave");
  strcpy(win32_tray_icon.szTip, "Workrave");
  Shell_NotifyIcon(NIM_ADD, &win32_tray_icon);
  DestroyIcon(win32_tray_icon.hIcon);
}

LRESULT CALLBACK
MainWindow::win32_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam,
                              LPARAM lParam)
{
  TRACE_ENTER("MainWindow::win32_window_proc");
  MainWindow *win = (MainWindow *) GetWindowLong(hwnd, GWL_USERDATA);
  if (win != NULL)
    {
      if (uMsg == win->wm_taskbarcreated)
        {
          win->win32_add_tray_icon();
        }
      else if (uMsg == MYWM_TRAY_MESSAGE)
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
      else if (uMsg == WM_POWERBROADCAST)
        {
          TRACE_MSG("WM_POWERBROADCAST " << wParam << " " << lParam);
          switch (wParam)
            {
            case PBT_APMQUERYSUSPEND:
              TRACE_MSG("Query Suspend");
              return TRUE;

            case PBT_APMQUERYSUSPENDFAILED:
              TRACE_MSG("Query Suspend Failed");
              break;

            case PBT_APMRESUMESUSPEND:
              {
                TRACE_MSG("Resume suspend");
                CoreInterface *core = CoreFactory::get_core();
                assert(core != NULL);
                core->set_powersave(false);
              }
              break;

            case PBT_APMSUSPEND:
              {
                TRACE_MSG("Suspend");
                CoreInterface *core = CoreFactory::get_core();
                assert(core != NULL);
                core->set_powersave(true);
              }
              break;
            }
        }
      else if (uMsg == WM_DISPLAYCHANGE)
        {
          TRACE_MSG("WM_DISPLAYCHANGE " << wParam << " " << lParam);
          GUI *gui = GUI::get_instance(); 
          assert(gui != NULL);
          gui->init_multihead();
        }
    }
  
  TRACE_EXIT();
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
  TRACE_ENTER("MainWindow::get_start_position");
  bool b;
  // FIXME: Default to right-bottom instead of 256x256
  ConfiguratorInterface *cfg = CoreFactory::get_configurator();
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
  TRACE_MSG(x << " " << y);
  TRACE_EXIT();
}


void
MainWindow::set_start_position(int x, int y)
{
  TRACE_ENTER_MSG("MainWindow::set_start_position", x << " " << y);
  ConfiguratorInterface *cfg = CoreFactory::get_configurator();
  cfg->set_value(CFG_KEY_MAIN_WINDOW_X, x);
  cfg->set_value(CFG_KEY_MAIN_WINDOW_Y, y);
  TRACE_EXIT();
}


void
MainWindow::get_head_start_position(int &x, int &y)
{
  TRACE_ENTER("MainWindow::get_head_start_position");
  bool b;
  int sx, sy;
  get_start_position(sx, sy);
  
  ConfiguratorInterface *cfg = CoreFactory::get_configurator();
  b = cfg->get_value(CFG_KEY_MAIN_WINDOW_HEAD_X, &x);
  if (! b)
    {
      x = sx;
    }
  b = cfg->get_value(CFG_KEY_MAIN_WINDOW_HEAD_Y, &y);
  if (! b)
    {
      y = sy;
    }
  
  TRACE_MSG(x << " " << y);
  TRACE_EXIT();
}


void
MainWindow::set_head_start_position(int x, int y)
{
  TRACE_ENTER_MSG("MainWindow::set_head_start_position", x << " " << y);
  ConfiguratorInterface *cfg = CoreFactory::get_configurator();
  cfg->set_value(CFG_KEY_MAIN_WINDOW_HEAD_X, x);
  cfg->set_value(CFG_KEY_MAIN_WINDOW_HEAD_Y, y);
  TRACE_EXIT();
}


void
MainWindow::remember_position()
{
  int x, y;
  get_position(x, y);
  set_start_position(x, y);
}


void
MainWindow::set_applet_active(bool a)
{
  TRACE_ENTER_MSG("MainWindow::set_applet_active", a);
  if (applet_active != a)
    {
      applet_active = a;

      if (!enabled)
        {
          if (applet_active)
            {
              hide_all();
            }
          else
            {
              iconify();
              show_all();
            }
        }
    }
  TRACE_EXIT();
}


bool
MainWindow::on_configure_event(GdkEventConfigure *event)
{
  TRACE_ENTER("MainWindow::on_configure_event");
  // This method doesn't seem to do anything. Howener, GUI.cc does not
  // receive the configure event signal without this. Donno why....
  (void) event;
  Widget::on_configure_event(event);
  TRACE_EXIT();;
  return false;
}
