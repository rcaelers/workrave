// Menus.cc --- Timer info Window
//
// Copyright (C) 2001, 2002, 2003, 2004 Rob Caelers & Raymond Penners
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

#include <gtkmm/menu.h>
#include <gtkmm/stock.h>

#include "Menus.hh"
#include "TimeBar.hh"
#include "GUI.hh"
#include "Util.hh"
#include "Text.hh"

#include "PreferencesDialog.hh"
#include "StatisticsDialog.hh"
#include "StatisticsInterface.hh"

#include "CoreFactory.hh"
#include "CoreInterface.hh"
#include "TimerInterface.hh"
#include "ConfiguratorInterface.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManagerInterface.hh"
#include "NetworkJoinDialog.hh"
#include "NetworkLogDialog.hh"
#endif
#ifdef HAVE_GNOME
#include "AppletWindow.hh"
#else
#include "gnome-about.h"
#endif
#include "MainWindow.hh"

#ifdef HAVE_EXERCISES
#include "ExercisesDialog.hh"
#include "Exercise.hh"
#endif

#ifdef WIN32
#include <gdk/gdkwin32.h>
#include "TimerBoxAppletView.hh"
#endif

Menus *Menus::instance = NULL;

#ifdef WIN32
enum
{
  MENU_COMMAND_PREFERENCES,
  MENU_COMMAND_EXERCISES,
  MENU_COMMAND_REST_BREAK,
  MENU_COMMAND_MODE_NORMAL,
  MENU_COMMAND_MODE_QUIET,
  MENU_COMMAND_MODE_SUSPENDED,
  MENU_COMMAND_NETWORK_CONNECT,
  MENU_COMMAND_NETWORK_DISCONNECT,
  MENU_COMMAND_NETWORK_LOG,
  MENU_COMMAND_NETWORK_RECONNECT,
  MENU_COMMAND_STATISTICS,
  MENU_COMMAND_ABOUT
};
#endif

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
  network_join_dialog(NULL),
#endif
  statistics_dialog(NULL),
  preferences_dialog(NULL),
#ifdef HAVE_EXERCISES
  exercises_dialog(NULL),
#endif
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
#ifndef WIN32
  if (check_menus == tray_check_menus)
    {
      menulist.push_front(Gtk::Menu_Helpers::StockMenuElem
                          (Gtk::Stock::OPEN,
                           SigC::slot(*this, &Menus::on_menu_open_main_window)));
    }
#endif
  
  menulist.push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::PREFERENCES,
                                                      SigC::slot(*this, &Menus::on_menu_preferences)));


  // Rest break
  string rb_icon = Util::complete_directory("timer-rest-break.png", Util::SEARCH_PATH_IMAGES);
  Gtk::Image *img = manage(new Gtk::Image(rb_icon));
  menulist.push_back(Gtk::Menu_Helpers::ImageMenuElem
                     (_("_Rest break"),
                      Gtk::Menu::AccelKey("<control>r"),
                      *img,
                      SigC::slot(*this, &Menus::on_menu_restbreak_now)));

#ifdef HAVE_EXERCISES
  // Exercises
  if (Exercise::has_exercises())
    {
      menulist.push_back(Gtk::Menu_Helpers::MenuElem
                         (_("Exercises"), 
                          SigC::slot(*this, &Menus::on_menu_exercises)));
    }
#endif  
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
  
#if defined(HAVE_GNOME)
  if (applet_window != NULL)
    {
      applet_window->set_menu_active(mode, true);
    }
#elif defined(WIN32)
  resync_applet();
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
  
#if defined(HAVE_GNOME)
  if (applet_window != NULL)
    {
      applet_window->set_menu_active(3, active);
    }
#elif defined(WIN32)
  resync_applet();
#endif
  
  syncing = false;

  TRACE_EXIT();
}


