// Menus.cc --- Timer info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
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
#include "credits.h"

#include <unistd.h>
#include <iostream>

#include <gtk/gtkmenu.h>
#include <gtkmm/menu.h>
#include <gtkmm/stock.h>
#include <gtkmm/aboutdialog.h>

#include "Menus.hh"
#include "TimeBar.hh"
#include "GUI.hh"
#include "Util.hh"
#include "Text.hh"

#include "PreferencesDialog.hh"
#include "StatisticsDialog.hh"
#include "IStatistics.hh"

#include "CoreFactory.hh"
#include "ICore.hh"
#include "ITimer.hh"
#include "IConfigurator.hh"

#ifdef HAVE_DISTRIBUTION
#include "IDistributionManager.hh"
#include "NetworkJoinDialog.hh"
#include "NetworkLogDialog.hh"
#endif
#ifdef HAVE_GNOME
#include "GnomeAppletWindow.hh"
#endif
#include "MainWindow.hh"

#ifdef HAVE_EXERCISES
#include "ExercisesDialog.hh"
#include "Exercise.hh"
#endif

#ifdef WIN32
#include <gdk/gdkwin32.h>
#include "W32AppletWindow.hh"
#endif

Menus *Menus::instance = 0;

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
#if defined(HAVE_GNOME) || defined(WIN32)
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
  assert(instance == 0);
  instance = this;
  gui = GUI::get_instance();

  for (int k = 0; k < MENU_SIZEOF; k++)
    {
      menus[k] = NULL;
      for (int i = 0; i < MENUSYNC_SIZEOF; i++)
        {
          sync_menus[k][i] = NULL;
        }
    }
}


//! Destructor.
Menus::~Menus()
{
  TRACE_ENTER("Menus::~Menus");
  instance = 0;

  for (int k = 0; k < MENU_SIZEOF; k++)
    {
      delete menus[k];
      menus[k] = NULL;
    }
  
  TRACE_EXIT();
}


Menus *
Menus::get_instance()
{
  assert(instance != 0);
  return instance;
}


void
Menus::create_menu(MenuKind kind)
{
  if (! menus[kind])
    {
      menus[kind] = manage(create_menu(kind, sync_menus[kind]));

#ifdef HAVE_DISTRIBUTION

      switch (kind)
        {
        case MENU_MAINWINDOW:
          sync_menus[kind][MENUSYNC_SHOW_LOG]->signal_toggled().
            connect(MEMBER_SLOT(*this, &Menus::on_menu_network_log_main_window));
          break;
      
        case MENU_APPLET:
          sync_menus[kind][MENUSYNC_SHOW_LOG]->signal_toggled().
            connect(MEMBER_SLOT(*this, &Menus::on_menu_network_log_tray));
          break;

        default:
          break;
        }
#endif
      if (kind == MENU_APPLET)
        {
          resync_applet();
        }
    }
}

void
Menus::popup(const MenuKind kind,
             const guint button,
             const guint activate_time)
{
  Gtk::Menu *pop_menu = menus[kind];
  if (pop_menu)
    {
      pop_menu->popup(button, activate_time);
    }
}

