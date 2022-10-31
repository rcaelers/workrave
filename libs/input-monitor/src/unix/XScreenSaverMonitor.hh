// Copyright (C) 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef XSCREENSAVERMONITOR_HH
#define XSCREENSAVERMONITOR_HH

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/scrnsaver.h>

#include "InputMonitor.hh"

class XScreenSaverMonitor : public InputMonitor
{
public:
  XScreenSaverMonitor() = default;
  ~XScreenSaverMonitor() override;

  bool init() override;
  void terminate() override;

private:
  virtual void run();

private:
  bool abort{false};
  std::shared_ptr<std::thread> monitor_thread;
  XScreenSaverInfo *screen_saver_info{nullptr};
  Display *xdisplay{nullptr};
  Drawable root;

  std::mutex mutex;
  std::condition_variable cond;
};

#endif // XSCREENSAVERMONITOR_HH
