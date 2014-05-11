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

#include <QSystemTrayIcon>
#include <QIcon>

#include "IStatusIcon.hh"

#include "MenuModel.hh"
#include "ToolkitMenu.hh"

class StatusIcon : public QObject, public IStatusIcon
{
  Q_OBJECT

public:
  StatusIcon(MenuModel::Ptr menu_model);
  virtual ~StatusIcon();

  virtual void init();
  virtual void set_operation_mode(workrave::OperationMode m);
  virtual void set_tooltip(std::string& tip);
  virtual bool is_visible() const;
  virtual void show_balloon(std::string id, const std::string& balloon);

  virtual boost::signals2::signal<void()> &signal_visibility_changed();
  virtual boost::signals2::signal<void()> &signal_activate();
  virtual boost::signals2::signal<void(std::string)> &signal_balloon_activate();

private:
  void insert_icon();

public slots:
  void on_activate(QSystemTrayIcon::ActivationReason reason);

private:
  std::map<workrave::OperationMode, QIcon> mode_icons;
  QSystemTrayIcon *tray_icon;
  ToolkitMenu::Ptr menu;

  boost::signals2::signal<void()> visibility_changed_signal;
  boost::signals2::signal<void()> activate_signal;
  boost::signals2::signal<void(std::string)> balloon_activate_signal;
};

#endif // STATUSICON_HH
