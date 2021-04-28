// Copyright (C) 2007 - 2013 Rob Caelers & Raymond Penners
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

#include <QApplication>

#include "commonui/GUIConfig.hh"
#include "config/IConfigurator.hh"

#include "DailyLimitWindow.hh"
#include "MicroBreakWindow.hh"
#include "PreludeWindow.hh"
#include "RestBreakWindow.hh"
#include "UiUtil.hh"

#if defined(PLATFORM_OS_MACOS)
#  include "ToolkitPlatformMac.hh"
#elif defined(PLATFORM_OS_WINDOWS)
#  include "ToolkitPlatformWindows.hh"
#elif defined(PLATFORM_OS_LINUX)
#  include "ToolkitPlatformLinux.hh"
#endif

using namespace workrave;
using namespace workrave::config;

Toolkit::Toolkit(int argc, char **argv)
  : QApplication(argc, argv)
  , heartbeat_timer(new QTimer(this))
{
}

void
Toolkit::init(MenuModel::Ptr menu_model, SoundTheme::Ptr sound_theme)
{
  this->menu_model = menu_model;
  this->sound_theme = sound_theme;

  setQuitOnLastWindowClosed(false);
  setAttribute(Qt::AA_UseHighDpiPixmaps, true);

#ifdef PLATFORM_OS_MACOS
  dock_menu = std::make_shared<ToolkitMenu>(menu_model, [](MenuNode::Ptr menu) { return menu->get_id() != Menus::QUIT; });
  dock_menu->get_menu()->setAsDockMenu();
#endif

  status_icon = std::make_shared<StatusIcon>(menu_model);

  main_window = new MainWindow(menu_model, nullptr);
  connect(heartbeat_timer, SIGNAL(timeout()), this, SLOT(on_timer()));
  heartbeat_timer->start(1000);

  main_window->show();
  main_window->raise();

#if defined(PLATFORM_OS_MACOS)
  dock = std::make_shared<Dock>();
  platform = std::make_shared<ToolkitPlatformMac>();
#elif defined(PLATFORM_OS_WINDOWS)
  platform = std::make_shared<ToolkitPlatformWindows>();
#elif defined(PLATFORM_OS_LINUX)
  platform = std::make_shared<ToolkitPlatformLinux>();
#endif
}

void
Toolkit::terminate()
{
  exit();
}

void
Toolkit::run()
{
  exec();
}

const char *
Toolkit::get_display_name()
{
  return NULL;
}

IBreakWindow::Ptr
Toolkit::create_break_window(int screen_index, BreakId break_id, BreakFlags break_flags)
{
  IBreakWindow::Ptr ret;

  QList<QScreen *> screens = QGuiApplication::screens();
  QScreen *screen = screens.at(screen_index);

  if (break_id == BREAK_ID_MICRO_BREAK)
    {
      ret = std::make_shared<MicroBreakWindow>(platform, screen, break_flags);
    }
  else if (break_id == BREAK_ID_REST_BREAK)
    {
      ret = std::make_shared<RestBreakWindow>(platform, sound_theme, screen, break_flags);
    }
  else if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      ret = std::make_shared<DailyLimitWindow>(platform, screen, break_flags);
    }

  return ret;
}

IPreludeWindow::Ptr
Toolkit::create_prelude_window(int screen_index, workrave::BreakId break_id)
{
  QList<QScreen *> screens = QGuiApplication::screens();
  QScreen *screen = screens.at(screen_index);

  return std::make_shared<PreludeWindow>(screen, break_id);
}

void
Toolkit::show_window(WindowType type)
{
  switch (type)
    {
    case WindowType::Main:
      main_window->show();
      main_window->raise();
      break;

    case WindowType::Statistics:
      if (!statistics_dialog)
        {
          statistics_dialog = new StatisticsDialog;
          statistics_dialog->setAttribute(Qt::WA_DeleteOnClose);
        }
      statistics_dialog->show();
      break;

    case WindowType::Preferences:
      if (!preferences_dialog)
        {
          preferences_dialog = new PreferencesDialog(sound_theme);
          preferences_dialog->setAttribute(Qt::WA_DeleteOnClose);
        }
      preferences_dialog->show();
      break;

    case WindowType::About:
      if (!about_dialog)
        {
          about_dialog = new AboutDialog;
          about_dialog->setAttribute(Qt::WA_DeleteOnClose);
        }
      about_dialog->show();
      break;

    case WindowType::Exercises:
      if (!exercises_dialog)
        {
          exercises_dialog = new ExercisesDialog(sound_theme);
          exercises_dialog->setAttribute(Qt::WA_DeleteOnClose);
        }
      exercises_dialog->show();
      break;
    }
}

int
Toolkit::get_screen_count() const
{
  QList<QScreen *> screens = QGuiApplication::screens();
  return screens.size();
}

void
Toolkit::show_balloon(const std::string &id, const std::string &title, const std::string &balloon)
{
  status_icon->show_balloon(QString::fromStdString(id), QString::fromStdString(title), QString::fromStdString(balloon));
}

void
Toolkit::create_oneshot_timer(int ms, std::function<void()> func)
{
  new OneshotTimer(ms, func);
}

void
Toolkit::on_timer()
{
  timer_signal();

  main_window->heartbeat();
}

boost::signals2::signal<void()> &
Toolkit::signal_timer()
{
  return timer_signal;
}
