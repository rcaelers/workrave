// PreludeWindow.hh
//
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

#include "IPreludeWindow.hh"

#include "utils/ScopedConnections.hh"

#include "TimeBar.hh"
#include "Frame.hh"

#ifdef PLATFORM_OS_OSX
#include "MouseMonitor.hh"
#endif

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>

class PreludeWindow : public QWidget, public IPreludeWindow
{
  Q_OBJECT

public:
  PreludeWindow(int screen, workrave::BreakId break_id);
  ~PreludeWindow() override;

  void start() override;
  void stop() override;
  void refresh() override;
  void set_progress(int value, int max_value) override;
  void set_stage(workrave::IApp::PreludeStage stage) override;
  void set_progress_text(workrave::IApp::PreludeProgressText text) override;

private:
  void on_frame_flash(bool frame_visible);
  void avoid_pointer(int x, int y);
  bool event(QEvent *event) override;

private:
  const static int SCREEN_MARGIN = 20;

  workrave::BreakId break_id;
  int screen;

  int progress_value;
  int progress_max_value;

  bool flash_visible;
  std::string progress_text;
  bool did_avoid;

  QVBoxLayout *layout;
  TimeBar *timebar;
  QLabel *label;
  QLabel *image;
  Frame* frame;
  scoped_connections connections;

#ifdef PLATFORM_OS_OSX
  MouseMonitor::Ptr mouse_monitor;
#endif
};

#endif // PRELUDEWINDOW_HH
