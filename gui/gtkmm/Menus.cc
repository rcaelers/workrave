// Menus.cc --- Timer info Window
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

#include "Menus.hh"
#include "TimeBar.hh"
#include "GUI.hh"
#include "GUIControl.hh"
#include "Util.hh"
#include "Text.hh"

#include "PreferencesDialog.hh"
#include "StatisticsDialog.hh"
#include "Statistics.hh"

#include "TimerInterface.hh"
#include "ActivityMonitorInterface.hh"
#include "Configurator.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#include "NetworkJoinDialog.hh"
#include "NetworkLogDialog.hh"
#endif
#ifdef HAVE_GNOME
#include "AppletWindow.hh"
#else
#include "gnome-about.h"
#endif
#include "MainWindow.hh"

#ifndef NDEBUG
#include "ControlInterface.hh"
#endif

Menus *Menus::instance = NULL;


//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
Menus::Menus() :
#ifdef HAVE_GNOME  
  applet_window(NULL),
#endif
#ifdef HAVE_DISTRIBUTION
  network_log_dialog(NULL),
#endif
  statistics_dialog(NULL),
  preferences_dialog(NULL),
  main_window(NULL)
{
  gui = GUI::get_instance();

  for (int i = 0; i < MAX_CHECKMENUS; i++)
    {
      main_window_check_menus[i] = NULL;
      tray_check_menus[i] = NULL;
    }
}


//! Destructor.
Menus::~Menus()
{
  TRACE_ENTER("Menus::~Menus");
  TRACE_EXIT();
}


Gtk::Menu *
Menus::create_main_window_menu()
{
  Gtk::Menu *menu = manage(create_menu(main_window_check_menus)); // FIXME: leak
    
#ifdef HAVE_DISTRIBUTION
  main_window_check_menus[3]->signal_toggled().connect(SigC::slot(*this,
                                                                  &Menus::on_menu_network_log_main_window));
#endif  

  return menu;
}


Gtk::Menu *
Menus::create_tray_menu()
{
  Gtk::Menu *menu = manage(create_menu(tray_check_menus));
#ifdef HAVE_DISTRIBUTION
  tray_check_menus[3]->signal_toggled().connect(SigC::slot(*this,
                                                           &Menus::on_menu_network_log_tray));
#endif

  return menu;
}


//! Create the popup-menu
Gtk::Menu *
Menus::create_menu(Gtk::CheckMenuItem *check_menus[4])
{
  TRACE_ENTER("Menus::create_menu");
  //FIXME: untested, added manage
  Gtk::Menu *pop_menu = manage(new Gtk::Menu());
  
  Gtk::Menu::MenuList &menulist = pop_menu->items();

  Gtk::Menu *mode_menu = manage(new Gtk::Menu());
  Gtk::Menu::MenuList &modemenulist = mode_menu->items();

  // Mode menu item
  Gtk::MenuItem *mode_menu_item = manage(new Gtk::MenuItem(_("_Mode"),true)); // FIXME: LEAK
  mode_menu_item->set_submenu(*mode_menu);
  mode_menu_item->show();

  Gtk::RadioMenuItem::Group gr;
  // Suspend menu item.
  Gtk::RadioMenuItem *normal_menu_item
    = manage(new Gtk::RadioMenuItem(gr, _("_Normal"), true));
  normal_menu_item->signal_toggled().connect(bind(SigC::slot(*this, &Menus::on_menu_normal_menu), normal_menu_item));
  normal_menu_item->show();
  modemenulist.push_back(*normal_menu_item);

  // Suspend menu item.
  Gtk::RadioMenuItem *suspend_menu_item
    = manage(new Gtk::RadioMenuItem(gr, _("_Suspended"), true));
  suspend_menu_item->signal_toggled().connect(bind(SigC::slot(*this, &Menus::on_menu_suspend_menu), suspend_menu_item));
  suspend_menu_item->show();
  modemenulist.push_back(*suspend_menu_item);

  // Quiet menu item.
  Gtk::RadioMenuItem *quiet_menu_item
    = manage(new Gtk::RadioMenuItem(gr, _("Q_uiet"), true));
  quiet_menu_item->signal_toggled().connect(bind(SigC::slot(*this, &Menus::on_menu_quiet_menu),quiet_menu_item));
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
  distr_join_menu_item->signal_activate().connect(SigC::slot(*this, &Menus::on_menu_network_join));
  distr_menu_list.push_back(*distr_join_menu_item);
  
  Gtk::MenuItem *distr_leave_menu_item = manage(new Gtk::MenuItem(_("_Disconnect"),true));
  distr_leave_menu_item->show();
  distr_leave_menu_item->signal_activate().connect(SigC::slot(*this, &Menus::on_menu_network_leave));
  distr_menu_list.push_back(*distr_leave_menu_item);

  Gtk::MenuItem *distr_reconnect_menu_item = manage(new Gtk::MenuItem(_("_Reconnect"),true));
  distr_reconnect_menu_item->show();
  distr_reconnect_menu_item->signal_activate().connect(SigC::slot(*this, &Menus::on_menu_network_reconnect));
  distr_menu_list.push_back(*distr_reconnect_menu_item);

  Gtk::CheckMenuItem *distr_log_menu_item = manage(new Gtk::CheckMenuItem(_("Show _log"), true));
  distr_log_menu_item->show();
  distr_menu_list.push_back(*distr_log_menu_item);

  check_menus[3] = distr_log_menu_item;
#endif
  
  // FIXME: add separators, etc...
  menulist.push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::PREFERENCES,
                                                      SigC::slot(*this, &Menus::on_menu_preferences)));


  Gtk::Image *img = manage(new Gtk::Image(GUIControl::get_instance()->timers[GUIControl::BREAK_ID_REST_BREAK].icon));
  menulist.push_back(Gtk::Menu_Helpers::ImageMenuElem
                     (_("_Rest break"),
                      Gtk::Menu::AccelKey("<control>r"),
                      *img,
                      SigC::slot(*this, &Menus::on_menu_restbreak_now)));

  menulist.push_back(*mode_menu_item);

