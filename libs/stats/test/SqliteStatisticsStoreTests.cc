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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gtest/gtest.h>

#include <string>

#include "StatisticsStoreTestFixture.hh"

using namespace workrave::stats;
using namespace workrave::stats::test;

namespace
{
  class SqliteStoreTest : public StatisticsStoreTest
  {
  protected:
    //! Counts the rows of a table, straight from the database.
    [[nodiscard]] int count_rows(const std::string &table) const
    {
      sqlite3 *db = nullptr;
      EXPECT_EQ(sqlite3_open_v2((directory / "statistics.db").string().c_str(), &db, SQLITE_OPEN_READONLY, nullptr), SQLITE_OK);

      sqlite3_stmt *stmt = nullptr;
      EXPECT_EQ(sqlite3_prepare_v2(db, ("SELECT COUNT(*) FROM " + table).c_str(), -1, &stmt, nullptr), SQLITE_OK);

      int count = -1;
      if (sqlite3_step(stmt) == SQLITE_ROW)
        {
          count = sqlite3_column_int(stmt, 0);
        }

      sqlite3_finalize(stmt);
      sqlite3_close(db);
      return count;
    }
  };

  TEST_F(SqliteStoreTest, empty_database)
  {
    auto store = sqlite_store();

    EXPECT_FALSE(store->load_today().has_value());
    EXPECT_TRUE(store->load_history().empty());
    EXPECT_TRUE(exists("statistics.db"));
  }

