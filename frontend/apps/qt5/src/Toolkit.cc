// Tookit.cc
//
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
#include "config.h"
#endif

#include "Toolkit.hh"

#include <boost/make_shared.hpp>

#include <QDesktopWidget>
#include <QApplication>

#include "config/IConfigurator.hh"

#include "Menus.hh"
#include "GUIConfig.hh"

#include "PreludeWindow.hh"
#include "MicroBreakWindow.hh"
#include "RestBreakWindow.hh"
#include "DailyLimitWindow.hh"
#include "PreferencesDialog.hh"

#include "ToolkitMenu.hh"

using namespace std;
using namespace workrave;
using namespace workrave::config;

IToolkit::Ptr
Toolkit::create(int argc, char **argv)
{
  return Ptr(new Toolkit(argc, argv));
}


Toolkit::Toolkit(int argc, char **argv)
  : QApplication(argc, argv),
    heartbeat_timer(new QTimer(this))
{
}


Toolkit::~Toolkit()
{
}


void
Toolkit::init(MenuModel::Ptr menu_model, SoundTheme::Ptr sound_theme)
{
  this->menu_model = menu_model;
  this->sound_theme = sound_theme;

  setQuitOnLastWindowClosed(false);

  dock_menu = ToolkitMenu::create(menu_model, [](MenuModel::Ptr menu) { return menu->get_id() != Menus::QUIT; });
  dock_menu->get_menu()->setAsDockMenu();

  main_window =  boost::make_shared<MainWindow>(menu_model);
  connect(heartbeat_timer.get(), SIGNAL(timeout()), this, SLOT(on_timer()));
  heartbeat_timer->start(1000);

  main_window->show();
  main_window->raise();
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

void Toolkit::grab()
{
}

void Toolkit::ungrab()
{
}

std::string Toolkit::get_display_name()
{
  return "";
}

IBreakWindow::Ptr
Toolkit::create_break_window(int screen, BreakId break_id, BreakFlags break_flags)
{
  IBreakWindow::Ptr ret;

  GUIConfig::BlockMode block_mode = GUIConfig::block_mode()();
  if (break_id == BREAK_ID_MICRO_BREAK)
   {
     ret = MicroBreakWindow::create(screen, break_flags, block_mode);
   }
  else if (break_id == BREAK_ID_REST_BREAK)
   {
     ret = RestBreakWindow::create(screen, break_flags, block_mode);
   }
  else if (break_id == BREAK_ID_DAILY_LIMIT)
   {
     ret = DailyLimitWindow::create(screen, break_flags, block_mode);
   }

  return ret;
}


IPreludeWindow::Ptr
Toolkit::create_prelude_window(int screen, workrave::BreakId break_id)
{
  return PreludeWindow::create(screen, break_id);
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
      break;

    case WindowType::Preferences:
      if (!preferences_dialog)
        {
          preferences_dialog = boost::make_shared<PreferencesDialog>(sound_theme);
          connect(preferences_dialog.get(), &QDialog::accepted, this, &Toolkit::on_preferences_closed);
        }
      preferences_dialog->show();
      break;

    case WindowType::About:
      break;

    case WindowType::Exercises:
      if (!exercises_dialog)
        {
          exercises_dialog = boost::make_shared<ExercisesDialog>();
          connect(exercises_dialog.get(), &QDialog::accepted, this, &Toolkit::on_exercises_closed);
        }
      exercises_dialog->show();
      break;
    }
}

void
Toolkit::hide_window(WindowType type)
{
  switch (type)
    {
    case WindowType::Main:
      main_window->hide();
      break;

    case WindowType::Statistics:
      break;

    case WindowType::Preferences:
      preferences_dialog->hide();
      preferences_dialog.reset();
      break;

    case WindowType::About:
      break;

    case WindowType::Exercises:
      exercises_dialog->hide();
      exercises_dialog.reset();
      break;
    }
}

int
Toolkit::get_screen_count() const
{
  QDesktopWidget *dw = QApplication::desktop();
  return dw->screenCount();
}

void
Toolkit::on_timer()
{
  timer_signal();

  main_window->heartbeat();
}

void
Toolkit::on_exercises_closed()
{
  exercises_dialog.reset();
}

void
Toolkit::on_preferences_closed()
{
  preferences_dialog.reset();
}

boost::signals2::signal<void()> &
Toolkit::signal_timer()
{
  return timer_signal;
}
