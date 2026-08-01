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

#ifndef WORKRAVE_LIBS_STATS_ISTATISTICSCONTEXT_HH
#define WORKRAVE_LIBS_STATS_ISTATISTICSCONTEXT_HH

#include <memory>


namespace workrave::stats
{
  //! Context the statistics need to know about the core they belong to.
  class IStatisticsContext
  {
  public:
    using Ptr = std::shared_ptr<IStatisticsContext>;

    virtual ~IStatisticsContext() = default;

    //! Whether the user is active right now.
    [[nodiscard]] virtual bool is_active() const = 0;
  };
} // namespace workrave::stats

#endif // WORKRAVE_LIBS_STATS_ISTATISTICSCONTEXT_HH
