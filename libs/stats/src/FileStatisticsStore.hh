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

#ifndef WORKRAVE_LIBS_STATS_FILESTATISTICSSTORE_HH
#define WORKRAVE_LIBS_STATS_FILESTATISTICSSTORE_HH

#include <filesystem>
#include <iosfwd>
#include <map>
#include <optional>
#include <vector>

#include "DailyStatsRecord.hh"

namespace workrave::stats
{
  //! Statistics storage in the historical "WorkRaveStats" text format.
  /*!
   *  Only ever used internally, to import a legacy text-format install into
   *  SqliteStatisticsStore, and by tests to build fixtures. Never held through an
   *  interface, so it is a plain class rather than an IStatisticsStore.
   */
  class FileStatisticsStore
  {
  public:
    explicit FileStatisticsStore(std::filesystem::path state_directory);

    [[nodiscard]] std::optional<DailyStatsRecord> load_today();
    [[nodiscard]] std::vector<DailyStatsRecord> load_history();
    void save_today(const DailyStatsRecord &record);
    void append_history(const DailyStatsRecord &record);
    bool delete_all();

    [[nodiscard]] std::optional<DailyStatsRecord> load_date(const Date &date);
    [[nodiscard]] std::vector<Date> get_dates(const Date &from, const Date &to);
    [[nodiscard]] std::optional<Date> get_previous_date(const Date &date);
    [[nodiscard]] std::optional<Date> get_next_date(const Date &date);
    [[nodiscard]] std::optional<Date> get_first_date();
    [[nodiscard]] std::optional<Date> get_last_date();
    [[nodiscard]] int64_t get_total_misc(int counter, const Date &from, const Date &to);

  private:
    [[nodiscard]] std::map<Date, DailyStatsRecord> load_all();

    [[nodiscard]] std::filesystem::path today_path() const;
    [[nodiscard]] std::filesystem::path history_path() const;

    [[nodiscard]] std::vector<DailyStatsRecord> load(const std::filesystem::path &path, bool history) const;

    static bool read_header(std::istream &file);
    static DailyStatsRecord parse_day(std::istream &args);
    static void parse_break(std::istream &args, DailyStatsRecord &record);
    static void parse_misc(std::istream &args, DailyStatsRecord &record, bool valid);
    static void parse_total_active(std::istream &args, DailyStatsRecord &record);
    static void write_time(std::ostream &file, LocalTime time);
    static void write_day(std::ostream &file, const DailyStatsRecord &record);

  private:
    std::filesystem::path state_directory;
  };
} // namespace workrave::stats

#endif // WORKRAVE_LIBS_STATS_FILESTATISTICSSTORE_HH
