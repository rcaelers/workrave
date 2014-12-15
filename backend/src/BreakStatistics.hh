// BreakStatistics.hh
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

#ifndef BREAKSTATISTICS_HH
#define BREAKSTATISTICS_HH

#include <boost/shared_ptr.hpp>

#include "utils/ScopedConnections.hh"

#include "BreakStateModel.hh"
#include "Statistics.hh"

using namespace workrave;

class BreakStatistics
{
public:
  typedef boost::shared_ptr<BreakStatistics> Ptr;

public:
  static Ptr create(BreakId break_id, BreakStateModel::Ptr break_state_model, Timer::Ptr timer, Statistics::Ptr statistics);

  BreakStatistics(BreakId break_id, BreakStateModel::Ptr break_state_model, Timer::Ptr timer, Statistics::Ptr statistics);

  void update();
  void daily_reset();
  
private:
  void on_break_event(BreakEvent stage);

private:
  BreakId break_id;
  BreakStateModel::Ptr break_state_model;
  Timer::Ptr timer;
  Statistics::Ptr statistics;
  scoped_connections connections;
};

#endif // BREAKSTATISTICS_HH
