// Copyright (C) 2001 -2013 Rob Caelers <robc@krandor.nl>
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

#ifndef ITOOLKIT_HH
#define ITOOLKIT_HH

#include <memory>
#include <boost/signals2.hpp>

#include "commonui/Backend.hh"
#include "commonui/SoundTheme.hh"
#include "commonui/UiTypes.hh"

#include "IBreakWindow.hh"
#include "IPreludeWindow.hh"
#include "MenuModel.hh"

class IToolkit
{
public:
  enum class WindowType
  {
    Main,
    Statistics,
    Preferences,
    About,
    Exercises
  };

  virtual ~IToolkit()
  {
  }

  virtual boost::signals2::signal<void()> &signal_timer() = 0;

  virtual void init(MenuModel::Ptr menu, SoundTheme::Ptr sound_theme) = 0;
  virtual void terminate() = 0;
  virtual void run() = 0;
  virtual const char *get_display_name() = 0;
  virtual IBreakWindow::Ptr create_break_window(int screen_index, workrave::BreakId break_id, BreakFlags break_flags) = 0;
  virtual IPreludeWindow::Ptr create_prelude_window(int screen_index, workrave::BreakId break_id) = 0;
  virtual void show_window(WindowType type) = 0;
  virtual int get_screen_count() const = 0;
  virtual void create_oneshot_timer(int ms, std::function<void()> func) = 0;
  virtual void show_balloon(const std::string &id, const std::string &title, const std::string &balloon) = 0;
};

#endif // ITOOLKIT_HH
