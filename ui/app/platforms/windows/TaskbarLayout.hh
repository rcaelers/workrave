// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Rob Caelers

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
