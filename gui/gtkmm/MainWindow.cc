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
#else
#include "gnome-about.h"
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

#ifdef WIN32
const char *WIN32_MAIN_CLASS_NAME = "Workrave";
const UINT MYWM_TRAY_MESSAGE = WM_USER +0x100;

const string MainWindow::CFG_KEY_MAIN_WINDOW_START_IN_TRAY
= "gui/main_window/start_in_tray";
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
  core_control(c),
  gui(g),
  timers_box(NULL),
  timer_names(NULL),
  timer_times(NULL),
  monitor_suspended(false)
#ifdef HAVE_DISTRIBUTION
  ,network_log_dialog(NULL)
#endif
{
  init();
}


//! Destructor.
MainWindow::~MainWindow()
{
  TRACE_ENTER("MainWindow::~MainWindow");

  if (timer_times != NULL)
    {
      delete [] timer_times;
    }

  if (timer_names != NULL)
    {
      delete [] timer_names;
    }

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
  
  Configurator *config = gui->get_configurator();
  if (config != NULL)
    {
      config->add_listener(GUIControl::CFG_KEY_MAIN_WINDOW, this);
    }
  
  popup_menu = manage(create_menu(popup_check_menus));

  set_border_width(2);
  timers_box = manage(new Gtk::Table(GUIControl::BREAK_ID_SIZEOF, 2, false));
  timers_box->set_spacings(2);
  
  timer_names = new Gtk::Widget*[GUIControl::BREAK_ID_SIZEOF];
  timer_times = new TimeBar*[GUIControl::BREAK_ID_SIZEOF];

  for (int count = 0; count < GUIControl::BREAK_ID_SIZEOF; count++)
    {
      GUIControl::TimerData *timer = &GUIControl::get_instance()->timers[count];
      Gtk::Image *img = manage(new Gtk::Image(timer->icon));
      Gtk::Widget *w;
      if (count == GUIControl::BREAK_ID_REST_BREAK)
	{
	  Gtk::Button *b = manage(new Gtk::Button());
          // GTK_WIDGET_UNSET_FLAGS(b->gobj(), GTK_CAN_FOCUS);
	  b->set_relief(Gtk::RELIEF_NONE);
	  b->set_border_width(0);
	  b->add(*img);
	  b->signal_clicked().connect(SigC::slot(*this, &MainWindow::on_menu_restbreak_now));
	  w = b;
	}
      else
	{
	  w = img;
	}
      timer_names[count] = w;
      timers_box->attach(*w, 0, 1, count, count + 1, Gtk::FILL);
      
      timer_times[count] = manage(new TimeBar);
      
      timer_times[count]->set_text_alignment(1);
      timer_times[count]->set_progress(0, 60);
      timer_times[count]->set_text(_("Wait"));
      
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
  window->set_functions(Gdk::FUNC_CLOSE|Gdk::FUNC_MOVE);
  WindowHints::set_tool_window(Gtk::Widget::gobj(), true);
  WindowHints::set_skip_winlist(Gtk::Widget::gobj(), true);

#ifdef WIN32
  win32_init();
  int x, y;
  win32_get_start_position(x, y);
  set_gravity(Gdk::GRAVITY_STATIC); 
  set_position(Gtk::WIN_POS_NONE);
  if (win32_get_start_in_tray())
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
#endif
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
  bool node_master = true;
  int num_peers = 0;

#ifdef HAVE_DISTRIBUTION
  DistributionManager *dist_manager = DistributionManager::get_instance();
  if (dist_manager != NULL)
    {
      node_master = dist_manager->is_master();
      num_peers = dist_manager->get_number_of_peers();
    }
#endif
  
  for (unsigned int count = 0; count < GUIControl::BREAK_ID_SIZEOF; count++)
    {
      TimerInterface *timer = GUIControl::get_instance()->timers[count].timer;
      TimeBar *bar = timer_times[count];

      if (!node_master && num_peers > 0)
        {
          bar->set_text(_("Inactive"));
          bar->update();
          continue;
        }
  
      if (timer == NULL)
        {
          // FIXME: error handling.
          continue;
        }
      
      TimerInterface::TimerState timerState = timer->get_state();

      // Collect some data.
      time_t maxActiveTime = timer->get_limit();
      time_t activeTime = timer->get_elapsed_time();
      time_t breakDuration = timer->get_auto_reset();
      time_t idleTime = timer->get_elapsed_idle_time();
      bool overdue = (maxActiveTime < activeTime);
          
      // Set the text
      if (timer->is_limit_enabled() && maxActiveTime != 0)
        {
          bar->set_text(Text::time_to_string(maxActiveTime - activeTime));
        }
      else
        {
          bar->set_text(Text::time_to_string(activeTime));
        }

      // And set the bar.
      bar->set_secondary_progress(0, 0);

      if (timerState == TimerInterface::STATE_INVALID)
        {
          bar->set_bar_color(TimeBar::COLOR_ID_INACTIVE);
          bar->set_progress(0, 60);
          bar->set_text(_("Wait"));
        }
      else
        {
          // Timer is running, show elapsed time.
          bar->set_progress(activeTime, maxActiveTime);
          
          if (overdue)
            {
              bar->set_bar_color(TimeBar::COLOR_ID_OVERDUE);
            }
          else
            {
              bar->set_bar_color(TimeBar::COLOR_ID_ACTIVE);
            }

	  if (//timerState == TimerInterface::STATE_STOPPED &&
	      timer->is_auto_reset_enabled() && breakDuration != 0)
	    {
	      // resting.
	      bar->set_secondary_bar_color(TimeBar::COLOR_ID_INACTIVE);
	      bar->set_secondary_progress(idleTime, breakDuration);
	    }
        }
      bar->update();
    }

  return;
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



//! Create the popup-menu
Gtk::Menu *
MainWindow::create_menu(Gtk::CheckMenuItem *check_menus[4])
{
  TRACE_ENTER("MainWindow::create_menu");
  //FIXME: untested, added manage
  Gtk::Menu *pop_menu = manage(new Gtk::Menu());
  
  Gtk::Menu::MenuList &menulist = pop_menu->items();

  Gtk::Menu *mode_menu = manage(new Gtk::Menu());
  Gtk::Menu::MenuList &modemenulist = mode_menu->items();

  // Mode menu item
  Gtk::MenuItem *mode_menu_item = manage(new Gtk::MenuItem(_("_Mode"),true));
  mode_menu_item->set_submenu(*mode_menu);
  mode_menu_item->show();

  Gtk::RadioMenuItem::Group gr;
  // Suspend menu item.
  Gtk::RadioMenuItem *normal_menu_item
    = manage(new Gtk::RadioMenuItem(gr, _("_Normal"), true));
  normal_menu_item->signal_toggled().connect(SigC::slot(*this, &MainWindow::on_menu_normal));
  normal_menu_item->show();
  modemenulist.push_back(*normal_menu_item);

  // Suspend menu item.
  Gtk::RadioMenuItem *suspend_menu_item
    = manage(new Gtk::RadioMenuItem(gr, _("_Suspended"), true));
  suspend_menu_item->signal_toggled().connect(SigC::slot(*this, &MainWindow::on_menu_suspend));
  suspend_menu_item->show();
  modemenulist.push_back(*suspend_menu_item);

  // Quiet menu item.
  Gtk::RadioMenuItem *quiet_menu_item
    = manage(new Gtk::RadioMenuItem(gr, _("Q_uiet"), true));
  quiet_menu_item->signal_toggled().connect(SigC::slot(*this, &MainWindow::on_menu_quiet));
  quiet_menu_item->show();
  modemenulist.push_back(*quiet_menu_item);

  check_menus[0] = normal_menu_item;
  check_menus[1] = suspend_menu_item;
  check_menus[2] = quiet_menu_item;

#ifdef HAVE_DISTRIBUTION
  // Distribution menu
  Gtk::Menu *distr_menu = manage(new Gtk::Menu());
  Gtk::Menu::MenuList &distr_menu_list = distr_menu->items();

  Gtk::MenuItem *distr_menu_item = manage(new Gtk::MenuItem(_("_Network"),true));
  distr_menu_item->set_submenu(*distr_menu);
  distr_menu_item->show();

  Gtk::MenuItem *distr_join_menu_item = manage(new Gtk::MenuItem(_("_Connect"),true));
  distr_join_menu_item->show();
  distr_join_menu_item->signal_activate().connect(SigC::slot(*this, &MainWindow::on_menu_network_join));
  distr_menu_list.push_back(*distr_join_menu_item);
  
  Gtk::MenuItem *distr_leave_menu_item = manage(new Gtk::MenuItem(_("_Disconnect"),true));
  distr_leave_menu_item->show();
  distr_leave_menu_item->signal_activate().connect(SigC::slot(*this, &MainWindow::on_menu_network_leave));
  distr_menu_list.push_back(*distr_leave_menu_item);

  Gtk::MenuItem *distr_reconnect_menu_item = manage(new Gtk::MenuItem(_("_Reconnect"),true));
  distr_reconnect_menu_item->show();
  distr_reconnect_menu_item->signal_activate().connect(SigC::slot(*this, &MainWindow::on_menu_network_reconnect));
  distr_menu_list.push_back(*distr_reconnect_menu_item);

  Gtk::CheckMenuItem *distr_log_menu_item = manage(new Gtk::CheckMenuItem(_("Show _log"), true));
  distr_log_menu_item->show();
  distr_log_menu_item->signal_toggled().connect(SigC::slot(*this, &MainWindow::on_menu_network_log));
  distr_menu_list.push_back(*distr_log_menu_item);

  check_menus[3] = distr_log_menu_item;
#endif
  
  // FIXME: add separators, etc...
  menulist.push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::PREFERENCES,
                                                      SigC::slot(*this, &MainWindow::on_menu_preferences)));


  Gtk::Image *img = manage(new Gtk::Image(GUIControl::get_instance()->timers[GUIControl::BREAK_ID_REST_BREAK].icon));
  menulist.push_back(Gtk::Menu_Helpers::ImageMenuElem
                     (_("_Rest break"),
                      Gtk::Menu::AccelKey("<control>r"),
                      *img,
                      SigC::slot(*this, &MainWindow::on_menu_restbreak_now)));

  menulist.push_back(*mode_menu_item);

#ifdef HAVE_DISTRIBUTION
  menulist.push_back(*distr_menu_item);
#endif
  
#ifndef NDEBUG
  menulist.push_back(Gtk::Menu_Helpers::MenuElem(_("Statistics"), 
                                                 SigC::slot(*this, &MainWindow::on_menu_statistics)));
  
  menulist.push_back(Gtk::Menu_Helpers::MenuElem("_Test",
                                                 SigC::slot(*this, &MainWindow::on_test_me)));
#endif
  
#ifdef HAVE_GNOME
  menulist.push_back(Gtk::Menu_Helpers::StockMenuElem
		     (Gtk::StockID(GNOME_STOCK_ABOUT),
		      SigC::slot(*this, &MainWindow::on_menu_about)));
#else
  menulist.push_back(Gtk::Menu_Helpers::MenuElem
		     (_("About..."),
		      SigC::slot(*this, &MainWindow::on_menu_about)));
#endif

  menulist.push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::QUIT,
                                                 SigC::slot(*this, &MainWindow::on_menu_quit)));

  TRACE_EXIT();
  return pop_menu;
}




