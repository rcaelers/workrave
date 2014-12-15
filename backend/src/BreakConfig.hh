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
#include "utils/ScopedConnections.hh"

#include "BreakStateModel.hh"

class DayTimePred;

using namespace workrave;

class BreakConfig
{
public:
  typedef boost::shared_ptr<BreakConfig> Ptr;
 
public:
  static Ptr create(BreakId break_id, BreakStateModel::Ptr break_state_model, Timer::Ptr timer);

  BreakConfig(BreakId break_id, BreakStateModel::Ptr break_state_model, Timer::Ptr timer);

  bool is_microbreak_used_for_activity() const;
  bool is_enabled() const; 
  
private:  
  void load_timer_config();
  void load_break_config();
  DayTimePred *create_time_pred(std::string spec);

private:
  BreakId break_id;
  BreakStateModel::Ptr break_state_model;
  Timer::Ptr timer;
  bool enabled;
  bool use_microbreak_activity;
  scoped_connections connections;
};

#endif // BREAKCONFIG_HH