//! Create the popup-menu
Gtk::Menu *
Menus::create_menu(MenuKind kind, Gtk::CheckMenuItem *check_menus[MENUSYNC_SIZEOF])
{
  TRACE_ENTER_MSG("Menus::create_menu", kind);
  Gtk::Menu *pop_menu = manage(new Gtk::Menu());
  
  Gtk::Menu::MenuList &menulist = pop_menu->items();

  Gtk::Menu *mode_menu = manage(new Gtk::Menu());
  Gtk::Menu::MenuList &modemenulist = mode_menu->items();

  // Mode menu item
  Gtk::MenuItem *mode_menu_item = manage(new Gtk::MenuItem(_("_Mode"),true)); // FIXME: LEAK
  mode_menu_item->set_submenu(*mode_menu);
  mode_menu_item->show();


  // Operation mode
  {
    const char *modes_str[] =
      {
        "_Normal",
        "_Suspended",
        "Q_uiet",
      };
    OperationMode modes[] =
      {
        OPERATION_MODE_NORMAL,
        OPERATION_MODE_SUSPENDED,
        OPERATION_MODE_QUIET
      };
    typedef void (Menus::*ModeFunc)(Gtk::CheckMenuItem *);
    ModeFunc modes_func[] =
      {
        &Menus::on_menu_normal_menu,
        &Menus::on_menu_suspend_menu,
        &Menus::on_menu_quiet_menu
      };
    
    ICore *core = CoreFactory::get_core();
    OperationMode mode = core->get_operation_mode();      

    Gtk::RadioMenuItem::Group gr;

    assert(sizeof(modes)/sizeof(modes[0]) == OPERATION_MODE_SIZEOF);
    for (size_t i = 0; i < OPERATION_MODE_SIZEOF; i++)
      {
        Gtk::RadioMenuItem *mode_menu_item
          = manage(new Gtk::RadioMenuItem(gr, _(modes_str[i]), true));
        mode_menu_item->show();
        modemenulist.push_back(*mode_menu_item);
        mode_menu_item->set_active(mode == modes[i]);
        check_menus[i] = mode_menu_item;
      }
    for (size_t i = 0; i < OPERATION_MODE_SIZEOF; i++)
      {
        check_menus[i]->signal_toggled()
          .connect(bind(MEMBER_SLOT(*this, modes_func[i]),
                        check_menus[i]));
      }
  }

#ifdef HAVE_DISTRIBUTION
  // Distribution menu
  Gtk::Menu *distr_menu = manage(new Gtk::Menu());
  Gtk::Menu::MenuList &distr_menu_list = distr_menu->items();

  Gtk::MenuItem *distr_menu_item = manage(new Gtk::MenuItem(_("_Network"),true));
  distr_menu_item->set_submenu(*distr_menu);
  distr_menu_item->show();

  Gtk::MenuItem *distr_join_menu_item = manage(new Gtk::MenuItem(_("_Connect"),true));
  distr_join_menu_item->show();
  distr_join_menu_item->signal_activate().connect(MEMBER_SLOT(*this, &Menus::on_menu_network_join));
  distr_menu_list.push_back(*distr_join_menu_item);
  
  Gtk::MenuItem *distr_leave_menu_item = manage(new Gtk::MenuItem(_("_Disconnect"),true));
  distr_leave_menu_item->show();
  distr_leave_menu_item->signal_activate().connect(MEMBER_SLOT(*this, &Menus::on_menu_network_leave));
  distr_menu_list.push_back(*distr_leave_menu_item);

  Gtk::MenuItem *distr_reconnect_menu_item = manage(new Gtk::MenuItem(_("_Reconnect"),true));
  distr_reconnect_menu_item->show();
  distr_reconnect_menu_item->signal_activate().connect(MEMBER_SLOT(*this, &Menus::on_menu_network_reconnect));
  distr_menu_list.push_back(*distr_reconnect_menu_item);

  Gtk::CheckMenuItem *distr_log_menu_item = manage(new Gtk::CheckMenuItem(_("Show _log"), true));
  distr_log_menu_item->show();
  distr_menu_list.push_back(*distr_log_menu_item);

  check_menus[MENUSYNC_SHOW_LOG] = distr_log_menu_item;
#endif
  
  // FIXME: add separators, etc...
  if (kind == MENU_APPLET)
    {
      menulist.push_front(Gtk::Menu_Helpers::StockMenuElem
                          (Gtk::Stock::OPEN,
                           MEMBER_SLOT(*this, &Menus::on_menu_open_main_window)));
    }
  
  menulist.push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::PREFERENCES,
                                                      MEMBER_SLOT(*this, &Menus::on_menu_preferences)));


  // Rest break
  string rb_icon = Util::complete_directory("timer-rest-break.png", Util::SEARCH_PATH_IMAGES);
  Gtk::Image *img = manage(new Gtk::Image(rb_icon));
  menulist.push_back(Gtk::Menu_Helpers::ImageMenuElem
                     (_("_Rest break"),
#ifdef HAVE_GTKMM24
                      Gtk::AccelKey("<control>r"),
#else
                      Gtk::Menu::AccelKey("<control>r"),
#endif
                      *img,
                      MEMBER_SLOT(*this, &Menus::on_menu_restbreak_now)));

