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
#include "config.h"
#endif

#include "debug.hh"

#include "StatusIcon.hh"

#include "Backend.hh"
#include "ICore.hh"

#include "UiUtil.hh"
#include "GUIConfig.hh"

using namespace std;
using namespace workrave;

StatusIcon::StatusIcon(MenuModel::Ptr menu_model)
{
  TRACE_ENTER("StatusIcon::StatusIcon");

  mode_icons[workrave::OperationMode::Normal] = UiUtil::create_icon("workrave-icon-medium.png");
  mode_icons[workrave::OperationMode::Suspended] = UiUtil::create_icon("workrave-suspended-icon-medium.png");
  mode_icons[workrave::OperationMode::Quiet] = UiUtil::create_icon("workrave-quiet-icon-medium.png");

  tray_icon = new QSystemTrayIcon();

  menu = std::make_shared<ToolkitMenu>(menu_model);
  tray_icon->setContextMenu(menu->get_menu());
  TRACE_EXIT();
}

StatusIcon::~StatusIcon()
{
}

void
StatusIcon::init()
{
  ICore::Ptr core = Backend::get_core();
  connections.connect(core->signal_operation_mode_changed(), std::bind(&StatusIcon::on_operation_mode_changed, this, std::placeholders::_1));

  OperationMode mode = core->get_operation_mode_regular();
  tray_icon->setIcon(mode_icons[mode]);

  QObject::connect(tray_icon, &QSystemTrayIcon::activated, this, &StatusIcon::on_activate);

  GUIConfig::trayicon_enabled().connect([&] (bool enabled)
                                        {
                                          tray_icon->setVisible(enabled);
                                        });

  bool tray_icon_enabled = GUIConfig::trayicon_enabled()();
  tray_icon->setVisible(tray_icon_enabled);
}

void
StatusIcon::on_operation_mode_changed(OperationMode m)
{
  TRACE_ENTER_MSG("StatusIcon::on_operation_mode_changed", (int)m);
  tray_icon->setIcon(mode_icons[m]);
  TRACE_EXIT();
}

void
StatusIcon::set_tooltip(std::string& tip)
{
  tray_icon->setToolTip(QString::fromStdString(tip));
}

void
StatusIcon::show_balloon(string id, const std::string& title, const string &balloon)
{
  tray_icon->showMessage(QString::fromStdString(title), QString::fromStdString(balloon));
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