  TEST_F(SqliteStoreTest, round_trip)
  {
    const DailyStatsRecord original = make_record();
    auto store = sqlite_store();
    store->save_today(original);

    auto record = store->load_today();
    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->break_stats, original.break_stats);
    EXPECT_EQ(record->misc_stats, original.misc_stats);
    EXPECT_EQ(record->start, original.start);
    EXPECT_EQ(record->stop, original.stop);
  }

  TEST_F(SqliteStoreTest, data_survives_reopen)
  {
    sqlite_store()->save_today(make_record());

    auto record = sqlite_store()->load_today();
    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->misc_stats[0], 100);
  }

  //! Saving the same day repeatedly must not accumulate days or counters.
  TEST_F(SqliteStoreTest, save_today_replaces_same_day)
  {
    auto store = sqlite_store();

    DailyStatsRecord record = make_record();
    store->save_today(record);
    record.misc_stats[0] = 999;
    store->save_today(record);

    EXPECT_TRUE(store->load_history().empty());

    auto today = store->load_today();
    ASSERT_TRUE(today.has_value());
    EXPECT_EQ(today->misc_stats[0], 999);
    EXPECT_EQ(today->misc_stats.size(), 6U);
  }

  //! Counters that are no longer reported must not linger.
  TEST_F(SqliteStoreTest, save_today_drops_removed_counters)
  {
    auto store = sqlite_store();

    store->save_today(make_record());

    DailyStatsRecord shorter = make_record();
    shorter.misc_stats.resize(2);
    shorter.break_stats.resize(1);
    store->save_today(shorter);

    auto today = store->load_today();
    ASSERT_TRUE(today.has_value());
    EXPECT_EQ(today->misc_stats.size(), 2U);
    EXPECT_EQ(today->break_stats.size(), 1U);
  }

  //! The day in progress is not part of the history.
  TEST_F(SqliteStoreTest, history_excludes_today)
  {
    auto store = sqlite_store();

    store->append_history(make_record(12));
    store->save_today(make_record(13));

    auto history = store->load_history();
    ASSERT_EQ(history.size(), 1U);
    EXPECT_EQ(history[0].date(), reference_date(12));

    auto today = store->load_today();
    ASSERT_TRUE(today.has_value());
    EXPECT_EQ(today->date(), reference_date(13));
  }

  //! Rolling over to a new day archives the previous one.
  TEST_F(SqliteStoreTest, day_rollover)
  {
    auto store = sqlite_store();

    store->save_today(make_record(12));
    EXPECT_TRUE(store->load_history().empty());

    // What Statistics::start_new_day() does.
    store->append_history(make_record(12));
    store->save_today(make_record(13));

    auto history = store->load_history();
    ASSERT_EQ(history.size(), 1U);
    EXPECT_EQ(history[0].date(), reference_date(12));
  }

  TEST_F(SqliteStoreTest, history_is_ordered_oldest_first)
  {
    auto store = sqlite_store();

    store->append_history(make_record(14));
    store->append_history(make_record(12));
    store->append_history(make_record(13));

    auto history = store->load_history();
    ASSERT_EQ(history.size(), 3U);
    EXPECT_EQ(history[0].date(), reference_date(12));
    EXPECT_EQ(history[1].date(), reference_date(13));
    EXPECT_EQ(history[2].date(), reference_date(14));
  }

  //! The store keeps whatever counters it is given, without knowing what they mean.
  TEST_F(SqliteStoreTest, unknown_counters_survive)
  {
    DailyStatsRecord record = make_record();
    record.misc_stats.push_back(4242);
    record.break_stats.emplace_back(std::vector<int64_t>{1, 2, 3});

    auto store = sqlite_store();
    store->save_today(record);

    auto today = sqlite_store()->load_today();
    ASSERT_TRUE(today.has_value());
    ASSERT_EQ(today->misc_stats.size(), 7U);
    EXPECT_EQ(today->misc_stats[6], 4242);
    ASSERT_EQ(today->break_stats.size(), 5U);
    EXPECT_EQ(today->break_stats[4][2], 3);
  }

  TEST_F(SqliteStoreTest, date_queries)
  {
    check_date_queries(sqlite_store());
  }

  TEST_F(SqliteStoreTest, delete_all)
  {
    auto store = sqlite_store();
    store->append_history(make_record(12));
    store->save_today(make_record(13));

    EXPECT_TRUE(store->delete_all());

    EXPECT_FALSE(store->load_today().has_value());
    EXPECT_TRUE(store->load_history().empty());
    EXPECT_TRUE(store->delete_all());
  }

  //! The counters must follow their day through the foreign key, not be orphaned.
  TEST_F(SqliteStoreTest, delete_all_leaves_no_counters_behind)
  {
    auto store = sqlite_store();
    store->append_history(make_record(12));
    store->save_today(make_record(13));

    EXPECT_GT(count_rows("stats_break"), 0);
    EXPECT_GT(count_rows("stats_misc"), 0);

    EXPECT_TRUE(store->delete_all());

    EXPECT_EQ(count_rows("stats_day"), 0);
    EXPECT_EQ(count_rows("stats_break"), 0);
    EXPECT_EQ(count_rows("stats_misc"), 0);
  }

  //! Working past midnight leaves a day whose stop time is on the next date.
  TEST_F(SqliteStoreTest, day_crossing_midnight)
  {
    using namespace std::chrono;

    DailyStatsRecord record = make_record(12);
    record.stop = to_local_time(reference_date(13), hours{1} + minutes{5});

    auto store = sqlite_store();
    store->save_today(record);

    auto today = sqlite_store()->load_today();
    ASSERT_TRUE(today.has_value());
    EXPECT_EQ(today->date(), reference_date(12));
    EXPECT_EQ(today->stop, to_local_time(reference_date(13), hours{1} + minutes{5}));
  }

  //----------------------------------------------------------------------------
  // Importing the statistics of an older Workrave.
  //----------------------------------------------------------------------------

  class MigrationTest : public StatisticsStoreTest
  {
  };

  TEST_F(MigrationTest, import_history_and_today)
  {
    auto text_store = file_store();
    text_store->append_history(make_record(12));
    text_store->append_history(make_record(13));
    text_store->save_today(make_record(14));

    auto store = sqlite_store();

    auto history = store->load_history();
    ASSERT_EQ(history.size(), 2U);
    EXPECT_EQ(history[0].date(), reference_date(12));
    EXPECT_EQ(history[1].date(), reference_date(13));

    auto today = store->load_today();
    ASSERT_TRUE(today.has_value());
    EXPECT_EQ(today->date(), reference_date(14));
    EXPECT_EQ(today->misc_stats[0], 100);
    EXPECT_EQ(today->break_stats[3][6], 28);
  }

  //! Downgrading to an older Workrave must not mean losing everything.
  TEST_F(MigrationTest, import_keeps_the_original_files)
  {
    auto text_store = file_store();
    text_store->append_history(make_record(12));
    text_store->save_today(make_record(13));

    auto store = sqlite_store();

    EXPECT_FALSE(exists("historystats"));
    EXPECT_FALSE(exists("todaystats"));
    EXPECT_TRUE(exists("historystats.bak"));
    EXPECT_TRUE(exists("todaystats.bak"));
  }

  //! Nothing to import is not a failure.
  TEST_F(MigrationTest, import_without_text_statistics)
  {
    auto store = sqlite_store();

    EXPECT_FALSE(store->load_today().has_value());
    EXPECT_TRUE(store->load_history().empty());
    EXPECT_FALSE(exists("historystats.bak"));
  }

  //! Text statistics are imported once, never again.
  TEST_F(MigrationTest, import_runs_only_once)
  {
    file_store()->append_history(make_record(12));

    EXPECT_EQ(sqlite_store()->load_history().size(), 1U);

    // As if an older Workrave had run again after the upgrade.
    file_store()->append_history(make_record(13));

    auto store = sqlite_store();
    EXPECT_EQ(store->load_history().size(), 1U);
    EXPECT_TRUE(exists("historystats"));
  }

  //! Statistics from before the import must survive it.
  TEST_F(MigrationTest, import_does_not_lose_counters)
  {
    write("historystats",
          "WorkRaveStats 4\n"
          "D 12 6 125 9 15 12 6 125 17 42\n"
          "B 2 7 15 16 17 18 19 20 21 \n"
          "m 6 100 200 300 400 500 600 \n");

    auto history = sqlite_store()->load_history();
    ASSERT_EQ(history.size(), 1U);
    ASSERT_EQ(history[0].break_stats.size(), 3U);
    EXPECT_EQ(history[0].break_stats[2][0], 15);
    EXPECT_EQ(history[0].break_stats[2][6], 21);
    ASSERT_EQ(history[0].misc_stats.size(), 6U);
    EXPECT_EQ(history[0].misc_stats[5], 600);
  }
} // namespace
