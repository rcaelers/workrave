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

#include "config/Config.hh"
#include "core/CoreTypes.hh"
#include "utils/Signals.hh"
#include "ui/IApplicationContext.hh"

#include "SizeGroup.hh"
#include "DataConnector.hh"

class TimerBoxPreferencesPanel
  : public QWidget
  , public workrave::utils::Trackable
{
  Q_OBJECT

public:
  explicit TimerBoxPreferencesPanel(std::shared_ptr<IApplicationContext> app, std::string name);

private:
  void init_enabled();
  void init_ontop();
  void init_placement();
  void init_cycle();
  void init_timer_display();
  void init_config();
  void init();

  void enable_buttons();
  void on_place_changed();

  auto on_enabled_toggled(const std::string &key, bool write) -> bool;
  auto on_timer_display_changed(int break_id, const std::string &key, bool write) -> bool;

private:
  std::shared_ptr<IApplicationContext> app;
  DataConnector::Ptr connector;
  std::string name;

  QVBoxLayout *layout{nullptr};
  QCheckBox *ontop_cb{nullptr};
  QCheckBox *enabled_cb{nullptr};
  QComboBox *place_button{nullptr};
  QComboBox *timer_display_button[workrave::BREAK_ID_SIZEOF];
  QSpinBox *cycle_entry{nullptr};
};

#endif // TIMERBOXUIPREFERENCESPANEL_HH