#ifdef HAVE_DISTRIBUTION
  menulist.push_back(*distr_menu_item);
#endif
  
  menulist.push_back(Gtk::Menu_Helpers::MenuElem(_("Statistics"), 
                                                 SigC::slot(*this, &Menus::on_menu_statistics)));
  
#ifndef NDEBUG
  menulist.push_back(Gtk::Menu_Helpers::MenuElem("_Test",
                                                 SigC::slot(*this, &Menus::on_test_me)));
#endif
  
#ifdef HAVE_GNOME
  menulist.push_back(Gtk::Menu_Helpers::StockMenuElem
		     (Gtk::StockID(GNOME_STOCK_ABOUT),
		      SigC::slot(*this, &Menus::on_menu_about)));
#else
  menulist.push_back(Gtk::Menu_Helpers::MenuElem
		     (_("About..."),
		      SigC::slot(*this, &Menus::on_menu_about)));
#endif

  menulist.push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::QUIT,
                                                 SigC::slot(*this, &Menus::on_menu_quit)));

  TRACE_EXIT();
  return pop_menu;
}


void
Menus::sync_mode_menu(int mode)
{
  TRACE_ENTER("sync_mode_menu");

  // Ugh, isn't there an other way to prevent endless signal loops?
  static bool syncing = false;
  if (syncing)
    return;
  syncing = true;

  if (main_window_check_menus[mode] != NULL && !main_window_check_menus[mode]->get_active())
    {
      TRACE_MSG("setting active");
      main_window_check_menus[mode]->set_active(true);
    }

  if (tray_check_menus[mode] != NULL && !tray_check_menus[mode]->get_active())
    tray_check_menus[mode]->set_active(true);
  
#ifdef HAVE_GNOME
  if (applet_window != NULL)
    {
      applet_window->set_menu_active(mode, true);
    }
#endif
  
  syncing = false;

  TRACE_EXIT();
}

void
Menus::sync_tray_menu(bool active)
{
  TRACE_ENTER("sync_tray_menu");

  static bool syncing = false;
  if (syncing)
    return;
  syncing = true;

  if (main_window_check_menus[3] != NULL)
    {
      main_window_check_menus[3]->set_active(active);
    }

  if (tray_check_menus[3] != NULL)
    {
      tray_check_menus[3]->set_active(active);
    }
  
#ifdef HAVE_GNOME
  if (applet_window != NULL)
    {
      applet_window->set_menu_active(3, active);
    }
#endif
  
  syncing = false;

  TRACE_EXIT();
}


