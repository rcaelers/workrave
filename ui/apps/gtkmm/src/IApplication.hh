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

#ifndef IAPPLICATION_HH
#define IAPPLICATION_HH

#include <glibmm.h>

#include <memory>
#include <boost/signals2.hpp>

#include "commonui/SoundTheme.hh"
#include "HeadInfo.hh"

class MainWindow;

class IApplication : public std::enable_shared_from_this<IApplication>
{
public:
  typedef std::shared_ptr<IApplication> Ptr;

  virtual ~IApplication() = default;

  // Next:
  // virtual void restbreak_now() = 0;
  // virtual void terminate() = 0;
  // virtual void interrupt_grab() = 0;

  // Legacy
  virtual sigc::signal0<void> &signal_heartbeat() = 0;

  virtual MainWindow *get_main_window() const = 0;
  virtual SoundTheme::Ptr get_sound_theme() const = 0;

  virtual void open_main_window() = 0;
  virtual void restbreak_now() = 0;

  virtual void interrupt_grab() = 0;

  virtual HeadInfo get_head_info(int screen_index) const = 0;
  virtual int get_head_count() const = 0;

  virtual int map_to_head(int &x, int &y) = 0;
  virtual void map_from_head(int &x, int &y, int head) = 0;
  virtual bool bound_head(int &x, int &y, int width, int height, int &head) = 0;
  virtual void terminate() = 0;
};

#endif // IAPPLICATION_HH
