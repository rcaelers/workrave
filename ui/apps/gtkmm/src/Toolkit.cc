// Copyright (C) 2001 - 2021 Rob Caelers <robc@krandor.nl>
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

#include "Toolkit.hh"

#include "commonui/GUIConfig.hh"
#include "config/IConfigurator.hh"

#include "DailyLimitWindow.hh"
#include "MicroBreakWindow.hh"
#include "PreludeWindow.hh"
#include "RestBreakWindow.hh"

//#include "UiUtil.hh"

#include "GtkUtil.hh"

#include "debug.hh"
#include "commonui/nls.h"
#include "commonui/credits.h"

// #if defined(PLATFORM_OS_MACOS)
// #  include "ToolkitPlatformMac.hh"
// #elif defined(PLATFORM_OS_WINDOWS)
// #  include "ToolkitPlatformWindows.hh"
// #elif defined(PLATFORM_OS_UNIX)
// #  include "ToolkitPlatformUnix.hh"
// #endif

using namespace workrave;
using namespace workrave::config;

Toolkit::Toolkit(int argc, char **argv)
{
  // app = Gtk::Application::create(argc, argv, "org.workrave.WorkraveApplication");
  // app->hold();
}

void
Toolkit::init(MenuModel::Ptr menu_model, SoundTheme::Ptr sound_theme)
{
  this->menu_model = menu_model;
  this->sound_theme = sound_theme;

  // #if defined(PLATFORM_OS_MACOS)
  //   dock = std::make_shared<Dock>();
  //   platform = std::make_shared<ToolkitPlatformMac>();
  // #elif defined(PLATFORM_OS_WINDOWS)
  //   platform = std::make_shared<ToolkitPlatformWindows>();
  // #elif defined(PLATFORM_OS_LINUX)
  //   platform = std::make_shared<ToolkitPlatformLinux>();
  // #endif
}

HeadInfo
Toolkit::get_head_info(int screen_index) const
{
  Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
  Glib::RefPtr<Gdk::Monitor> monitor = display->get_monitor(screen_index);

  Gdk::Rectangle rect;
  monitor->get_geometry(rect);
  HeadInfo head;
  head.monitor = screen_index;
  monitor->get_geometry(head.geometry);

  return head;
}

int
Toolkit::get_head_count() const
{
  Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
  return display->get_n_monitors();
}

IBreakWindow::Ptr
Toolkit::create_break_window(int screen_index, BreakId break_id, BreakFlags break_flags)
{
  IBreakWindow::Ptr ret;

  HeadInfo head = get_head_info(screen_index);

  GUIConfig::BlockMode block_mode = GUIConfig::block_mode()();

  if (break_id == BREAK_ID_MICRO_BREAK)
    {
      ret = std::make_shared<MicroBreakWindow>(head, break_flags, block_mode);
    }
  else if (break_id == BREAK_ID_REST_BREAK)
    {
      ret = std::make_shared<RestBreakWindow>(sound_theme, head, break_flags, block_mode);
    }
  else if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      ret = std::make_shared<DailyLimitWindow>(head, break_flags, block_mode);
    }

  return ret;
}

IPreludeWindow::Ptr
Toolkit::create_prelude_window(int screen_index, workrave::BreakId break_id)
{
  HeadInfo head = get_head_info(screen_index);
  return std::make_shared<PreludeWindow>(head, break_id);
}

void
Toolkit::show_window(WindowType type)
{
  switch (type)
    {
    case WindowType::Main:
      show_main_window();
      break;

    case WindowType::Statistics:
      show_statistics();
      break;

    case WindowType::Preferences:
      show_preferences();
      break;

    case WindowType::About:
      show_about();
      break;

    case WindowType::Exercises:
      show_exercises();
      break;

    case WindowType::Debug:
      show_debug();
      break;
    }
}

void
Toolkit::show_about()
{
  if (about_dialog == NULL)
    {
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = GtkUtil::create_pixbuf("workrave.png");
      about_dialog = new Gtk::AboutDialog;

      about_dialog->set_name("Workrave");
      std::vector<Glib::ustring> authors;
      for (int index = 0; workrave_authors[index] != NULL; index++)
        {
          authors.push_back(workrave_authors[index]);
        }
      about_dialog->set_authors(authors);
      about_dialog->set_copyright(workrave_copyright);
      about_dialog->set_comments(
        _("This program assists in the prevention and recovery"
          " of Repetitive Strain Injury (RSI)."));
      about_dialog->set_logo(pixbuf);
      about_dialog->set_translator_credits(workrave_translators);

#ifdef GIT_VERSION
      about_dialog->set_version(WORKRAVE_VERSION "\n(" GIT_VERSION ")");
#else
      about_dialog->set_version(WORKRAVE_VERSION);
#endif
      about_dialog->set_website("http://www.workrave.org/");
      about_dialog->set_website_label("www.workrave.org");

      about_dialog->signal_response().connect([this](int reponse) {
        about_dialog->hide();
        delete about_dialog;
        about_dialog = NULL;
      });
    }
  about_dialog->present();
}

void
Toolkit::show_debug()
{
  if (!debug_dialog)
    {
      debug_dialog = new DebugDialog();
      debug_dialog->signal_response().connect([this](int reponse) {
        debug_dialog->hide();
        delete debug_dialog;
        debug_dialog = NULL;
      });
      debug_dialog->run();
    }
  else
    {
      debug_dialog->present();
    }
}

void
Toolkit::show_exercises()
{
  if (!exercises_dialog)
    {
      exercises_dialog = new ExercisesDialog(sound_theme);
      exercises_dialog->signal_response().connect([this](int reponse) {
        exercises_dialog->hide();
        delete exercises_dialog;
        exercises_dialog = NULL;
      });
      exercises_dialog->run();
    }
  else
    {
      exercises_dialog->present();
    }
}

void
Toolkit::show_main_window()
{
  // main_window->open_window();
  // main_window->show();
  // main_window->raise();
}

void
Toolkit::show_preferences()
{
  if (!preferences_dialog)
    {
      preferences_dialog = new PreferencesDialog(sound_theme);
      preferences_dialog->signal_response().connect([this](int reponse) {
        preferences_dialog->hide();
        delete preferences_dialog;
        preferences_dialog = NULL;
      });
    }
  else
    {
      preferences_dialog->present();
    }
}

void
Toolkit::show_statistics()
{
  if (!statistics_dialog)
    {
      statistics_dialog = new StatisticsDialog();
      statistics_dialog->signal_response().connect([this](int reponse) {
        statistics_dialog->hide();
        delete statistics_dialog;
        statistics_dialog = NULL;
      });

      statistics_dialog->run();
    }
  else
    {
      statistics_dialog->present();
    }
}

// void
// Menus::locale_changed()
// {
//   static bool syncing = false;
//   if (syncing)
//     return;
//   syncing = true;

//   for (int i = 0; i < MENU_SIZEOF; i++)
//     {
//       if (menus[i] != NULL)
//         {
//           menus[i]->init();
//         }
//     }

//   resync();

//   syncing = false;
// }