void
Menus::resync_applet()
{
#if defined(HAVE_GNOME)
  if (applet_window != NULL)
    {
      CoreInterface *core = CoreFactory::get_core();
      OperationMode mode = core->get_operation_mode();      
      switch(mode)
        {
        case OPERATION_MODE_NORMAL:
          applet_window->set_menu_active(0, true);
          break;
        case OPERATION_MODE_SUSPENDED:
          applet_window->set_menu_active(1, true);
          break;
        case OPERATION_MODE_QUIET:
          applet_window->set_menu_active(2, true);
          break;
        default:
          break;
        }
#if defined(HAVE_DISTRIBUTION)
      applet_window->set_menu_active(3, network_log_dialog != NULL);
#endif
    }
#elif defined(WIN32)
  if (applet_window != NULL && main_window != NULL )
    {
      CoreInterface *core = CoreFactory::get_core();

      HWND cmd_win = (HWND) GDK_WINDOW_HWND( main_window
                                             ->Gtk::Widget::gobj()->window);
      applet_window->init_menu(cmd_win);

      applet_window->add_menu(_("Preferences"), MENU_COMMAND_PREFERENCES, 0);
      applet_window->add_menu(_("_Rest break"), MENU_COMMAND_REST_BREAK, 0);
      applet_window->add_menu(_("Exercises"), MENU_COMMAND_EXERCISES, 0);


      applet_window->add_menu(_("_Normal"), MENU_COMMAND_MODE_NORMAL,
                              TimerBoxAppletView::MENU_FLAG_TOGGLE
                              |TimerBoxAppletView::MENU_FLAG_POPUP
                              |(core->get_operation_mode()
                                == OPERATION_MODE_NORMAL
                                ? TimerBoxAppletView::MENU_FLAG_SELECTED
                                : 0));
      applet_window->add_menu(_("_Suspended"), MENU_COMMAND_MODE_SUSPENDED,
                              TimerBoxAppletView::MENU_FLAG_TOGGLE
                              |TimerBoxAppletView::MENU_FLAG_POPUP
                              |(core->get_operation_mode()
                                == OPERATION_MODE_SUSPENDED
                                ? TimerBoxAppletView::MENU_FLAG_SELECTED
                                : 0));
      applet_window->add_menu(_("Q_uiet"), MENU_COMMAND_MODE_QUIET,
                              TimerBoxAppletView::MENU_FLAG_TOGGLE
                              |TimerBoxAppletView::MENU_FLAG_POPUP
                              |(core->get_operation_mode()
                                == OPERATION_MODE_QUIET
                                ? TimerBoxAppletView::MENU_FLAG_SELECTED
                                : 0));
      applet_window->add_menu(_("_Mode"), 0, 0);

#ifdef HAVE_DISTRIBUTION
      applet_window->add_menu(_("_Connect"), MENU_COMMAND_NETWORK_CONNECT,
                              TimerBoxAppletView::MENU_FLAG_TOGGLE
                              |TimerBoxAppletView::MENU_FLAG_POPUP);
      applet_window->add_menu(_("_Disconnect"),
                              MENU_COMMAND_NETWORK_DISCONNECT,
                              TimerBoxAppletView::MENU_FLAG_TOGGLE
                              |TimerBoxAppletView::MENU_FLAG_POPUP);
      applet_window->add_menu(_("_Reconnect"), MENU_COMMAND_NETWORK_RECONNECT,
                              TimerBoxAppletView::MENU_FLAG_TOGGLE
                              |TimerBoxAppletView::MENU_FLAG_POPUP);
      applet_window->add_menu(_("Show _log"), MENU_COMMAND_NETWORK_LOG,
                              TimerBoxAppletView::MENU_FLAG_TOGGLE
                              |TimerBoxAppletView::MENU_FLAG_POPUP
                              |(network_log_dialog != NULL
                                ? TimerBoxAppletView::MENU_FLAG_SELECTED
                                : 0));
      applet_window->add_menu(_("_Network"), 0, 0);
#endif
      applet_window->add_menu(_("Statistics"), MENU_COMMAND_STATISTICS, 0);
      applet_window->add_menu(_("About..."), MENU_COMMAND_ABOUT, 0);
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

  CoreInterface *core = CoreFactory::get_core();
  core->set_operation_mode(OPERATION_MODE_QUIET);
  sync_mode_menu(2);
  TRACE_EXIT();
}



//! User requested immediate restbreak.
void
Menus::on_menu_suspend()
{
  TRACE_ENTER("Menus::on_menu_suspend");

  CoreInterface *core = CoreFactory::get_core();
  core->set_operation_mode(OPERATION_MODE_SUSPENDED);
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
  CoreInterface *core = CoreFactory::get_core();
  core->set_operation_mode(OPERATION_MODE_NORMAL);
  sync_mode_menu(0);
  TRACE_EXIT();
}


#ifndef NDEBUG
//! User test code.
void
Menus::on_test_me()
{ 
  CoreInterface *core = CoreFactory::get_core();
  StatisticsInterface *stats = core->get_statistics();
  stats->dump();

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
  else
    {
      preferences_dialog->present();
    }
}

#ifdef HAVE_EXERCISES
//! Exercises Dialog.
void
Menus::on_menu_exercises()
{
  TRACE_ENTER("Menus::on_menu_exercises");
  if (exercises_dialog == NULL)
    {
      exercises_dialog = new ExercisesDialog();
      exercises_dialog->signal_response().connect(SigC::slot(*this, &Menus::on_exercises_response));
          
      exercises_dialog->run();
    }
  else
    {
      exercises_dialog->present();
    }
  TRACE_EXIT();
}

void
Menus::on_exercises_response(int response)
{
  (void) response;
  
  assert(exercises_dialog != NULL);
  exercises_dialog->hide_all();

  delete exercises_dialog;
  exercises_dialog = NULL;
}

#endif


//! Statistics Dialog.
void
Menus::on_menu_statistics()
{
  if (statistics_dialog == NULL)
    {
      CoreInterface *core = CoreFactory::get_core();
      StatisticsInterface *stats = core->get_statistics();
      stats->update();
      
      statistics_dialog = new StatisticsDialog();
      statistics_dialog->signal_response().connect(SigC::slot(*this, &Menus::on_statistics_response));
          
      statistics_dialog->run();
    }
  else
    {
      statistics_dialog->present();
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
  const gchar *translators = 
    "Raymond Penners <raymond@dotsphinx.com>\n"
    "Johannes Rohr <j.rohr@comlink.apc.org>\n"
    "Christian Vejlbo <christian@vejlbo.dk>\n"
    "Mikolaj Machowski <mikmach@wp.pl>\n"
    "Pablo Rodriguez\n"
    "Rex Tsai <chihchun@linux.org.tw>\n";
  string icon = Util::complete_directory("workrave.png",
                                         Util::SEARCH_PATH_IMAGES);
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(icon.c_str(), NULL); 
  gtk_widget_show (gnome_about_new
                   ("Workrave", VERSION,
                    "Copyright 2001-2004 Rob Caelers & Raymond Penners",
                    _("This program assists in the prevention and recovery"
                      " of Repetitive Strain Injury (RSI)."),
                    (const gchar **) authors,
                    (const gchar **) NULL,
                    translators,
                    pixbuf));
  g_object_unref(pixbuf);
}


void
Menus::on_menu_network_join()
{
#ifdef HAVE_DISTRIBUTION
  if (network_join_dialog == NULL)
    {
      network_join_dialog = new NetworkJoinDialog();
      network_join_dialog->signal_response().connect(SigC::slot(*this, &Menus::on_network_join_response));
      network_join_dialog->run();
    }
  else
    {
      network_join_dialog->present();
    }
#endif
}

#ifdef HAVE_DISTRIBUTION
void
Menus::on_network_join_response(int response)
{
  (void) response;
  
  assert(network_join_dialog != NULL);
  network_join_dialog->hide_all();

  if (response == Gtk::RESPONSE_OK)
    {
      CoreInterface *core = CoreFactory::get_core();
      DistributionManagerInterface *dist_manager
        = core->get_distribution_manager();
      std::string peer = network_join_dialog->get_connect_url();
      if (network_join_dialog->is_connect_at_startup_selected())
        {
          dist_manager->add_peer(peer);
        }
      else
        {
          dist_manager->connect(peer);
        }
      CoreFactory::get_configurator()->save();
    }

  delete network_join_dialog;
  network_join_dialog = NULL;
}
#endif

void
Menus::on_menu_network_leave()
{
#ifdef HAVE_DISTRIBUTION
  CoreInterface *core = CoreFactory::get_core();
  DistributionManagerInterface *dist_manager = core->get_distribution_manager();
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
  CoreInterface *core = CoreFactory::get_core();
  DistributionManagerInterface *dist_manager = core->get_distribution_manager();
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

  CoreFactory::get_configurator()->save();

  delete preferences_dialog;
  preferences_dialog = NULL;
}

#ifdef WIN32

void
Menus::on_applet_command(short cmd)
{
  switch (cmd)
    {
    case MENU_COMMAND_PREFERENCES:
      on_menu_preferences();
      break;
    case MENU_COMMAND_EXERCISES:
      on_menu_exercises();
      break;
    case MENU_COMMAND_REST_BREAK:
      on_menu_restbreak_now();
      break;
    case MENU_COMMAND_MODE_NORMAL:
      on_menu_normal();
      break;
    case MENU_COMMAND_MODE_QUIET:
      on_menu_quiet();
      break;
    case MENU_COMMAND_MODE_SUSPENDED:
      on_menu_suspend();
      break;
    case MENU_COMMAND_NETWORK_CONNECT:
      on_menu_network_join();
      break;
    case MENU_COMMAND_NETWORK_DISCONNECT:
      on_menu_network_leave();
      break;
    case MENU_COMMAND_NETWORK_LOG:
      on_menu_network_log(network_log_dialog==NULL);
      break;
    case MENU_COMMAND_NETWORK_RECONNECT:
      on_menu_network_reconnect();
      break;
    case MENU_COMMAND_STATISTICS:
      on_menu_statistics();
      break;
    case MENU_COMMAND_ABOUT:
      on_menu_about();
      break;
    }
}

#endif
