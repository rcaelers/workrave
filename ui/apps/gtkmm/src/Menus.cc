// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "commonui/nls.h"
#include "debug.hh"
#include "commonui/credits.h"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <iostream>

#include "Menus.hh"
#include "GUI.hh"
#include "GtkUtil.hh"

#include "PreferencesDialog.hh"
#include "StatisticsDialog.hh"
#include "DebugDialog.hh"
#include "core/IStatistics.hh"

#include "commonui/Backend.hh"
#include "core/ICore.hh"
#include "config/IConfigurator.hh"

#ifdef HAVE_DISTRIBUTION
#  include "core/IDistributionManager.hh"
#  include "NetworkJoinDialog.hh"
#  include "NetworkLogDialog.hh"
#endif

#include "ExercisesDialog.hh"
#include "commonui/Exercise.hh"

#include "MainGtkMenu.hh"
#include "AppletControl.hh"

#ifdef HAVE_INDICATOR
#  include "IndicatorAppletMenu.hh"
#endif

#ifdef HAVE_DBUS
#  include "GenericDBusApplet.hh"
#endif

#ifdef PLATFORM_OS_WINDOWS
#  include <shellapi.h>

#  include "W32AppletWindow.hh"
#  include "W32TrayMenu.hh"
#  include "W32AppletMenu.hh"
#endif

#if defined(PLATFORM_OS_MACOS)
#  include "MacOSGtkMenu.hh"
#endif

#include "commonui/MenuEnums.hh"

using namespace workrave;
using namespace workrave::utils;

Menus::Menus(SoundTheme::Ptr sound_theme)
  : sound_theme(sound_theme)
{
  gui = GUI::get_instance();

  for (int i = 0; i < MENU_SIZEOF; i++)
    {
      menus[i] = nullptr;
    }
}

Menus::~Menus()
{
  TRACE_ENTER("Menus::~Menus");
  TRACE_EXIT();
}

void
Menus::init(AppletControl *applet_control)
{
#if defined(PLATFORM_OS_WINDOWS) || defined(HAVE_DBUS)
  std::shared_ptr<IAppletWindow> applet_window;
#endif

#if defined(PLATFORM_OS_MACOS)
  menus[MENU_MAINWINDOW] = new MacOSGtkMenu(true);
#else
  menus[MENU_MAINWINDOW] = new MainGtkMenu(false);
#endif

#ifdef PLATFORM_OS_WINDOWS
  menus[MENU_MAINAPPLET] = new W32TrayMenu();
#else
  menus[MENU_MAINAPPLET] = new MainGtkMenu(true);
#endif

#if defined(PLATFORM_OS_WINDOWS)
  applet_window = applet_control->get_applet_window(AppletControl::AppletType::Windows);
  std::shared_ptr<W32AppletWindow> w32_applet_window = std::dynamic_pointer_cast<W32AppletWindow>(applet_window);
  menus[MENU_APPLET_W32] = new W32AppletMenu(w32_applet_window.get());
#endif

#if defined(HAVE_DBUS)
  applet_window = applet_control->get_applet_window(AppletControl::AppletType::GenericDBus);
  std::shared_ptr<GenericDBusApplet> indicator_applet = std::dynamic_pointer_cast<GenericDBusApplet>(applet_window);
  menus[MENU_APPLET_GENERICDBUS] = indicator_applet.get();

#  if defined(HAVE_INDICATOR)
  menus[MENU_APPLET_INDICATOR] = new IndicatorAppletMenu();
#  endif
#endif

  for (int i = 0; i < MENU_SIZEOF; i++)
    {
      if (menus[i] != nullptr)
        {
          menus[i]->init();
        }
    }

  resync();
}

