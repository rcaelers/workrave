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
  class FileStoreTest : public StatisticsStoreTest
  {
  };

  TEST_F(FileStoreTest, load_missing_today)
  {
    EXPECT_FALSE(file_store()->load_today().has_value());
  }

  TEST_F(FileStoreTest, load_missing_history)
  {
    EXPECT_TRUE(file_store()->load_history().empty());
  }

  TEST_F(FileStoreTest, load_today)
  {
    write("todaystats", REFERENCE_STATS);

    auto record = file_store()->load_today();
    ASSERT_TRUE(record.has_value());

    using namespace std::chrono;
    EXPECT_EQ(record->start, to_local_time(reference_date(12), hours{9} + minutes{15}));
    EXPECT_EQ(record->stop, to_local_time(reference_date(12), hours{17} + minutes{42}));

    ASSERT_EQ(record->break_stats.size(), 4U);
    ASSERT_EQ(record->break_stats[0].size(), 7U);
    EXPECT_EQ(record->break_stats[0][0], 1);
    EXPECT_EQ(record->break_stats[3][6], 28);

    ASSERT_EQ(record->misc_stats.size(), 6U);
    EXPECT_EQ(record->misc_stats[0], 100);
    EXPECT_EQ(record->misc_stats[5], 600);
  }

  //! The bytes written must stay readable by 1.10 and 1.11.
  TEST_F(FileStoreTest, save_today_is_format_compatible)
  {
    file_store()->save_today(make_record());

    EXPECT_EQ(read("todaystats"), REFERENCE_STATS);
  }

  TEST_F(FileStoreTest, save_today_replaces_previous_day)
  {
    auto store = file_store();
    store->save_today(make_record());
    store->save_today(make_record());

    EXPECT_EQ(read("todaystats"), REFERENCE_STATS);
    EXPECT_FALSE(exists("todaystats.tmp"));
  }

  TEST_F(FileStoreTest, append_history_writes_header_once)
  {
    auto store = file_store();
    store->append_history(make_record());
    store->append_history(make_record());

    const std::string expected = std::string(REFERENCE_STATS)
                                 + std::string(REFERENCE_STATS).substr(strlen("WorkRaveStats 4\n"));
    EXPECT_EQ(read("historystats"), expected);

    EXPECT_EQ(store->load_history().size(), 2U);
  }

  TEST_F(FileStoreTest, round_trip)
  {
    const DailyStatsRecord original = make_record();
    auto store = file_store();
    store->save_today(original);

    auto record = store->load_today();
    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->break_stats, original.break_stats);
    EXPECT_EQ(record->misc_stats, original.misc_stats);
    EXPECT_EQ(record->start, original.start);
    EXPECT_EQ(record->stop, original.stop);
  }

  //! A version we do not know about must not be interpreted.
  TEST_F(FileStoreTest, load_rejects_unknown_version)
  {
    write("todaystats", "WorkRaveStats 99\nD 12 6 125 9 15 12 6 125 17 42\n");

    EXPECT_FALSE(file_store()->load_today().has_value());
  }

  TEST_F(FileStoreTest, load_rejects_unknown_tag)
  {
    write("todaystats", "SomethingElse 4\nD 12 6 125 9 15 12 6 125 17 42\n");

    EXPECT_FALSE(file_store()->load_today().has_value());
  }

  //! Version 3 is still read, including its 'G' total active time record.
  TEST_F(FileStoreTest, load_legacy_version_3)
  {
    write("todaystats", "WorkRaveStats 3\nD 12 6 125 9 15 12 6 125 17 42\nG 4242\n");

    auto record = file_store()->load_today();
    ASSERT_TRUE(record.has_value());
    ASSERT_FALSE(record->misc_stats.empty());
    EXPECT_EQ(record->misc_stats[0], 4242);
  }

  //! The values of the old 'M' record are known to be wrong and are discarded.
  TEST_F(FileStoreTest, load_discards_broken_misc_record)
  {
    write("todaystats", "WorkRaveStats 4\nD 12 6 125 9 15 12 6 125 17 42\nM 3 7 8 9 \n");

    auto record = file_store()->load_today();
    ASSERT_TRUE(record.has_value());
    ASSERT_EQ(record->misc_stats.size(), 3U);
    EXPECT_EQ(record->misc_stats[0], 0);
    EXPECT_EQ(record->misc_stats[2], 0);
  }

  //! More than one day in todaystats means the file is corrupt.
  TEST_F(FileStoreTest, load_today_stops_at_second_day)
  {
    write("todaystats",
          "WorkRaveStats 4\n"
          "D 12 6 125 9 15 12 6 125 17 42\n"
          "D 13 6 125 9 15 13 6 125 17 42\n");

    auto record = file_store()->load_today();
    ASSERT_TRUE(record.has_value());
    EXPECT_EQ(record->date(), reference_date(12));
  }

  //! A break id outside any sane range must not be trusted.
  TEST_F(FileStoreTest, load_ignores_out_of_range_break_id)
  {
    write("todaystats",
          "WorkRaveStats 4\n"
          "D 12 6 125 9 15 12 6 125 17 42\n"
          "B 99999 7 1 2 3 4 5 6 7 \n"
          "B -1 7 1 2 3 4 5 6 7 \n");

    auto record = file_store()->load_today();
    ASSERT_TRUE(record.has_value());
    EXPECT_TRUE(record->break_stats.empty());
  }

  TEST_F(FileStoreTest, load_history_returns_days_in_file_order)
  {
    write("historystats",
          "WorkRaveStats 4\n"
          "D 12 6 125 9 15 12 6 125 17 42\n"
          "m 1 11 \n"
          "D 13 6 125 9 15 13 6 125 17 42\n"
          "m 1 22 \n");

    auto history = file_store()->load_history();
    ASSERT_EQ(history.size(), 2U);
    EXPECT_EQ(history[0].date(), reference_date(12));
    EXPECT_EQ(history[0].misc_stats[0], 11);
    EXPECT_EQ(history[1].date(), reference_date(13));
    EXPECT_EQ(history[1].misc_stats[0], 22);
  }

  TEST_F(FileStoreTest, date_queries)
  {
    check_date_queries(file_store());
  }

  TEST_F(FileStoreTest, delete_all)
  {
    auto store = file_store();
    store->save_today(make_record());
    store->append_history(make_record());

    EXPECT_TRUE(store->delete_all());

    EXPECT_FALSE(exists("todaystats"));
    EXPECT_FALSE(exists("historystats"));
    EXPECT_TRUE(store->delete_all());
  }
} // namespace
