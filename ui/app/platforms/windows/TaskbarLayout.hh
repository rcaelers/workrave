// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_TASKBAR_LAYOUT_HH
#define WORKRAVE_TASKBAR_LAYOUT_HH

#include <windows.h>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "TaskbarPlacement.hh"

// UI Automation stays on a windowless MTA thread. No COM interface crosses
// apartments; the UI consumes only a copied geometry snapshot.
class TaskbarLayout
{
public:
  struct Snapshot
  {
    HWND taskbar{nullptr};
    RECT bounds{};
    std::vector<workrave::taskbar::Interval> occupied;
    std::chrono::steady_clock::time_point measured;
  };

  TaskbarLayout();
  ~TaskbarLayout();
  TaskbarLayout(const TaskbarLayout &) = delete;
  TaskbarLayout &operator=(const TaskbarLayout &) = delete;
  TaskbarLayout(TaskbarLayout &&) = delete;
  TaskbarLayout &operator=(TaskbarLayout &&) = delete;
  Snapshot snapshot();

private:
  void run();
  std::mutex mutex;
  std::condition_variable wake;
  bool stopping{false};
  Snapshot latest;
  std::thread worker;
};

#endif
