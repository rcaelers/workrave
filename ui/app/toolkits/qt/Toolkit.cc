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

#include <QApplication>

#include "Icon.hh"
#include "DailyLimitWindow.hh"
#include "MicroBreakWindow.hh"
#include "PreludeWindow.hh"
#include "RestBreakWindow.hh"
#include "UiUtil.hh"
#include "ui/GUIConfig.hh"
#include "debug.hh"

using namespace workrave;
using namespace workrave::config;

Toolkit::Toolkit(int argc, char **argv)
  : QApplication(argc, argv)
  , heartbeat_timer(new QTimer(this))
{
  TRACE_ENTRY();
  QCoreApplication::setOrganizationName("Workrave");
  QCoreApplication::setOrganizationDomain("workrave.org");
  QCoreApplication::setApplicationName("Workrave");
}

void
Toolkit::preinit(std::shared_ptr<workrave::config::IConfigurator> config)
{
  TRACE_ENTRY();
}

void
Toolkit::init(std::shared_ptr<IApplicationContext> app)
{
  this->app = app;

  setQuitOnLastWindowClosed(false);

  menu_model = app->get_menu_model();
  sound_theme = app->get_sound_theme();

  main_window = new MainWindow(app);

  // event_connections.emplace_back(main_window->signal_closed().connect(sigc::mem_fun(*this,
  // &Toolkit::on_main_window_closed)));

  status_icon = std::make_shared<StatusIcon>(app);
  // event_connections.emplace_back(status_icon->signal_activated().connect(sigc::mem_fun(*this,
  // &Toolkit::on_status_icon_activated)));
  // event_connections.emplace_back(status_icon->signal_balloon_activated().connect(sigc::mem_fun(*this,
  // &Toolkit::on_status_icon_balloon_activated)));

  connect(heartbeat_timer, SIGNAL(timeout()), this, SLOT(on_timer()));
  heartbeat_timer->start(1000);

  main_window->show();
  main_window->raise();
}

void
Toolkit::deinit()
{
  get_locker()->unlock();
}

void
Toolkit::terminate()
{
  exit();
}

void
Toolkit::hold()
{
  TRACE_ENTRY_PAR(hold_count);
  hold_count++;
  // TODO: main_window->set_can_close(hold_count > 0);
  setQuitOnLastWindowClosed(hold_count == 0);
}

void
Toolkit::release()
{
  TRACE_ENTRY_PAR(hold_count);
  hold_count--;
  setQuitOnLastWindowClosed(hold_count == 0);
  // TODO: main_window->set_can_close(hold_count > 0);
}

void
Toolkit::run()
{
  exec();
}

auto
Toolkit::get_head_count() const -> int
{
  QList<QScreen *> screens = QGuiApplication::screens();
  return screens.size();
}

auto
Toolkit::create_break_window(int screen_index, BreakId break_id, BreakFlags break_flags) -> IBreakWindow::Ptr
{
  IBreakWindow::Ptr ret;

  QList<QScreen *> screens = QGuiApplication::screens();
  QScreen *screen = screens.at(screen_index);

  if (break_id == BREAK_ID_MICRO_BREAK)
    {
      ret = std::make_shared<MicroBreakWindow>(app, screen, break_flags);
    }
  else if (break_id == BREAK_ID_REST_BREAK)
    {
      ret = std::make_shared<RestBreakWindow>(app, screen, break_flags);
    }
  else if (break_id == BREAK_ID_DAILY_LIMIT)
    {
      ret = std::make_shared<DailyLimitWindow>(app, screen, break_flags);
    }

  return ret;
}

auto
Toolkit::create_prelude_window(int screen_index, workrave::BreakId break_id) -> IPreludeWindow::Ptr
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
  if (about_dialog == nullptr)
    {
      about_dialog = new AboutDialog;
      about_dialog->setAttribute(Qt::WA_DeleteOnClose);
    }
  about_dialog->show();
}

void
Toolkit::show_debug()
{
}

void
Toolkit::show_exercises()
{
  if (exercises_dialog == nullptr)
    {
      exercises_dialog = new ExercisesDialog(app);
      exercises_dialog->setAttribute(Qt::WA_DeleteOnClose);
    }
  exercises_dialog->show();
}

void
Toolkit::show_main_window()
{
  TRACE_ENTRY();
  main_window->show();
  main_window->raise();
}

void
Toolkit::show_preferences()
{
  if (preferences_dialog == nullptr)
    {
      preferences_dialog = new PreferencesDialog(app);
      preferences_dialog->setAttribute(Qt::WA_DeleteOnClose);
    }
  preferences_dialog->show();
}

void
Toolkit::show_statistics()
{
  if (statistics_dialog == nullptr)
    {
      statistics_dialog = new StatisticsDialog(app);
      statistics_dialog->setAttribute(Qt::WA_DeleteOnClose);
    }
  statistics_dialog->show();
}

auto
Toolkit::signal_timer() -> boost::signals2::signal<void()> &
{
  return timer_signal;
}

auto
Toolkit::signal_main_window_closed() -> boost::signals2::signal<void()> &
{
  return main_window_closed_signal;
}

auto
Toolkit::signal_session_idle_changed() -> boost::signals2::signal<void(bool)> &
{
  return session_idle_changed_signal;
}

auto
Toolkit::signal_session_unlocked() -> boost::signals2::signal<void()> &
{
  return session_unlocked_signal;
}

auto
Toolkit::signal_status_icon_activated() -> boost::signals2::signal<void()> &
{
  return status_icon_activated_signal;
}

auto
Toolkit::get_display_name() const -> const char *
{
  return nullptr;
}

void
Toolkit::create_oneshot_timer(int ms, std::function<void()> func)
{
  // TODO: ms == 0
  new OneshotTimer(ms, func);
}

void
Toolkit::show_notification(const std::string &id,
                           const std::string &title,
                           const std::string &balloon,
                           std::function<void()> func)
{
  notify_add_confirm_function(id, func);
  status_icon->show_balloon(QString::fromStdString(id), QString::fromStdString(title), QString::fromStdString(balloon));
}

void
Toolkit::show_tooltip(const std::string &tip)
{
}

void
Toolkit::on_timer()
{
  timer_signal();
  main_window->heartbeat();
}

void
Toolkit::on_main_window_closed()
{
  main_window_closed_signal();
}

void
Toolkit::on_status_icon_balloon_activated(const std::string &id)
{
  TRACE_ENTRY();
  notify_confirm(id);
}

void
Toolkit::on_status_icon_activated()
{
  TRACE_ENTRY();
  status_icon_activated_signal();
}

void
Toolkit::notify_add_confirm_function(const std::string &id, std::function<void()> func)
{
  TRACE_ENTRY();
  notifiers[id] = func;
}

void
Toolkit::notify_confirm(const std::string &id)
{
  TRACE_ENTRY();
  if (notifiers.find(id) != notifiers.end())
    {
      notifiers[id]();
    }
}
