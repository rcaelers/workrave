// Copyright (C) 2026 Rob Caelers
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

#ifndef WORKRAVE_CORENEXT_STATISTICSCONTEXT_HH
#define WORKRAVE_CORENEXT_STATISTICSCONTEXT_HH

#include <utility>

#include "stats/IStatisticsContext.hh"

#include "IActivityMonitor.hh"

//! What the shared statistics need from this core.
class StatisticsContext : public workrave::stats::IStatisticsContext
{
public:
  explicit StatisticsContext(IActivityMonitor::Ptr monitor)
    : monitor(std::move(monitor))
  {
  }

  [[nodiscard]] bool is_active() const override
  {
    return monitor->is_active();
  }

private:
  IActivityMonitor::Ptr monitor;
};

#endif // WORKRAVE_CORENEXT_STATISTICSCONTEXT_HH