void
Menus::popup(const MenuKind kind, const guint button, const guint activate_time)
{
  IMenu *pop_menu = menus[kind];
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
Menus::on_set_operation_mode(OperationMode m)
{
  auto core = Backend::get_core();
  core->set_operation_mode(m);
  resync();
}

void
Menus::set_usage_mode(UsageMode m)
{
  auto core = Backend::get_core();
  core->set_usage_mode(m);
  resync();
}

void
Menus::on_menu_quiet()
{
  TRACE_ENTER("Menus::on_menu_quiet");
  on_set_operation_mode(OperationMode::Quiet);
  TRACE_EXIT();
}

void
Menus::on_menu_suspend()
{
  TRACE_ENTER("Menus::on_menu_suspend");
  on_set_operation_mode(OperationMode::Suspended);
  TRACE_EXIT();
}

void
Menus::on_menu_normal()
{
  TRACE_ENTER("Menus::on_menu_normal");
  on_set_operation_mode(OperationMode::Normal);
  TRACE_EXIT();
}

void
Menus::on_menu_reading(bool reading)
{
  TRACE_ENTER("Menus::on_menu_reading");
  set_usage_mode(reading ? UsageMode::Reading : UsageMode::Normal);
  resync();
  TRACE_EXIT();
}

//! Preferences Dialog.
void
Menus::on_menu_preferences()
{
  if (preferences_dialog == nullptr)
    {
      preferences_dialog = new PreferencesDialog(sound_theme);
      preferences_dialog->signal_response().connect(sigc::mem_fun(*this, &Menus::on_preferences_response));

      preferences_dialog->run();
    }
  else
    {
      preferences_dialog->present();
    }
}

//! Exercises Dialog.
void
Menus::on_menu_exercises()
{
  TRACE_ENTER("Menus::on_menu_exercises");
  if (exercises_dialog == nullptr)
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
  (void)response;

  assert(exercises_dialog != nullptr);
  exercises_dialog->hide();

  delete exercises_dialog;
  exercises_dialog = nullptr;
}

//! Statistics Dialog.
void
Menus::on_menu_statistics()
{
  if (statistics_dialog == nullptr)
    {
      auto core = Backend::get_core();
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

//! Debug Dialog.
void
Menus::on_menu_debug()
{
  if (debug_dialog == nullptr)
    {
      debug_dialog = new DebugDialog();
      debug_dialog->signal_response().connect(sigc::mem_fun(*this, &Menus::on_debug_response));

      debug_dialog->run();
    }
  else
    {
      debug_dialog->present();
    }
}

//! About Dialog.
void
Menus::on_menu_about()
{
  if (about == nullptr)
    {
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = GtkUtil::create_pixbuf("workrave.png");
      about = new Gtk::AboutDialog;

      about->set_name("Workrave");
#ifdef HAVE_GTK3
      std::vector<Glib::ustring> authors;
      for (int index = 0; workrave_authors[index] != nullptr; index++)
        {
          authors.emplace_back(workrave_authors[index]);
        }
      about->set_authors(authors);
#else
      about->set_authors(workrave_authors);
#endif

      about->set_copyright(workrave_copyright);
      about->set_comments(
        _("This program assists in the prevention and recovery"
          " of Repetitive Strain Injury (RSI)."));
      about->set_logo(pixbuf);
      about->set_translator_credits(workrave_translators);

#if defined(PLATFORM_OS_WINDOWS) && !defined(HAVE_GTK3)
      about->set_url_hook(sigc::mem_fun(*this, &Menus::on_about_link_activate));
#endif

#ifdef WORKRAVE_GIT_VERSION
      about->set_version(PACKAGE_VERSION "\n(" WORKRAVE_GIT_VERSION ")");
#else
      about->set_version(PACKAGE_VERSION);
#endif
      about->set_website("http://www.workrave.org/");
      about->set_website_label("www.workrave.org");

      about->signal_response().connect(sigc::mem_fun(*this, &Menus::on_about_response));
    }
  about->present();
}

#ifdef PLATFORM_OS_WINDOWS
void
Menus::on_about_link_activate(Gtk::AboutDialog &about, const Glib::ustring &link)
{
  (void)about;
  ShellExecuteA(NULL, "open", link.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
#endif

void
Menus::on_about_response(int response)
{
  (void)response;

  delete about;
  about = nullptr;
}

void
Menus::on_menu_network_join()
{
#ifdef HAVE_DISTRIBUTION
  if (network_join_dialog == nullptr)
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
  (void)response;

  assert(network_join_dialog != nullptr);
  network_join_dialog->hide();

  if (response == Gtk::RESPONSE_OK)
    {
      auto core = Backend::get_core();
      IDistributionManager *dist_manager = core->get_distribution_manager();
      std::string peer = network_join_dialog->get_connect_url();
      dist_manager->connect(peer);
      Backend::get_configurator()->save();
    }

  delete network_join_dialog;
  network_join_dialog = nullptr;
}
#endif

void
Menus::on_menu_network_leave()
{
#ifdef HAVE_DISTRIBUTION
  auto core = Backend::get_core();
  IDistributionManager *dist_manager = core->get_distribution_manager();
  if (dist_manager != nullptr)
    {
      dist_manager->disconnect_all();
    }
#endif
}

void
Menus::on_menu_network_reconnect()
{
#ifdef HAVE_DISTRIBUTION
  auto core = Backend::get_core();
  IDistributionManager *dist_manager = core->get_distribution_manager();
  if (dist_manager != nullptr)
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
      if (network_log_dialog == nullptr)
        {
          network_log_dialog = new NetworkLogDialog();
          network_log_dialog->signal_response().connect(sigc::mem_fun(*this, &Menus::on_network_log_response));

          resync();

          network_log_dialog->run();
        }
    }
  else if (network_log_dialog != nullptr)
    {
      network_log_dialog->hide();
      delete network_log_dialog;
      network_log_dialog = nullptr;
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
  (void)response;

  assert(network_log_dialog != nullptr);

  network_log_dialog->hide();

  // done by gtkmm ??? delete network_log_dialog;
  network_log_dialog = nullptr;

  resync();

  TRACE_EXIT();
}
#endif

void
Menus::on_statistics_response(int response)
{
  (void)response;

  assert(statistics_dialog != nullptr);
  statistics_dialog->hide();

  delete statistics_dialog;
  statistics_dialog = nullptr;
}

void
Menus::on_preferences_response(int response)
{
  (void)response;

  assert(preferences_dialog != nullptr);
  preferences_dialog->hide();

  Backend::get_configurator()->save();

  delete preferences_dialog;
  preferences_dialog = nullptr;
}

void
Menus::on_debug_response(int response)
{
  (void)response;

  assert(debug_dialog != nullptr);
  debug_dialog->hide();

  delete debug_dialog;
  debug_dialog = nullptr;
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
#ifdef HAVE_DISTRIBUTION
    case MENU_COMMAND_NETWORK_LOG:
      on_menu_network_log(network_log_dialog == nullptr);
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
        auto core = Backend::get_core();
        on_menu_reading(core->get_usage_mode() == UsageMode::Normal);
      }
      break;
    case MENU_COMMAND_QUIT:
      on_menu_quit();
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

  for (int i = 0; i < MENU_SIZEOF; i++)
    {
      if (menus[i] != nullptr)
        {
          auto core = Backend::get_core();

          /* Use operation_mode_regular here to show the mode that will be restored
          if an override is in place. That is also necessary because if get_operation_mode()
          were called instead then that would cause a mode that's possibly an override to
          be returned. If that then the override mode menu item would be set_active() during
          the resync, and cause a signal that then calls back into core with the override mode
          as a regular mode. That would erase whatever the user's regular mode was.
          */
          menus[i]->resync(core->get_operation_mode_regular(),
                           core->get_usage_mode(),
#ifdef HAVE_DISTRIBUTION
                           network_log_dialog != nullptr);
#else
                           false);
#endif
        }
    }

  syncing = false;
}

void
Menus::locale_changed()
{
  static bool syncing = false;
  if (syncing)
    return;
  syncing = true;

  for (int i = 0; i < MENU_SIZEOF; i++)
    {
      if (menus[i] != nullptr)
        {
          menus[i]->init();
        }
    }

  resync();

  syncing = false;
}
