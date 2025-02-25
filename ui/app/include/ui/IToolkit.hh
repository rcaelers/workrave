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

#ifndef WORKRAVE_UI_ITOOLKIT_HH
#define WORKRAVE_UI_ITOOLKIT_HH

#include <memory>
#include <boost/signals2.hpp>

#include "config/IConfigurator.hh"
#include "ui/UiTypes.hh"

#include "ui/Locker.hh"
#include "ui/IBreakWindow.hh"
#include "ui/IPreludeWindow.hh"

class IApplicationContext;

class IToolkit
{
public:
  using Ptr = std::shared_ptr<IToolkit>;

  enum class WindowType
  {
    Main,
    Debug,
    Statistics,
    Preferences,
    About,
    Exercises
  };

  virtual ~IToolkit() = default;

  virtual void preinit(std::shared_ptr<workrave::config::IConfigurator> config) = 0;
  virtual void init(std::shared_ptr<IApplicationContext> app) = 0;
  virtual void deinit() = 0;

  virtual void terminate() = 0;
  virtual void run() = 0;

  virtual void hold() = 0;
  virtual void release() = 0;

  virtual int get_head_count() const = 0;

  virtual IBreakWindow::Ptr create_break_window(int screen_index, workrave::BreakId break_id, BreakFlags break_flags) = 0;
  virtual IPreludeWindow::Ptr create_prelude_window(int screen_index, workrave::BreakId break_id) = 0;
  virtual void show_window(WindowType type) = 0;

  virtual boost::signals2::signal<void()> &signal_timer() = 0;
  virtual boost::signals2::signal<void()> &signal_main_window_closed() = 0;
  virtual boost::signals2::signal<void(bool)> &signal_session_idle_changed() = 0;
  virtual boost::signals2::signal<void()> &signal_session_unlocked() = 0;
  virtual boost::signals2::signal<void()> &signal_status_icon_activated() = 0;

  virtual const char *get_display_name() const = 0;
  virtual void create_oneshot_timer(int ms, std::function<void()> func) = 0;
  virtual void show_notification(const std::string &id,
                                 const std::string &title,
                                 const std::string &balloon,
                                 std::function<void()> func) = 0;
  virtual void show_tooltip(const std::string &tip) = 0;

  virtual std::shared_ptr<Locker> get_locker() = 0;
};

inline std::ostream &
operator<<(std::ostream &stream, IToolkit::WindowType type)
{
  switch (type)
    {
    case IToolkit::WindowType::Main:
      stream << "main";
      break;
    case IToolkit::WindowType::Debug:
      stream << "debug";
      break;
    case IToolkit::WindowType::Statistics:
      stream << "statictics";
      break;
    case IToolkit::WindowType::Preferences:
      stream << "preferences";
      break;
    case IToolkit::WindowType::About:
      stream << "about";
      break;
    case IToolkit::WindowType::Exercises:
      stream << "exercises";
      break;
    }
  return stream;
}

#endif // WORKRAVE_UI_ITOOLKIT_HH
