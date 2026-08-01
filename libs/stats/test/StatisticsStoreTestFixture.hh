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

#ifndef WORKRAVE_LIBS_STATS_TEST_STATISTICSSTORETESTFIXTURE_HH
#define WORKRAVE_LIBS_STATS_TEST_STATISTICSSTORETESTFIXTURE_HH

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "stats/IStatisticsStore.hh"

#include "FileStatisticsStore.hh"
#include "SqliteStatisticsStore.hh"

namespace workrave::stats
{
  //! Reports dates and times readably rather than as a byte dump when a check fails.
  inline void
  PrintTo(const Date &date, std::ostream *os)
  {
    *os << date;
  }

  inline void
  PrintTo(const LocalTime &time, std::ostream *os)
  {
    *os << date_of(time) << " " << std::chrono::hh_mm_ss{time_of_day(time)};
  }
} // namespace workrave::stats

namespace workrave::stats::test
{
  //! A single day exactly as Workrave 1.10/1.11 writes it.
  inline const char *const REFERENCE_STATS = "WorkRaveStats 4\n"
                                             "D 12 6 125 9 15 12 6 125 17 42\n"
                                             "B 0 7 1 2 3 4 5 6 7 \n"
                                             "B 1 7 8 9 10 11 12 13 14 \n"
                                             "B 2 7 15 16 17 18 19 20 21 \n"
                                             "B 3 7 22 23 24 25 26 27 28 \n"
                                             "m 6 100 200 300 400 500 600 \n";

  //! The day the reference statistics belong to, optionally moved within the month.
  inline Date
  reference_date(int mday = 12)
  {
    return std::chrono::year{2025} / 7 / mday;
  }

  //! The reference day, optionally moved to another day of the month.
  inline DailyStatsRecord
  make_record(int mday = 12)
  {
    using namespace std::chrono;

    DailyStatsRecord record;
    record.start = to_local_time(reference_date(mday), hours{9} + minutes{15});
    record.stop = to_local_time(reference_date(mday), hours{17} + minutes{42});

    record.break_stats.resize(4);
    int64_t value = 1;
    for (auto &break_stats: record.break_stats)
      {
        break_stats.resize(7);
        for (auto &counter: break_stats)
          {
            counter = value++;
          }
      }

    record.misc_stats = {100, 200, 300, 400, 500, 600};
    return record;
  }

  //! Gives every test its own state directory to keep statistics in.
  class StatisticsStoreTest : public ::testing::Test
  {
  protected:
    StatisticsStoreTest()
    {
      // Named after the test, so that the test binaries can run in parallel and
      // anything left behind by a crash says which test left it.
      std::string name = "workrave-stats-test";
      const auto *info = ::testing::UnitTest::GetInstance()->current_test_info();
      if (info != nullptr)
        {
          name += std::string("-") + info->test_suite_name() + "-" + info->name();
        }

      directory = std::filesystem::temp_directory_path() / name;
      std::filesystem::remove_all(directory);
      std::filesystem::create_directories(directory);
    }

    ~StatisticsStoreTest() override
    {
      std::filesystem::remove_all(directory);
    }

    StatisticsStoreTest(const StatisticsStoreTest &) = delete;
    StatisticsStoreTest &operator=(const StatisticsStoreTest &) = delete;
    StatisticsStoreTest(StatisticsStoreTest &&) = delete;
    StatisticsStoreTest &operator=(StatisticsStoreTest &&) = delete;

    void write(const std::string &name, const std::string &content) const
    {
      std::ofstream file((directory / name).string());
      file << content;
    }

    [[nodiscard]] std::string read(const std::string &name) const
    {
      std::ifstream file((directory / name).string());
      std::stringstream ss;
      ss << file.rdbuf();
      return ss.str();
    }

    [[nodiscard]] bool exists(const std::string &name) const
    {
      return std::filesystem::exists(directory / name);
    }

    [[nodiscard]] std::shared_ptr<FileStatisticsStore> file_store() const
    {
      return std::make_shared<FileStatisticsStore>(directory);
    }

    [[nodiscard]] std::shared_ptr<SqliteStatisticsStore> sqlite_store() const
    {
      auto store = std::make_shared<SqliteStatisticsStore>(directory);
      EXPECT_TRUE(store->open());
      return store;
    }

    std::filesystem::path directory;
  };

  //! The date based queries, which both stores must answer the same way.
  inline void
  check_date_queries(const IStatisticsStore::Ptr &store)
  {
    store->append_history(make_record(12));
    store->append_history(make_record(14));
    store->save_today(make_record(16));

    const Date first = reference_date(12);
    const Date middle = reference_date(14);
    const Date today = reference_date(16);
    const Date gap = reference_date(15);

    EXPECT_EQ(store->get_first_date(), first);
    EXPECT_EQ(store->get_last_date(), today);

    EXPECT_EQ(store->get_previous_date(middle), first);
    EXPECT_EQ(store->get_next_date(middle), today);
    EXPECT_FALSE(store->get_previous_date(first).has_value());
    EXPECT_FALSE(store->get_next_date(today).has_value());

    // A date without statistics still finds its neighbours.
    EXPECT_EQ(store->get_previous_date(gap), middle);
    EXPECT_EQ(store->get_next_date(gap), today);

    std::vector<Date> dates = store->get_dates(first, today);
    ASSERT_EQ(dates.size(), 3U);
    EXPECT_EQ(dates[0], first);
    EXPECT_EQ(dates[1], middle);
    EXPECT_EQ(dates[2], today);

    dates = store->get_dates(reference_date(13), gap);
    ASSERT_EQ(dates.size(), 1U);
    EXPECT_EQ(dates[0], middle);

    const Date next_month_start = std::chrono::year{2025} / 8 / 1;
    const Date next_month_end = std::chrono::year{2025} / 8 / std::chrono::last;
    EXPECT_TRUE(store->get_dates(next_month_start, next_month_end).empty());

    // Every day of the reference record holds 100 in counter 0 and 200 in counter 1.
    EXPECT_EQ(store->get_total_misc(0, first, today), 300);
    EXPECT_EQ(store->get_total_misc(0, first, middle), 200);
    EXPECT_EQ(store->get_total_misc(0, first, first), 100);
    EXPECT_EQ(store->get_total_misc(1, first, today), 600);
    EXPECT_EQ(store->get_total_misc(0, next_month_start, next_month_end), 0);

    std::optional<DailyStatsRecord> day = store->load_date(middle);
    ASSERT_TRUE(day.has_value());
    EXPECT_EQ(day->date(), middle);
    EXPECT_FALSE(store->load_date(gap).has_value());
  }
} // namespace workrave::stats::test

#endif // WORKRAVE_LIBS_STATS_TEST_STATISTICSSTORETESTFIXTURE_HH
