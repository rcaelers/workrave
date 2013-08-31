// TimerBoxPreferencesPanel.hh
//
// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#ifndef TIMERBOXUIPREFERENCESPANEL_HH
#define TIMERBOXUIPREFERENCESPANEL_HH

#include <QtGui>
#include <QtWidgets>

#include "SizeGroup.hh"
#include "DataConnector.hh"

#include "config/Config.hh"
#include "CoreTypes.hh"

class TimerBoxPreferencesPanel :
  public QGroupBox,
  public workrave::config::IConfiguratorListener
{
  Q_OBJECT
  
public:
  TimerBoxPreferencesPanel(std::string name);
  virtual ~TimerBoxPreferencesPanel();
  
private: 
  void create_page();
  void init_page_values();
  void init_page_callbacks();
  void enable_buttons();
  void on_enabled_toggled();
  void on_place_changed();
  void on_display_changed(int break_id);
  void on_cycle_time_changed();
  void on_always_on_top_toggled();

  void config_changed_notify(const std::string &key);

private:
  DataConnector *connector;

  std::string name;

  QVBoxLayout *layout;
  
  QCheckBox *ontop_cb;
  QCheckBox *enabled_cb;
  QComboBox *place_button;
  QComboBox *timer_display_button[workrave::BREAK_ID_SIZEOF];
  QSpinBox *cycle_entry;
 
};

#endif // TIMERBOXUIPREFERENCESPANEL_HH
