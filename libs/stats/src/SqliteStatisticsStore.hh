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

#ifndef WORKRAVE_LIBS_STATS_SQLITESTATISTICSSTORE_HH
#define WORKRAVE_LIBS_STATS_SQLITESTATISTICSSTORE_HH

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <sqlite3.h>

#include "IStatisticsStore.hh"

namespace workrave::stats
{
  //! Statistics storage in a SQLite database.
  /*!
   *  One row per day in stats_day, with the counters normalized into
   *  stats_break and stats_misc so that counters can be added, or arrive from a
   *  newer version of Workrave, without a schema change.
   *
   *  The day in progress is the day named by the "today" key in schema_info;
   *  every other day is history. That mirrors the todaystats/historystats split
   *  of the text format that preceded this store.
   */
  class SqliteStatisticsStore : public IStatisticsStore
  {
  public:
    explicit SqliteStatisticsStore(std::filesystem::path state_directory);
    ~SqliteStatisticsStore() override;

    SqliteStatisticsStore(const SqliteStatisticsStore &) = delete;
    SqliteStatisticsStore &operator=(const SqliteStatisticsStore &) = delete;
    SqliteStatisticsStore(SqliteStatisticsStore &&) = delete;
    SqliteStatisticsStore &operator=(SqliteStatisticsStore &&) = delete;

    bool open();

    [[nodiscard]] std::optional<DailyStatsRecord> load_today() override;
    [[nodiscard]] std::vector<DailyStatsRecord> load_history() override;
    void save_today(const DailyStatsRecord &record) override;
    void set_break_counter(workrave::BreakId break_id, int counter, int64_t value) override;
    void append_history(const DailyStatsRecord &record) override;
    bool delete_all() override;

    [[nodiscard]] std::optional<DailyStatsRecord> load_date(const Date &date) override;
    [[nodiscard]] std::vector<Date> get_dates(const Date &from, const Date &to) override;
    [[nodiscard]] std::optional<Date> get_previous_date(const Date &date) override;
    [[nodiscard]] std::optional<Date> get_next_date(const Date &date) override;
    [[nodiscard]] std::optional<Date> get_first_date() override;
    [[nodiscard]] std::optional<Date> get_last_date() override;
    [[nodiscard]] int64_t get_total_misc(int counter, const Date &from, const Date &to) override;

  private:
    [[nodiscard]] std::optional<Date> query_date(const char *sql, const std::optional<std::string> &day);
    [[nodiscard]] std::filesystem::path database_path() const;

    bool execute(const char *sql);
    bool create_schema();
    bool import_text_statistics();

    [[nodiscard]] std::optional<std::string> get_property(const std::string &key);
    bool set_property(const std::string &key, const std::string &value);

    [[nodiscard]] bool has_day(const Date &date);
    bool store_day(const DailyStatsRecord &record);
    [[nodiscard]] std::optional<DailyStatsRecord> load_day(const std::string &day);
    [[nodiscard]] std::vector<std::string> load_days();

  private:
    std::filesystem::path state_directory;
    sqlite3 *db{nullptr};
  };
} // namespace workrave::stats

#endif // WORKRAVE_LIBS_STATS_SQLITESTATISTICSSTORE_HH
