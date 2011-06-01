// Menus.cc --- Timer info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"
#include "credits.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <iostream>

#include "Menus.hh"
#include "GUI.hh"
#include "Util.hh"

#include "PreferencesDialog.hh"
#include "StatisticsDialog.hh"
#include "IStatistics.hh"

#include "CoreFactory.hh"
#include "ICore.hh"
#include "IConfigurator.hh"

#include "MainWindow.hh"

#ifdef HAVE_DISTRIBUTION
#include "IDistributionManager.hh"
#include "NetworkJoinDialog.hh"
#include "NetworkLogDialog.hh"
#endif

#ifdef HAVE_EXERCISES
#include "ExercisesDialog.hh"
#include "Exercise.hh"
#endif

#include "MainGtkMenu.hh"

#ifdef HAVE_GNOMEAPPLET
#include "GnomeAppletMenu.hh"
#include "GnomeAppletWindow.hh"
#endif

#ifdef PLATFORM_OS_WIN32
#include <shellapi.h>

#include "W32AppletWindow.hh"
#include "W32TrayMenu.hh"
#include "W32AppletMenu.hh"
#endif

#if defined(PLATFORM_OS_OSX)
#include "OSXGtkMenu.hh"
#endif

Menus *Menus::instance = 0;

//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
Menus::Menus() :
#ifdef HAVE_DISTRIBUTION
  network_log_dialog(NULL),
  network_join_dialog(NULL),
#endif
  statistics_dialog(NULL),
  preferences_dialog(NULL),
#ifdef HAVE_EXERCISES
  exercises_dialog(NULL),
#endif
  main_window(NULL),
  applet_window(NULL),
  about(NULL)
{
  assert(instance == 0);
  instance = this;
  gui = GUI::get_instance();

  for (int i = 0; i < MENU_SIZEOF; i++)
    {
      menus[i] = NULL;
    }
}


//! Destructor.
Menus::~Menus()
{
  TRACE_ENTER("Menus::~Menus");
  instance = 0;
  TRACE_EXIT();
}


Menus *
Menus::get_instance()
{
  assert(instance != 0);
  return instance;
}


void
Menus::init(MainWindow *main_window, AppletWindow *applet_window)
{
  this->main_window = main_window;
  this->applet_window = applet_window;
  
#ifdef PLATFORM_OS_WIN32
  menus[MENU_APPLET] = new W32TrayMenu();
#else
  menus[MENU_APPLET] = new MainGtkMenu(true);
#endif

#if defined(PLATFORM_OS_WIN32)
  W32AppletWindow *w32_applet_window = dynamic_cast<W32AppletWindow*>(applet_window);
  menus[MENU_NATIVE] = new W32AppletMenu(main_window, w32_applet_window);
#elif defined(HAVE_GNOMEAPPLET)
  GnomeAppletWindow *gnome_applet_window = dynamic_cast<GnomeAppletWindow*>(applet_window);
  menus[MENU_NATIVE] = new GnomeAppletMenu(gnome_applet_window);
#endif
  
#if defined(PLATFORM_OS_OSX)
  menus[MENU_MAINWINDOW] = new OSXGtkMenu(true);
#else
  menus[MENU_MAINWINDOW] = new MainGtkMenu(false);
#endif      

  for (int i = 0; i < MENU_SIZEOF; i++)
    {
      if (menus[i] != NULL)
        {
          menus[i]->init();
        }
    }

  if (main_window != NULL)
    {
      menus[MENU_MAINWINDOW]->add_accel(*main_window);
    }

  resync();
}


void
Menus::popup(const MenuKind kind,
             const guint button,
             const guint activate_time)
{
  Menu *pop_menu = menus[kind];
  if (pop_menu)
    {
      pop_menu->popup(button, activate_time);
    }
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
  core->set_operation_mode(m, true);
  resync();  
}


void
Menus::set_usage_mode(UsageMode m)
{
  ICore *core = CoreFactory::get_core();
  core->set_usage_mode(m, true);
  resync();  
}


void
Menus::on_menu_quiet()
{
  TRACE_ENTER("Menus::on_menu_quiet");
  set_operation_mode(OPERATION_MODE_QUIET);
  TRACE_EXIT();
}



void
Menus::on_menu_suspend()
{
  TRACE_ENTER("Menus::on_menu_suspend");
  set_operation_mode(OPERATION_MODE_SUSPENDED);
  TRACE_EXIT();
}


void
Menus::on_menu_normal()
{
  TRACE_ENTER("Menus::on_menu_normal");
  set_operation_mode(OPERATION_MODE_NORMAL);
  TRACE_EXIT();
}


void
Menus::on_menu_reading(bool reading)
{
  TRACE_ENTER("Menus::on_menu_reading");
  set_usage_mode(reading ? USAGE_MODE_READING : USAGE_MODE_NORMAL);
  resync();
  TRACE_EXIT();
}


