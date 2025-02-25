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

#ifndef MONITORINGPREFERENCEPANEL_HH
#define MONITORINGPREFERENCEPANEL_HH

#include <string>

#include "utils/Signals.hh"
#include "ui/IApplicationContext.hh"

#include "ui/prefwidgets/gtkmm/Widget.hh"
#include "ui/prefwidgets/gtkmm/BoxWidget.hh"

#include <gtkmm.h>

class DataConnector;
class Configurator;

class MonitoringPreferencePanel
  : public Gtk::VBox
  , public workrave::utils::Trackable
{
public:
  explicit MonitoringPreferencePanel(std::shared_ptr<IApplicationContext> app);
  ~MonitoringPreferencePanel() override;

private:
  void create_panel();
  void on_debug_pressed();

#if defined(PLATFORM_OS_WINDOWS)
  void on_monitor_type_toggled();
#endif

private:
  std::shared_ptr<IApplicationContext> app;
  std::shared_ptr<DataConnector> connector;

  Gtk::Button *debug_btn{nullptr};

#if defined(PLATFORM_OS_WINDOWS)
  Gtk::CheckButton *monitor_type_cb{nullptr};
  Glib::RefPtr<Gtk::Adjustment> sensitivity_adjustment{Gtk::Adjustment::create(3, 0, 100)};
  Gtk::HBox *sensitivity_box{nullptr};
#endif
};

#endif // MONITORINGPREFERENCEPANEL_HH
