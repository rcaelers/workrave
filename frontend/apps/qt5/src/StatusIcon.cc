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

#include "CoreFactory.hh"
#include "ICore.hh"

#include "UiUtil.hh"
#include "GUIConfig.hh"

// #include "GUI.hh"
// #include "CoreFactory.hh"
// #include "config/IConfigurator.hh"
// #include "GUIConfig.hh"
// #include "Menus.hh"
// #include "TimerBoxControl.hh"
// #include "GtkUtil.hh"

using namespace std;

StatusIcon::StatusIcon(MenuModel::Ptr menu_model)
{
  TRACE_ENTER("StatusIcon::StatusIcon");

  mode_icons[workrave::OperationMode::Normal] = UiUtil::create_icon("workrave-icon-medium.png");
  mode_icons[workrave::OperationMode::Suspended] = UiUtil::create_icon("workrave-suspended-icon-medium.png");
  mode_icons[workrave::OperationMode::Quiet] = UiUtil::create_icon("workrave-quiet-icon-medium.png");

  tray_icon = new QSystemTrayIcon();
  
  menu = ToolkitMenu::create(menu_model);
  tray_icon->setContextMenu(menu->get_menu());
  TRACE_EXIT();
}

StatusIcon::~StatusIcon()
{
}

void
StatusIcon::init()
{
  insert_icon();

  QObject::connect(tray_icon, &QSystemTrayIcon::activated, this, &StatusIcon::on_activate);

  GUIConfig::trayicon_enabled().connect([&] (bool enabled)
                                        {
                                          if (tray_icon->isVisible() != enabled)
                                            {
                                              visibility_changed_signal();
                                              tray_icon->setVisible(enabled);
                                            }
                                        });
  
  bool tray_icon_enabled = GUIConfig::trayicon_enabled()();
  tray_icon->setVisible(tray_icon_enabled);
}

void
StatusIcon::insert_icon()
{
  // Create status icon
  ICore::Ptr core = CoreFactory::get_core();
  OperationMode mode = core->get_operation_mode_regular();
  tray_icon->setIcon(mode_icons[mode]);
}

void
StatusIcon::set_operation_mode(OperationMode m)
{
  TRACE_ENTER_MSG("StatusIcon::set_operation_mode", (int)m);
  tray_icon->setIcon(mode_icons[m]);
  TRACE_EXIT();
}

bool
StatusIcon::is_visible() const
{
  return tray_icon->isVisible();
}

void
StatusIcon::set_tooltip(std::string& tip)
{
  tray_icon->setToolTip(QString::fromStdString(tip));
}

void
StatusIcon::show_balloon(string id, const string &balloon)
{
  tray_icon->showMessage("Workrave", QString::fromStdString(balloon));
}

void
StatusIcon::on_activate(QSystemTrayIcon::ActivationReason reason)
{
  activate_signal();
}

boost::signals2::signal<void()> &
StatusIcon::signal_visibility_changed()
{
  return visibility_changed_signal;
}

boost::signals2::signal<void()> &
StatusIcon::signal_activate()
{
  return activate_signal;
}

boost::signals2::signal<void(string)> &
StatusIcon::signal_balloon_activate()
{
  return balloon_activate_signal;
}