//! Preferences Dialog.
void
Menus::on_menu_preferences()
{
  if (preferences_dialog == NULL)
    {
      preferences_dialog = new PreferencesDialog();
      preferences_dialog->signal_response().connect(sigc::mem_fun(*this, &Menus::on_preferences_response));

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
      exercises_dialog->signal_response().connect(sigc::mem_fun(*this, &Menus::on_exercises_response));

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
  exercises_dialog->hide();

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
      statistics_dialog->signal_response().connect(sigc::mem_fun(*this, &Menus::on_statistics_response));

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
  if (about == NULL)
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


      about = new Gtk::AboutDialog;

      about->set_name("Workrave");
#ifdef HAVE_GTK3
      std::vector<Glib::ustring> authors;
      for (int index = 0; workrave_authors[index] != NULL; index++)
        {
          authors.push_back(workrave_authors[index]);
        }
      about->set_authors(authors);
#else
      about->set_authors(workrave_authors);
#endif
      
      about->set_copyright(workrave_copyright);
      about->set_comments(_("This program assists in the prevention and recovery"
                            " of Repetitive Strain Injury (RSI)."));
      about->set_logo(pixbuf);
      about->set_translator_credits(workrave_translators);

#ifdef PLATFORM_OS_WIN32
      about->set_url_hook(sigc::mem_fun(*this, &Menus::on_about_link_activate));
#endif

#ifdef GIT_VERSION
      about->set_version(PACKAGE_VERSION "\n(" GIT_VERSION ")");
#else
      about->set_version(PACKAGE_VERSION);
#endif
      about->set_website("http://www.workrave.org/");
      about->set_website_label("www.workrave.org");

      about->signal_response().connect(sigc::mem_fun(*this, &Menus::on_about_response));
    }
  about->present();
}


#ifdef PLATFORM_OS_WIN32
void
Menus::on_about_link_activate(Gtk::AboutDialog &about, const Glib::ustring &link)
{
  (void) about;
  ShellExecute(NULL, "open", link.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
#endif

void
Menus::on_about_response(int response)
{
  (void) response;

  delete about;
  about = NULL;
}

void
Menus::on_menu_network_join()
{
#ifdef HAVE_DISTRIBUTION
  if (network_join_dialog == NULL)
    {
      network_join_dialog = new NetworkJoinDialog();
      network_join_dialog->signal_response().connect(sigc::mem_fun(*this, &Menus::on_network_join_response));
    }
  network_join_dialog->present();
#endif
}

#ifdef HAVE_DISTRIBUTION
void
Menus::on_network_join_response(int response)
{
  (void) response;

  assert(network_join_dialog != NULL);
  network_join_dialog->hide();

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
            connect(sigc::mem_fun(*this, &Menus::on_network_log_response));

          resync();

          network_log_dialog->run();
        }
    }
  else if (network_log_dialog != NULL)
    {
      network_log_dialog->hide();
      delete network_log_dialog;
      network_log_dialog = NULL;
      resync();  
    }


  TRACE_EXIT();
#endif
}


#ifdef HAVE_DISTRIBUTION
void
Menus::on_network_log_response(int response)
{
  TRACE_ENTER_MSG("Menus::on_network_log_response", response);
  (void) response;

  assert(network_log_dialog != NULL);

  network_log_dialog->hide();

  // done by gtkmm ??? delete network_log_dialog;
  network_log_dialog = NULL;

  resync();  

  TRACE_EXIT();
}
#endif

void
Menus::on_statistics_response(int response)
{
  (void) response;

  assert(statistics_dialog != NULL);
  statistics_dialog->hide();

  delete statistics_dialog;
  statistics_dialog = NULL;
}

void
Menus::on_preferences_response(int response)
{
  (void) response;

  assert(preferences_dialog != NULL);
  preferences_dialog->hide();

  CoreFactory::get_configurator()->save();

  delete preferences_dialog;
  preferences_dialog = NULL;
}

void
Menus::applet_command(short cmd)
{
  switch (cmd)
    {
    case MENU_COMMAND_OPEN:
      on_menu_open_main_window();
      break;
    case MENU_COMMAND_PREFERENCES:
      on_menu_preferences();
      break;
#ifdef HAVE_EXERCISES
    case MENU_COMMAND_EXERCISES:
      on_menu_exercises();
      break;
#endif
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
#ifdef HAVE_DISTRIBUTION
    case MENU_COMMAND_NETWORK_LOG:
      on_menu_network_log(network_log_dialog == NULL);
      break;
    case MENU_COMMAND_NETWORK_RECONNECT:
      on_menu_network_reconnect();
      break;
#endif
    case MENU_COMMAND_STATISTICS:
      on_menu_statistics();
      break;
    case MENU_COMMAND_ABOUT:
      on_menu_about();
      break;
    case MENU_COMMAND_MODE_READING:
      {
        ICore *core = CoreFactory::get_core();
        on_menu_reading(core->get_usage_mode() == USAGE_MODE_NORMAL);
      }
      break;
    }
}


void
Menus::resync()
{
  static bool syncing = false;
  if (syncing)
    return;
  syncing = true;
  TRACE_ENTER("Menus::resync");
  
  for (int i = 0; i < MENU_SIZEOF; i++)
    {
      if (menus[i] != NULL)
        {
          ICore *core = CoreFactory::get_core();

          menus[i]->resync(core->get_operation_mode(),
                           core->get_usage_mode(),
#ifdef HAVE_DISTRIBUTION
                           network_log_dialog != NULL);
#else
                           false);
#endif

        }
    }

  syncing = false;
  TRACE_EXIT();
}


void
Menus::locale_changed()
{
  static bool syncing = false;
  if (syncing)
    return;
  syncing = true;
  TRACE_ENTER("Menus::locale_changed");
  
  for (int i = 0; i < MENU_SIZEOF; i++)
    {
      if (menus[i] != NULL)
        {
          menus[i]->init();
        }
    }

  resync();
  
  syncing = false;
  TRACE_EXIT();
}