void
Menus::resync_applet()
{
#ifdef HAVE_GNOME
  if (applet_window != NULL)
    {
      GUIControl *gui_control = GUIControl::get_instance();
      GUIControl::OperationMode mode = gui_control->get_operation_mode();      
      switch(mode)
        {
        case GUIControl::OPERATION_MODE_NORMAL:
          applet_window->set_menu_active(0, true);
          break;
        case GUIControl::OPERATION_MODE_SUSPENDED:
          applet_window->set_menu_active(1, true);
          break;
        case GUIControl::OPERATION_MODE_QUIET:
          applet_window->set_menu_active(2, true);
          break;
        default:
          break;
        }
    }
#endif

#ifdef HAVE_GNOME
  if (applet_window != NULL)
    {
      applet_window->set_menu_active(3, network_log_dialog != NULL);
    }
#endif
}


void
Menus::on_menu_open_main_window()
{
  gui->open_main_window();
}


//! User requested application quit....
void
Menus::on_menu_quit()
{
  TRACE_ENTER("Menus::on_menu_quit");

  gui->terminate();

  TRACE_EXIT();
}


//! User requested immediate restbreak.
void
Menus::on_menu_restbreak_now()
{
  gui->restbreak_now();
}


//! User requested immediate restbreak.
void
Menus::on_menu_quiet()
{
  TRACE_ENTER("Menus::on_menu_quiet");

  GUIControl *gui_control = GUIControl::get_instance();
  gui_control->set_operation_mode(GUIControl::OPERATION_MODE_QUIET);
  sync_mode_menu(2);
  TRACE_EXIT();
}



//! User requested immediate restbreak.
void
Menus::on_menu_suspend()
{
  TRACE_ENTER("Menus::on_menu_suspend");

  GUIControl *gui_control = GUIControl::get_instance();
  gui_control->set_operation_mode(GUIControl::OPERATION_MODE_SUSPENDED);
  sync_mode_menu(1);
  TRACE_EXIT();
}


void
Menus::on_menu_normal_menu(Gtk::CheckMenuItem *menu)
{
  TRACE_ENTER("Menus::on_menu_normal");
  if (menu != NULL && menu->get_active())
    {
      on_menu_normal();
    }
  TRACE_EXIT();
}


//! User requested immediate restbreak.
void
Menus::on_menu_quiet_menu(Gtk::CheckMenuItem *menu)
{
  TRACE_ENTER("Menus::on_menu_quiet");

  if (menu != NULL && menu->get_active())
    {
      on_menu_quiet();
    }
  TRACE_EXIT();
}



//! User requested immediate restbreak.
void
Menus::on_menu_suspend_menu(Gtk::CheckMenuItem *menu)
{
  TRACE_ENTER("Menus::on_menu_suspend");

  if (menu != NULL && menu->get_active())
    {
      on_menu_suspend();
    }

  TRACE_EXIT();
}


void
Menus::on_menu_normal()
{
  TRACE_ENTER("Menus::on_menu_normal");
  GUIControl *gui_control = GUIControl::get_instance();
  gui_control->set_operation_mode(GUIControl::OPERATION_MODE_NORMAL);
  sync_mode_menu(0);
  TRACE_EXIT();
}


#ifndef NDEBUG
//! User test code.
void
Menus::on_test_me()
{
  Statistics *stats = Statistics::get_instance();
  stats->dump();

  GUIControl *gui_control = GUIControl::get_instance();
  ControlInterface *core = gui_control->get_core();
  core->test_me();
}
#endif


//! Preferences Dialog.
// void
// Menus::on_menu_preferences()
// {
//   TRACE_ENTER("Menus::on_menu_preferences");
//   GUIControl::OperationMode mode;
//   GUIControl *gui_control = GUIControl::get_instance();
//   mode = gui_control->set_operation_mode(GUIControl::OPERATION_MODE_QUIET);

//   PreferencesDialog *dialog = new PreferencesDialog();
//   dialog->run();
//   delete dialog;

// #ifdef WIN32
//   // FIXME: bug 130:
//   // due to current Gtk+ behaviour of exit()'ing on WM_QUIT, we cannot
//   // store main window position on shutdown (bug 130).
//   // Therefore, this hack.

//   MainWindow *window = gui->get_main_window();
//   if (window != NULL)
//     {
//       window->win32_remember_position();
//     }
// #endif
//   gui_control->set_operation_mode(mode);
//   TRACE_EXIT();
// }


//! Preferences Dialog.
void
Menus::on_menu_preferences()
{
  if (preferences_dialog == NULL)
    {
      preferences_dialog = new PreferencesDialog();
      preferences_dialog->signal_response().connect(SigC::slot(*this, &Menus::on_preferences_response));
          
      preferences_dialog->run();
    }
}


