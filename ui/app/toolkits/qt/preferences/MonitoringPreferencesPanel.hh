// Copyright (C) 2002-  2012 Rob Caelers <robc@krandor.nl>
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

#ifndef MONITORINGPREFERENCESPANEL_HH
#define MONITORINGPREFERENCESPANEL_HH

#include <QtGui>
#include <QtWidgets>

#include <memory>

#include "ui/IApplicationContext.hh"

class DataConnector;
class Configurator;

class MonitoringPreferencesPanel
  : public QWidget
{
  Q_OBJECT

public:
  explicit MonitoringPreferencesPanel(std::shared_ptr<IApplicationContext> app, QWidget *parent = nullptr);
  ~MonitoringPreferencesPanel() override;

private slots:
  void on_debug_pressed();

#if defined(PLATFORM_OS_WINDOWS)
  void on_monitor_type_toggled();
#endif

private:
  void create_panel();

private:
  std::shared_ptr<IApplicationContext> app;
  std::shared_ptr<DataConnector> connector;

  QPushButton *debug_btn{nullptr};

#if defined(PLATFORM_OS_WINDOWS)
  QCheckBox *monitor_type_cb{nullptr};
  QSlider *sensitivity_slider{nullptr};
  QHBoxLayout *sensitivity_box{nullptr};
#endif
};

#endif // MONITORINGPREFERENCESPANEL_HH
