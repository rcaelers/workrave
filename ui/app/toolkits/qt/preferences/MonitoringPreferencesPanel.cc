// Copyright (C) 2002 - 2013 Rob Caelers & Raymond Penners
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

#include "MonitoringPreferencesPanel.hh"

#include "debug.hh"

#include "DataConnector.hh"

#include "core/CoreConfig.hh"

using namespace workrave;
using namespace workrave::utils;

MonitoringPreferencesPanel::MonitoringPreferencesPanel(std::shared_ptr<IApplicationContext> app, QWidget *parent)
  : QWidget(parent)
  , app(app)
  , connector(std::make_shared<DataConnector>(app))
{
  TRACE_ENTRY();
  create_panel();
}

MonitoringPreferencesPanel::~MonitoringPreferencesPanel()
{
  TRACE_ENTRY();
}

void MonitoringPreferencesPanel::create_panel()
{
  auto *main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(12, 12, 12, 12);

#if defined(PLATFORM_OS_WINDOWS)
  monitor_type_cb = new QCheckBox(this);
  monitor_type_cb->setLayoutDirection(Qt::RightToLeft);
  monitor_type_cb->setText(tr("Use alternate monitor"));
  connect(monitor_type_cb, &QCheckBox::toggled, this, &MonitoringPreferencesPanel::on_monitor_type_toggled);
  main_layout->addWidget(monitor_type_cb);

  auto *monitor_type_help1 = new QLabel(tr("Enable this option if Workrave fails to detect when you are using your computer"), this);
  main_layout->addWidget(monitor_type_help1);
  auto *monitor_type_help2 = new QLabel(tr("Workrave needs to be restarted manually after changing this setting"), this);
  main_layout->addWidget(monitor_type_help2);

  sensitivity_box = new QHBoxLayout();
  auto *sensitivity_label = new QLabel(tr("Mouse sensitivity:"), this);
  sensitivity_box->addWidget(sensitivity_label);
  auto *sensitivity_spin = new QSpinBox(this);
  sensitivity_box->addWidget(sensitivity_spin);
  main_layout->addLayout(sensitivity_box);

  connector->connect(CoreConfig::monitor_sensitivity(), dc::wrap(sensitivity_spin));

  std::string monitor_type;
  app->get_configurator()->get_value_with_default("advanced/monitor", monitor_type, "default");

  monitor_type_cb->setChecked(monitor_type != "default");
  sensitivity_box->setEnabled(monitor_type != "default");
#endif

  debug_btn = new QPushButton(tr("Debug monitoring"), this);
  connect(debug_btn, &QPushButton::clicked, this, &MonitoringPreferencesPanel::on_debug_pressed);
  main_layout->addWidget(debug_btn);
}

#if defined(PLATFORM_OS_WINDOWS)
void MonitoringPreferencesPanel::on_monitor_type_toggled()
{
  bool on = monitor_type_cb->isChecked();
  app->get_configurator()->set_value("advanced/monitor", on ? "lowlevel" : "default");
  sensitivity_box->setEnabled(on);
}
#endif

void MonitoringPreferencesPanel::on_debug_pressed()
{
  app->get_toolkit()->show_window(IToolkit::WindowType::Debug);
}