//! Statistics Dialog.
void
Menus::on_menu_statistics()
{
  if (statistics_dialog == NULL)
    {
      Statistics *stats = Statistics::get_instance();
      stats->heartbeat();
      
      statistics_dialog = new StatisticsDialog();
      statistics_dialog->signal_response().connect(SigC::slot(*this, &Menus::on_statistics_response));
          
      statistics_dialog->run();
    }
}



//! About Dialog.
void
Menus::on_menu_about()
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
                    "Copyright 2001-2003 Rob Caelers & Raymond Penners",
                    _("This program assists in the prevention and recovery"
                      " of Repetitive Strain Injury (RSI)."),
                    (const gchar **) authors,
                    (const gchar **) NULL,
                    NULL,
                    pixbuf));
}


void
Menus::on_menu_network_join()
{
#ifdef HAVE_DISTRIBUTION
  GUIControl::OperationMode mode;
  GUIControl *gui_control = GUIControl::get_instance();
  mode = gui_control->set_operation_mode(GUIControl::OPERATION_MODE_QUIET);

  NetworkJoinDialog *dialog = new NetworkJoinDialog();
  dialog->run();
  delete dialog;

  gui_control->set_operation_mode(mode);
#endif
}


void
Menus::on_menu_network_leave()
{
#ifdef HAVE_DISTRIBUTION
  DistributionManager *dist_manager = DistributionManager::get_instance();
  if (dist_manager != NULL)
    {
      dist_manager->disconnect_all();
    }
#endif
}

void
Menus::on_menu_network_reconnect()
{
#ifdef HAVE_DISTRIBUTION
  DistributionManager *dist_manager = DistributionManager::get_instance();
  if (dist_manager != NULL)
    {
      dist_manager->reconnect_all();
    }
#endif
}

void
Menus::on_menu_network_log(bool active)
{
#ifdef HAVE_DISTRIBUTION
  TRACE_ENTER("Menus::on_menu_network_log");

  if (active)
    {
      if (network_log_dialog == NULL)
        {
          network_log_dialog = new NetworkLogDialog();
          network_log_dialog->signal_response().connect(SigC::slot(*this, &Menus::on_network_log_response));
          
          sync_tray_menu(active);
      
          network_log_dialog->run();
        }

    }
  else if (network_log_dialog != NULL)
    {
      network_log_dialog->hide_all();
      delete network_log_dialog;
      network_log_dialog = NULL;
      sync_tray_menu(active);
    }


  TRACE_EXIT();
#endif
}


#ifdef HAVE_DISTRIBUTION

void
Menus::on_menu_network_log_tray()
{
  TRACE_ENTER("Menus::on_menu_network_log_tray");

  if (tray_check_menus[3] != NULL)
    {
      bool active = tray_check_menus[3]->get_active();
      on_menu_network_log(active);
    }

  TRACE_EXIT();
}

void
Menus::on_menu_network_log_main_window()
{
  TRACE_ENTER("Menus::on_menu_network_log");

  if (main_window_check_menus[3] != NULL)
    {
      bool active = main_window_check_menus[3]->get_active();
      on_menu_network_log(active);
    }

  TRACE_EXIT();
}

void
Menus::on_network_log_response(int response)
{
  (void) response;

  assert(network_log_dialog != NULL);
  
  network_log_dialog->hide_all();

  sync_tray_menu(false);
  // done by gtkmm ??? delete network_log_dialog;
  network_log_dialog = NULL;
}

#endif

void
Menus::on_statistics_response(int response)
{
  (void) response;
  
  assert(statistics_dialog != NULL);
  statistics_dialog->hide_all();

  delete statistics_dialog;
  statistics_dialog = NULL;
}


void
Menus::on_preferences_response(int response)
{
  (void) response;
  
  assert(preferences_dialog != NULL);
  preferences_dialog->hide_all();

  GUIControl::get_instance()->get_configurator()->save();

  delete preferences_dialog;
  preferences_dialog = NULL;

#ifdef WIN32
  // FIXME: bug 130:
  // due to current Gtk+ behaviour of exit()'ing on WM_QUIT, we cannot
  // store main window position on shutdown (bug 130).
  // Therefore, this hack.

  MainWindow *window = gui->get_main_window();
  if (window != NULL)
    {
      window->win32_remember_position();
    }
#endif
  
}
