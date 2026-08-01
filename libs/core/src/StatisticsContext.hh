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

#ifndef WORKRAVE_CORE_STATISTICSCONTEXT_HH
#define WORKRAVE_CORE_STATISTICSCONTEXT_HH

#include "stats/IStatisticsContext.hh"

class Core;

class StatisticsContext : public workrave::stats::IStatisticsContext
{
public:
  explicit StatisticsContext(Core *core)
    : core(core)
  {
  }

  [[nodiscard]] bool is_active() const override;

private:
  Core *core{nullptr};
};

#endif // WORKRAVE_CORE_STATISTICSCONTEXT_HH
