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

#include "stats/IStatisticsStore.hh"

#include <spdlog/spdlog.h>

#include "FileStatisticsStore.hh"
#include "SqliteStatisticsStore.hh"

namespace workrave::stats
{
  IStatisticsStore::Ptr StatisticsStoreFactory::create(const std::filesystem::path &state_directory)
  {
    auto store = std::make_shared<SqliteStatisticsStore>(state_directory);
    if (store->open())
      {
        return store;
      }

    spdlog::warn("falling back to file based statistics");
    return std::make_shared<FileStatisticsStore>(state_directory);
  }
} // namespace workrave::stats
