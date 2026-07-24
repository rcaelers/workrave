// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#ifndef TOOLKIT_MACOS_HH
#define TOOLKIT_MACOS_HH

#include "Toolkit.hh"

#include <memory>
#include <QProxyStyle>
#include <QStyleOption>
#include "MacDockTile.hh"
#include "ui/macos/MacOSLocker.hh"

class MacOSMenuStyle : public QProxyStyle
{
public:
  using QProxyStyle::QProxyStyle;
  int pixelMetric(PixelMetric metric, const QStyleOption *opt, const QWidget *w) const override;
};

class ToolkitMacOS : public Toolkit
{
public:
  ToolkitMacOS(int argc, char **argv);
  ~ToolkitMacOS() override = default;

  // IToolkit
  void init(std::shared_ptr<IApplicationContext> app) override;
  auto get_locker() -> std::shared_ptr<Locker> override;

  auto get_desktop_image() -> QPixmap override;

private:
  bool eventFilter(QObject *obj, QEvent *event) override;

  std::shared_ptr<MacOSLocker> locker;
  std::shared_ptr<ToolkitMenu> dock_menu;
  std::unique_ptr<MacDockTile> dock_tile;
  QTimer *dock_timer{nullptr};
};

#endif // TOOLKIT_MACOS_HH
