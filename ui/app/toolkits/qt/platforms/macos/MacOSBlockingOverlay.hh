// Copyright (C) 2024 Rob Caelers <robc@krandor.nl>
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

#ifndef MACOS_BLOCKING_OVERLAY_HH
#define MACOS_BLOCKING_OVERLAY_HH

#include "ui/IBreakWindow.hh"

#include <QScreen>
#include <QWidget>

// Covers a secondary screen during a break so the user cannot interact with
// other monitors while the break window is shown on the active screen.
class MacOSBlockingOverlay : public IBreakWindow
{
public:
  explicit MacOSBlockingOverlay(QScreen *screen);
  ~MacOSBlockingOverlay() override;

  void init() override {}
  void start() override;
  void stop() override;
  void refresh() override {}
  void set_progress(int, int) override {}

private:
  QScreen *screen{nullptr};
  QWidget *overlay{nullptr};
};

#endif // MACOS_BLOCKING_OVERLAY_HH
