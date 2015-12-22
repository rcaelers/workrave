// Copyright (C) 20014 Rob Caelers
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

#ifndef STATUSICON_HH
#define STATUSICON_HH

#include <string>
#include <memory>
#include <boost/signals2.hpp>

#include <QSystemTrayIcon>
#include <QIcon>

#include "core/CoreTypes.hh"
#include "utils/ScopedConnections.hh"

#include "MenuModel.hh"

class ToolkitMenu;

class StatusIcon : public QObject
{
  Q_OBJECT

public:
  explicit StatusIcon(MenuModel::Ptr menu_model);

  void set_tooltip(std::string &tip);
  void show_balloon(std::string id, const std::string &title, const std::string &balloon);

  boost::signals2::signal<void()> &signal_activate();

private:
  void on_operation_mode_changed(workrave::OperationMode m);

public Q_SLOTS:
  void on_activate(QSystemTrayIcon::ActivationReason reason);

private:
  std::map<workrave::OperationMode, QIcon> mode_icons;
  std::shared_ptr<QSystemTrayIcon> tray_icon;
  std::shared_ptr<ToolkitMenu> menu;

  boost::signals2::signal<void()> activate_signal;
  scoped_connections connections;
};

#endif // STATUSICON_HH