#ifdef HAVE_EXERCISES
  // Exercises
  if (Exercise::has_exercises())
    {
      menulist.push_back(Gtk::Menu_Helpers::MenuElem
                         (_("Exercises"), 
                          MEMBER_SLOT(*this, &Menus::on_menu_exercises)));
    }
#endif  
  menulist.push_back(*mode_menu_item);

  

#ifdef HAVE_DISTRIBUTION
  menulist.push_back(*distr_menu_item);
#endif
  
  menulist.push_back(Gtk::Menu_Helpers::MenuElem(_("Statistics"), 
                                                 MEMBER_SLOT(*this, &Menus::on_menu_statistics)));
  
#ifndef NDEBUG
  menulist.push_back(Gtk::Menu_Helpers::MenuElem("_Test",
                                                 MEMBER_SLOT(*this, &Menus::on_test_me)));
#endif
  
#ifdef HAVE_GNOME
  menulist.push_back(Gtk::Menu_Helpers::StockMenuElem
		     (Gtk::StockID(GNOME_STOCK_ABOUT),
		      MEMBER_SLOT(*this, &Menus::on_menu_about)));
#else
  menulist.push_back(Gtk::Menu_Helpers::MenuElem
		     (_("About..."),
		      MEMBER_SLOT(*this, &Menus::on_menu_about)));
#endif

  menulist.push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::QUIT,
                                                 MEMBER_SLOT(*this, &Menus::on_menu_quit)));


#ifdef WIN32
  if (kind == MENU_APPLET)
    {
      win32_popup_hack_connect(pop_menu);
    }
#endif  

  TRACE_EXIT();
  return pop_menu;
}


void
Menus::sync_mode_menu(int mode)
{
  // Ugh, isn't there an other way to prevent endless signal loops?
  static bool syncing = false;

  TRACE_ENTER_MSG("Menus::sync_mode_menu", mode << " " << syncing);
  if (syncing)
    return;
  syncing = true;

  for (int k = MENU_MAINWINDOW; k < MENU_SIZEOF; k++)
    {
      if (sync_menus[k][mode] != NULL && !sync_menus[k][mode]->get_active())
        {
          TRACE_MSG("setting active " << k <<" " << mode);
          sync_menus[k][mode]->set_active(true);
        }
 
    }
  
#if defined(HAVE_GNOME)
  GnomeAppletWindow *aw = static_cast<GnomeAppletWindow*>(applet_window);
  if (aw != NULL)
    {
      TRACE_MSG("setting gnome active " << mode);
      aw->set_menu_active(mode, true);
    }
#elif defined(WIN32)
  resync_applet();
#endif
  
  syncing = false;

  TRACE_EXIT();
}