void
MainWindow::config_changed_notify(string key)
{
  TRACE_ENTER("MainWindow::config_changed_notify");
  (void) key;
  setup();
  TRACE_EXIT();
}


//! Users pressed some mouse button in the main window.
bool
MainWindow::on_button_press_event(GdkEventButton *event)
{
  TRACE_ENTER("MainWindow::on_button_press_event");
  bool ret = false;

  // TODO: magic number.
  if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
    {
      popup_menu->popup(event->button, event->time);
      ret = true;
    }

  TRACE_EXIT();
  return ret;
}


//! User requested application quit....
void
MainWindow::on_menu_quit()
{
  TRACE_ENTER("MainWindow::on_menu_quit");

  gui->terminate();

  TRACE_EXIT();
}


//! User requested immediate restbreak.
void
MainWindow::on_menu_restbreak_now()
{
  gui->restbreak_now();
}


//! User requested immediate restbreak.
void
MainWindow::on_menu_quiet()
{
  TRACE_ENTER("MainWindow::on_menu_quiet");

  gui->set_operation_mode(GUIControl::OPERATION_MODE_QUIET);
#ifdef WIN32
  win32_sync_menu(2);
#endif
  TRACE_EXIT();
}



//! User requested immediate restbreak.
void
MainWindow::on_menu_suspend()
{
  TRACE_ENTER("MainWindow::on_menu_suspend");

  gui->set_operation_mode(GUIControl::OPERATION_MODE_SUSPENDED);
#ifdef WIN32
  win32_sync_menu(1);
#endif
  TRACE_EXIT();
}

