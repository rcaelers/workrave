// Copyright (C) 2014 Rob Caelers <robc@krandor.org>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "StatusIcon.hh"

#include "commonui/Backend.hh"
#include "commonui/GUIConfig.hh"

#include "ToolkitMenu.hh"
#include "UiUtil.hh"

using namespace workrave;

StatusIcon::StatusIcon(MenuModel::Ptr menu_model)
{
  mode_icons[workrave::OperationMode::Normal]    = UiUtil::create_icon("workrave-icon-medium.png");
  mode_icons[workrave::OperationMode::Suspended] = UiUtil::create_icon("workrave-suspended-icon-medium.png");
  mode_icons[workrave::OperationMode::Quiet]     = UiUtil::create_icon("workrave-quiet-icon-medium.png");

  tray_icon = std::make_shared<QSystemTrayIcon>();

  menu = std::make_shared<ToolkitMenu>(menu_model, [](MenuNode::Ptr menu) { return true; });
  tray_icon->setContextMenu(menu->get_menu());

  ICore::Ptr core = Backend::get_core();
  connections.connect(core->signal_operation_mode_changed(),
                      std::bind(&StatusIcon::on_operation_mode_changed, this, std::placeholders::_1));
  OperationMode mode = core->get_operation_mode_regular();
  tray_icon->setIcon(mode_icons[mode]);

  GUIConfig::trayicon_enabled().attach([&](bool enabled) { tray_icon->setVisible(enabled); });

  QObject::connect(tray_icon.get(), &QSystemTrayIcon::activated, this, &StatusIcon::on_activate);
}

void
StatusIcon::on_operation_mode_changed(OperationMode m)
{
  tray_icon->setIcon(mode_icons[m]);
}

void
StatusIcon::set_tooltip(const QString &tip)
{
  tray_icon->setToolTip(tip);
}

void
StatusIcon::show_balloon(const QString &id, const QString &title, const QString &balloon)
{
  tray_icon->showMessage(title, balloon);
}

void
StatusIcon::on_activate(QSystemTrayIcon::ActivationReason reason)
{
  activate_signal();
}

boost::signals2::signal<void()> &
StatusIcon::signal_activate()
{
  return activate_signal;
}