void
Menus::sync_log_menu(bool active)
{
  TRACE_ENTER("Menus::sync_log_menu");

  static bool syncing = false;
  if (syncing)
    return;
  syncing = true;

  for (int k = 0; k < MENU_SIZEOF; k++)
    {
      if (sync_menus[k][MENUSYNC_SHOW_LOG] != NULL)
        {
          sync_menus[k][MENUSYNC_SHOW_LOG]->set_active(active);
        }
    }
  
#if defined(HAVE_GNOME)
  GnomeAppletWindow *aw = static_cast<GnomeAppletWindow*>(applet_window);
  if (aw != NULL)
    {
      aw->set_menu_active(MENUSYNC_SHOW_LOG, active);
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
  ICore *core = CoreFactory::get_core();
  OperationMode mode = core->get_operation_mode();
  
  for (int k = MENU_MAINWINDOW; k < MENU_SIZEOF; k++)
    {
      if (sync_menus[k][mode] != NULL && !sync_menus[k][mode]->get_active())
        {
          sync_menus[k][mode]->set_active(true);
        }
    }
  
#if defined(HAVE_DISTRIBUTION)
  bool network_log_active = network_log_dialog != NULL;
  for (int k = 0; k < MENU_SIZEOF; k++)
    {
      if (sync_menus[k][MENUSYNC_SHOW_LOG] != NULL)
        {
          sync_menus[k][MENUSYNC_SHOW_LOG]->set_active(network_log_active);
        }
    }
#endif
      
#if defined(HAVE_GNOME)
  GnomeAppletWindow *aw = static_cast<GnomeAppletWindow*>(applet_window);
  if (aw != NULL)
    {
      switch(mode)
        {
        case OPERATION_MODE_NORMAL:
          aw->set_menu_active(MENUSYNC_MODE_NORMAL, true);
          break;
        case OPERATION_MODE_SUSPENDED:
          aw->set_menu_active(MENUSYNC_MODE_SUSPENDED, true);
          break;
        case OPERATION_MODE_QUIET:
          aw->set_menu_active(MENUSYNC_MODE_QUIET, true);
          break;
        default:
          break;
        }
#if defined(HAVE_DISTRIBUTION)
      bool network_log_active = network_log_dialog != NULL;
      aw->set_menu_active(MENUSYNC_SHOW_LOG, network_log_dialog);
      
#endif
    }

#elif defined(WIN32)
  if (applet_window != NULL && main_window != NULL)
    {
      HWND cmd_win = (HWND) GDK_WINDOW_HWND( main_window
                                             ->Gtk::Widget::gobj()->window);
      W32AppletWindow *w32aw = static_cast<W32AppletWindow*>(applet_window);
      w32aw->init_menu(cmd_win);

      w32aw->add_menu(_("Preferences"), MENU_COMMAND_PREFERENCES, 0);
      w32aw->add_menu(_("_Rest break"), MENU_COMMAND_REST_BREAK, 0);
      w32aw->add_menu(_("Exercises"), MENU_COMMAND_EXERCISES, 0);


      w32aw->add_menu(_("_Normal"), MENU_COMMAND_MODE_NORMAL,
                      W32AppletWindow::MENU_FLAG_TOGGLE
                      |W32AppletWindow::MENU_FLAG_POPUP
                      |(core->get_operation_mode()
                        == OPERATION_MODE_NORMAL
                        ? W32AppletWindow::MENU_FLAG_SELECTED
                        : 0));
      w32aw->add_menu(_("_Suspended"), MENU_COMMAND_MODE_SUSPENDED,
                      W32AppletWindow::MENU_FLAG_TOGGLE
                      |W32AppletWindow::MENU_FLAG_POPUP
                      |(core->get_operation_mode()
                        == OPERATION_MODE_SUSPENDED
                        ? W32AppletWindow::MENU_FLAG_SELECTED
                        : 0));
      w32aw->add_menu(_("Q_uiet"), MENU_COMMAND_MODE_QUIET,
                      W32AppletWindow::MENU_FLAG_TOGGLE
                      |W32AppletWindow::MENU_FLAG_POPUP
                      |(core->get_operation_mode()
                        == OPERATION_MODE_QUIET
                        ? W32AppletWindow::MENU_FLAG_SELECTED
                        : 0));
      w32aw->add_menu(_("_Mode"), 0, 0);

#ifdef HAVE_DISTRIBUTION
      w32aw->add_menu(_("_Connect"), MENU_COMMAND_NETWORK_CONNECT,
                      W32AppletWindow::MENU_FLAG_TOGGLE
                      |W32AppletWindow::MENU_FLAG_POPUP);
      w32aw->add_menu(_("_Disconnect"),
                      MENU_COMMAND_NETWORK_DISCONNECT,
                      W32AppletWindow::MENU_FLAG_TOGGLE
                      |W32AppletWindow::MENU_FLAG_POPUP);
      w32aw->add_menu(_("_Reconnect"), MENU_COMMAND_NETWORK_RECONNECT,
                      W32AppletWindow::MENU_FLAG_TOGGLE
                      |W32AppletWindow::MENU_FLAG_POPUP);
      w32aw->add_menu(_("Show _log"), MENU_COMMAND_NETWORK_LOG,
                      W32AppletWindow::MENU_FLAG_TOGGLE
                      |W32AppletWindow::MENU_FLAG_POPUP
                      |(network_log_dialog != NULL
                        ? W32AppletWindow::MENU_FLAG_SELECTED
                        : 0));
      w32aw->add_menu(_("_Network"), 0, 0);
#endif
      w32aw->add_menu(_("Statistics"), MENU_COMMAND_STATISTICS, 0);
      w32aw->add_menu(_("About..."), MENU_COMMAND_ABOUT, 0);
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


void
Menus::set_operation_mode(OperationMode m)
{
  ICore *core = CoreFactory::get_core();
  core->set_operation_mode(m);
  sync_mode_menu(m);
}


//! User requested immediate restbreak.
void
Menus::on_menu_quiet()
{
  TRACE_ENTER("Menus::on_menu_quiet");
  set_operation_mode(OPERATION_MODE_QUIET);
  TRACE_EXIT();
}



//! User requested immediate restbreak.
void
Menus::on_menu_suspend()
{
  TRACE_ENTER("Menus::on_menu_suspend");
  set_operation_mode(OPERATION_MODE_SUSPENDED);
  TRACE_EXIT();
}


void
Menus::on_menu_normal_menu(Gtk::CheckMenuItem *menu)
{
  TRACE_ENTER("Menus::on_menu_normal_menu");
  if (menu != NULL && menu->get_active())
    {
      TRACE_MSG("active");
      on_menu_normal();
    }
  TRACE_EXIT();
}


//! User requested immediate restbreak.
void
Menus::on_menu_quiet_menu(Gtk::CheckMenuItem *menu)
{
  TRACE_ENTER("Menus::on_menu_quiet_menu");
  
  if (menu != NULL && menu->get_active())
    {
      TRACE_MSG("active");
      on_menu_quiet();
    }
  TRACE_EXIT();
}



//! User requested immediate restbreak.
void
Menus::on_menu_suspend_menu(Gtk::CheckMenuItem *menu)
{
  TRACE_ENTER("Menus::on_menu_suspend_menu");
  
  if (menu != NULL && menu->get_active())
    {
      TRACE_MSG("active");
      on_menu_suspend();
    }
  
  TRACE_EXIT();
}


void
Menus::on_menu_normal()
{
  TRACE_ENTER("Menus::on_menu_normal");
  set_operation_mode(OPERATION_MODE_NORMAL);
  TRACE_EXIT();
}


#ifndef NDEBUG
//! User test code.
void
Menus::on_test_me()
{ 
  ICore *core = CoreFactory::get_core();
  IStatistics *stats = core->get_statistics();
  stats->dump();
  
  core->test_me();
}
#endif


//! Preferences Dialog.
void
Menus::on_menu_preferences()
{
  if (preferences_dialog == NULL)
    {
      preferences_dialog = new PreferencesDialog();
      preferences_dialog->signal_response().connect(MEMBER_SLOT(*this, &Menus::on_preferences_response));
      
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
      exercises_dialog->signal_response().connect(MEMBER_SLOT(*this, &Menus::on_exercises_response));
      
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
      ICore *core = CoreFactory::get_core();
      IStatistics *stats = core->get_statistics();
      stats->update();
      
      statistics_dialog = new StatisticsDialog();
      statistics_dialog->signal_response().connect(MEMBER_SLOT(*this, &Menus::on_statistics_response));
      
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
  string icon = Util::complete_directory("workrave.png",
                                         Util::SEARCH_PATH_IMAGES);
  Glib::RefPtr<Gdk::Pixbuf> pixbuf;
  
  try
    {
      pixbuf = Gdk::Pixbuf::create_from_file(icon);
    }
  catch (...)
    {
    }
  
  Gtk::AboutDialog about;

  about.set_name("Workrave");
  about.set_authors(workrave_authors);
  about.set_copyright(workrave_copyright);
  about.set_comments(_("This program assists in the prevention and recovery"
                       " of Repetitive Strain Injury (RSI)."));
  about.set_logo(pixbuf);
  about.set_translator_credits(workrave_translators);
  about.set_version(VERSION);
  about.set_website("http://www.workrave.org/");
  about.set_website_label("www.workrave.org");

  about.run();
}


void
Menus::on_menu_network_join()
{
#ifdef HAVE_DISTRIBUTION
  if (network_join_dialog == NULL)
    {
      network_join_dialog = new NetworkJoinDialog();
      network_join_dialog->signal_response().connect(MEMBER_SLOT(*this, &Menus::on_network_join_response));
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
      ICore *core = CoreFactory::get_core();
      IDistributionManager *dist_manager
        = core->get_distribution_manager();
      std::string peer = network_join_dialog->get_connect_url();
      dist_manager->connect(peer);
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
  ICore *core = CoreFactory::get_core();
  IDistributionManager *dist_manager = core->get_distribution_manager();
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
  ICore *core = CoreFactory::get_core();
  IDistributionManager *dist_manager = core->get_distribution_manager();
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
          network_log_dialog->signal_response().
            connect(MEMBER_SLOT(*this, &Menus::on_network_log_response));
          
          sync_log_menu(active);
          
          network_log_dialog->run();
        }
    }
  else if (network_log_dialog != NULL)
    {
      network_log_dialog->hide_all();
      delete network_log_dialog;
      network_log_dialog = NULL;
      sync_log_menu(active);
    }
  
  
  TRACE_EXIT();
#endif
}


#ifdef HAVE_DISTRIBUTION

void
Menus::on_menu_network_log_tray()
{
  TRACE_ENTER("Menus::on_menu_network_log_tray");
  
  if (sync_menus[MENU_APPLET][MENUSYNC_SHOW_LOG] != NULL)
    {
      bool active = sync_menus[MENU_APPLET][MENUSYNC_SHOW_LOG]->get_active();
      on_menu_network_log(active);
    }
  
  TRACE_EXIT();
}

void
Menus::on_menu_network_log_main_window()
{
  TRACE_ENTER("Menus::on_menu_network_log");
  
  if (sync_menus[MENU_MAINWINDOW][MENUSYNC_SHOW_LOG] != NULL)
    {
      bool active = sync_menus[MENU_MAINWINDOW][MENUSYNC_SHOW_LOG]->get_active();
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
  
  sync_log_menu(false);
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

#if defined(HAVE_GNOME) || defined(WIN32)
void
Menus::set_applet_window(AppletWindow *applet)
{
  applet_window = applet;
  resync_applet();
}
#endif


#ifdef WIN32
// /* Taken from Gaim. needs to be gtkmm-ified. */
// /* This is a workaround for a bug in windows GTK+. Clicking outside of the
//    menu does not get rid of it, so instead we get rid of it as soon as the
//    pointer leaves the menu. */

void
Menus::win32_popup_hack_connect(Gtk::Menu *menu)
{
  GtkWidget *window = (GtkWidget*) menu->gobj();
  // RC: FIXME: remove this c hack HACK 
  g_signal_connect(window, "leave-notify-event",
                   G_CALLBACK(win32_popup_hack_leave_enter), NULL);
  g_signal_connect(window, "enter-notify-event",
                   G_CALLBACK(win32_popup_hack_leave_enter), NULL);
}

gboolean 
Menus::win32_popup_hack_hide(gpointer data)
{
  if (data != NULL)
    {
      gtk_menu_popdown(GTK_MENU(data));
    }
  return FALSE;
}


gboolean
Menus::win32_popup_hack_leave_enter(GtkWidget *menu, GdkEventCrossing *event,
                                    void *data)
{
  (void) data;
  static guint hide_docklet_timer = 0;
  if (event->type == GDK_LEAVE_NOTIFY
      && event->detail == GDK_NOTIFY_ANCESTOR) {
    /* Add some slop so that the menu doesn't annoyingly disappear
       when mousing around */
    if (hide_docklet_timer == 0) {
      hide_docklet_timer = g_timeout_add(500, win32_popup_hack_hide, menu);
    }
  } else if (event->type == GDK_ENTER_NOTIFY
             && event->detail == GDK_NOTIFY_ANCESTOR) {
    if (hide_docklet_timer != 0) {
      /* Cancel the hiding if we reenter */
      
      g_source_remove(hide_docklet_timer);
      hide_docklet_timer = 0;
    }
  }
  return FALSE;
}

#endif // WIN32