void
MainWindow::on_menu_normal()
{
  gui->set_operation_mode(GUIControl::OPERATION_MODE_NORMAL);
#ifdef WIN3
  win32_sync_menu(0);
#endif
}


#ifndef NDEBUG
//! User test code.
void
MainWindow::on_test_me()
{
  core_control->test_me();

  Statistics *stats = Statistics::get_instance();
  stats->dump();
}
#endif

//! Preferences Dialog.
void
MainWindow::on_menu_preferences()
{
  GUIControl::OperationMode mode;
  GUIControl *ctrl = GUIControl::get_instance();
  mode = ctrl->set_operation_mode(GUIControl::OPERATION_MODE_QUIET);

  PreferencesDialog *dialog = new PreferencesDialog();
  dialog->run();
  delete dialog;

#ifdef WIN32
  // FIXME: bug 130:
  // due to current Gtk+ behaviour of exit()'ing on WM_QUIT, we cannot
  // store main window position on shutdown (bug 130).
  // Therefore, this hack.
  win32_remember_position();
#endif
  ctrl->set_operation_mode(mode);
}


//! Preferences Dialog.
void
MainWindow::on_menu_statistics()
{
  GUIControl::OperationMode mode;
  GUIControl *ctrl = GUIControl::get_instance();
  mode = ctrl->set_operation_mode(GUIControl::OPERATION_MODE_QUIET);

//   Statistics *stats = Statistics::get_instance();
//   stats->start_new_day();
  
  StatisticsDialog *dialog = new StatisticsDialog();
  dialog->run();
  delete dialog;

  ctrl->set_operation_mode(mode);
}


