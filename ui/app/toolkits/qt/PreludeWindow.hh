// Copyright (C) 2006, 2007, 2013 Raymond Penners & Rob Caelers
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

#ifndef PRELUDEWINDOW_HH
#define PRELUDEWINDOW_HH

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>

#include "utils/Signals.hh"

#include "Frame.hh"
#include "ui/IPreludeWindow.hh"
#include "TimeBar.hh"

#if defined(PLATFORM_OS_MACOS)
#  include "MouseMonitor.hh"
#endif

class PreludeWindow
  : public QWidget
  , public IPreludeWindow
  , public workrave::utils::Trackable
{
  Q_OBJECT

public:
  PreludeWindow(QScreen *screen, workrave::BreakId break_id);

  void start() override;
  void stop() override;
  void refresh() override;
  void set_progress(int value, int max_value) override;
  void set_stage(workrave::IApp::PreludeStage stage) override;
  void set_progress_text(workrave::IApp::PreludeProgressText text) override;

private:
  void on_frame_flash(bool frame_visible);
  void avoid_pointer(int x, int y);
  auto event(QEvent *event) -> bool override;

private:
  const static int SCREEN_MARGIN = 20;

  workrave::BreakId break_id;
  QScreen *screen{nullptr};

  int progress_value{0};
  int progress_max_value{1};

  bool flash_visible = false;
  QString progress_text;
  bool did_avoid = false;

  QVBoxLayout *layout{nullptr};
  TimeBar *timebar{nullptr};
  QLabel *label{nullptr};
  QLabel *image{nullptr};
  Frame *frame{nullptr};

#if defined(PLATFORM_OS_MACOS)
  MouseMonitor::Ptr mouse_monitor;
#endif
#if defined(HAVE_WAYLAND)
  std::shared_ptr<WaylandWindowManager> window_manager;
#endif
};

#endif // PRELUDEWINDOW_HH
