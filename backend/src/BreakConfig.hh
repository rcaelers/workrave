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

#ifndef BREAKCONFIG_HH
#define BREAKCONFIG_HH

#include <boost/shared_ptr.hpp>

#include "config/Config.hh"

#include "BreakStateModel.hh"

class DayTimePred;

using namespace workrave;

class BreakConfig : public workrave::config::IConfiguratorListener
{
public:
  typedef boost::shared_ptr<BreakConfig> Ptr;
 
public:
  static Ptr create(BreakId break_id, BreakStateModel::Ptr break_state_model, Timer::Ptr timer, workrave::config::IConfigurator::Ptr configurator);

  BreakConfig(BreakId break_id, BreakStateModel::Ptr break_state_model, Timer::Ptr timer, workrave::config::IConfigurator::Ptr configurator);
  virtual ~BreakConfig();

  bool is_microbreak_used_for_activity() const;
  bool is_enabled() const; 
  
private:  
  void load_timer_config();
  void load_break_config();
  void config_changed_notify(const std::string &key);
  DayTimePred *create_time_pred(std::string spec);

private:
  BreakId break_id;
  BreakStateModel::Ptr break_state_model;
  Timer::Ptr timer;
  workrave::config::IConfigurator::Ptr configurator;
  bool enabled;
  bool use_microbreak_activity;
};

#endif // BREAKCONFIG_HH