//! About Dialog.
void
MainWindow::on_menu_about()
{
  const gchar *authors[] = {
   "Rob Caelers <robc@krandor.org>",
   "Raymond Penners <raymond@dotsphinx.com>",
   NULL
  };
  string icon = Util::complete_directory("workrave.png",
                                         Util::SEARCH_PATH_IMAGES);
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(icon.c_str(), NULL);  
  gtk_widget_show (gnome_about_new
                   ("Workrave", VERSION,
                    "Copyright 2001-2002 Rob Caelers & Raymond Penners",
                    _("This program assists in the prevention and recovery"
                      " of Repetitive Strain Injury (RSI)."),
                    (const gchar **) authors,
                    (const gchar **) NULL,
                    NULL,
                    pixbuf));
}


#ifdef HAVE_DISTRIBUTION
void
MainWindow::on_menu_network_join()
{
  GUIControl::OperationMode mode;
  GUIControl *ctrl = GUIControl::get_instance();
  mode = ctrl->set_operation_mode(GUIControl::OPERATION_MODE_QUIET);

  NetworkJoinDialog *dialog = new NetworkJoinDialog();
  dialog->run();
  delete dialog;

  ctrl->set_operation_mode(mode);
}


void
MainWindow::on_menu_network_leave()
{
  DistributionManager *dist_manager = DistributionManager::get_instance();
  if (dist_manager != NULL)
    {
      dist_manager->disconnect_all();
    }
}

void
MainWindow::on_menu_network_reconnect()
{
  DistributionManager *dist_manager = DistributionManager::get_instance();
  if (dist_manager != NULL)
    {
      dist_manager->reconnect_all();
    }
}

void
MainWindow::on_menu_network_log()
{
  TRACE_ENTER("MainWindow::on_menu_network_log");

#ifdef WIN32 // HACK ALERT.
  static bool syncing = false;
  if (syncing)
    return;
  syncing = true;

  static bool prev_popup_active = false;
  static bool prev_tray_active = false;
  
  bool popup_active = popup_check_menus[3]->get_active();
  bool tray_active = win32_tray_check_menus[3]->get_active();
  bool active = false;
  
  if (popup_active != prev_popup_active)
    {
      active = popup_active;
    }
  else if (tray_active != prev_tray_active)
    {
      active = tray_active;
    }
  prev_popup_active = active;
  prev_tray_active = active;

  popup_check_menus[3]->set_active(active);
  win32_tray_check_menus[3]->set_active(active);
  
  syncing = false;
  
#else
  bool active = popup_check_menus[3]->get_active();
#endif
  TRACE_MSG("active = " << active);
  
  if (active)
    {
      TRACE_MSG("new log ");
      network_log_dialog = new NetworkLogDialog();
      network_log_dialog->signal_response().connect(SigC::slot(*this, &MainWindow::on_network_log_response));
  
      network_log_dialog->run();
    }
  else if (network_log_dialog != NULL)
    {
      TRACE_MSG("close log ");
      network_log_dialog->hide_all();
      delete network_log_dialog;
      network_log_dialog = NULL;
    }

  TRACE_EXIT();
}

void
MainWindow::on_network_log_response(int response)
{ 
  network_log_dialog->hide_all();
  popup_check_menus[3]->set_active(false);
#ifdef WIN32
  win32_tray_check_menus[3]->set_active(false);
#endif
  // done by gtkmm ??? delete network_log_dialog;
  network_log_dialog = NULL;
}

#endif

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
  win32_tray_menu = manage(create_menu(win32_tray_check_menus));
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
MainWindow::win32_sync_menu(int mode)
{
  TRACE_ENTER("win32_sync_menu");

  // Ugh, isn't there an other way to prevent endless signal loops?
  static bool syncing = false;
  if (syncing)
    return;
  syncing = true;

  if (! popup_check_menus[mode]->get_active())
    popup_check_menus[mode]->set_active(true);
  if (! win32_tray_check_menus[mode]->get_active())
    win32_tray_check_menus[mode]->set_active(true);

  syncing = false;

  TRACE_EXIT();
}

bool
MainWindow::win32_get_start_in_tray() 
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
MainWindow::win32_set_start_in_tray(bool b)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_MAIN_WINDOW_START_IN_TRAY, b);
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
