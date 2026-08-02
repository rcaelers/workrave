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

#include "SqliteStatisticsStore.hh"

#include <iomanip>
#include <sstream>
#include <utility>

#include <spdlog/spdlog.h>

#include "FileStatisticsStore.hh"

namespace
{
  const int SCHEMA_VERSION = 1;

  const char *const PROPERTY_SCHEMA_VERSION = "schema_version";
  const char *const PROPERTY_TODAY = "today";

  //! The mtime (raw file_clock ticks) of historystats/todaystats at the time
  //! each was last imported. Re-checked on every open() so that new data
  //! written by an older Workrave, after a downgrade, is picked up again.
  const char *const PROPERTY_IMPORTED_HISTORY_MTIME = "imported_historystats_mtime";
  const char *const PROPERTY_IMPORTED_TODAY_MTIME = "imported_todaystats_mtime";

  //! The mtime of a file, in raw file_clock ticks, or nullopt if it does not
  //! exist. file_clock's epoch is not guaranteed to align with any other
  //! clock, so ticks can legitimately be negative: they may only be compared
  //! to other ticks from this same clock, never against a sentinel like 0.
  std::optional<int64_t>
  mtime_ticks(const std::filesystem::path &path)
  {
    std::error_code ec;
    const auto time = std::filesystem::last_write_time(path, ec);
    if (ec)
      {
        return std::nullopt;
      }
    return time.time_since_epoch().count();
  }

  //! Formats a calendar date as 'YYYY-MM-DD', which sorts chronologically.
  std::string
  to_day(const workrave::stats::Date &date)
  {
    std::ostringstream ss;
    ss << std::setfill('0') << std::setw(4) << static_cast<int>(date.year()) << "-" << std::setw(2)
       << static_cast<unsigned>(date.month()) << "-" << std::setw(2) << static_cast<unsigned>(date.day());
    return ss.str();
  }

  std::string
  to_day(workrave::stats::LocalTime time)
  {
    return to_day(workrave::stats::date_of(time));
  }

  //! Reads back a 'YYYY-MM-DD' day. An unreadable day comes back as a date that is not ok().
  workrave::stats::Date
  to_date(const std::string &day)
  {
    std::istringstream ss(day);
    int year = 0;
    unsigned month = 0;
    unsigned mday = 0;
    char dash = 0;

    ss >> year >> dash >> month >> dash >> mday;
    if (ss.fail())
      {
        return {};
      }

    return std::chrono::year{year} / month / mday;
  }

  //! Minimal owner of a prepared statement.
  class Statement
  {
  public:
    Statement(sqlite3 *db, const char *sql)
    {
      if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
          spdlog::error("failed to prepare statistics query: {}", sqlite3_errmsg(db));
          stmt = nullptr;
        }
    }

    ~Statement()
    {
      sqlite3_finalize(stmt);
    }

    Statement(const Statement &) = delete;
    Statement &operator=(const Statement &) = delete;
    Statement(Statement &&) = delete;
    Statement &operator=(Statement &&) = delete;

    explicit operator bool() const
    {
      return stmt != nullptr;
    }

    void bind(int index, int64_t value)
    {
      sqlite3_bind_int64(stmt, index, value);
    }

    void bind(int index, const std::string &value)
    {
      sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT);
    }

    //! Advances to the next row. Returns false at the end of the result, or on error.
    bool step()
    {
      return sqlite3_step(stmt) == SQLITE_ROW;
    }

    //! Runs a statement that returns no rows, leaving it ready to be bound again.
    bool run()
    {
      const bool ok = sqlite3_step(stmt) == SQLITE_DONE;
      sqlite3_reset(stmt);
      return ok;
    }

    [[nodiscard]] int64_t column_int(int index) const
    {
      return sqlite3_column_int64(stmt, index);
    }

    [[nodiscard]] std::string column_text(int index) const
    {
      const auto *text = sqlite3_column_text(stmt, index);
      return text != nullptr ? reinterpret_cast<const char *>(text) : std::string{};
    }

  private:
    sqlite3_stmt *stmt{nullptr};
  };

  //! Rolls back on destruction unless committed.
  class Transaction
  {
  public:
    explicit Transaction(sqlite3 *db)
      : db(db)
    {
      active = sqlite3_exec(db, "BEGIN IMMEDIATE", nullptr, nullptr, nullptr) == SQLITE_OK;
    }

    ~Transaction()
    {
      if (active)
        {
          sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        }
    }

    Transaction(const Transaction &) = delete;
    Transaction &operator=(const Transaction &) = delete;
    Transaction(Transaction &&) = delete;
    Transaction &operator=(Transaction &&) = delete;

    explicit operator bool() const
    {
      return active;
    }

    bool commit()
    {
      if (!active)
        {
          return false;
        }
      active = false;
      return sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr) == SQLITE_OK;
    }

  private:
    sqlite3 *db;
    bool active{false};
  };
} // namespace

namespace workrave::stats
{
  SqliteStatisticsStore::SqliteStatisticsStore(std::filesystem::path state_directory)
    : state_directory(std::move(state_directory))
  {
  }

  SqliteStatisticsStore::~SqliteStatisticsStore()
  {
    sqlite3_close(db);
  }

  std::filesystem::path
  SqliteStatisticsStore::database_path() const
  {
    return state_directory / "statistics.db";
  }

  bool
  SqliteStatisticsStore::open()
  {
    std::error_code ec;
    std::filesystem::create_directories(state_directory, ec);

    const std::filesystem::path path = database_path();
    if (sqlite3_open_v2(path.string().c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK)
      {
        spdlog::error("failed to open {}: {}", path.string(), sqlite3_errmsg(db));
        sqlite3_close(db);
        db = nullptr;
        return false;
      }

    execute("PRAGMA busy_timeout = 5000");
    execute("PRAGMA foreign_keys = ON");

    // WAL is the better journal mode, but it does not work on every file
    // system a home directory can live on. SQLite reports the mode it ended up
    // with rather than failing, so simply carry on with whatever we get.
    execute("PRAGMA journal_mode = WAL");
    execute("PRAGMA synchronous = NORMAL");

    if (!create_schema())
      {
        sqlite3_close(db);
        db = nullptr;
        return false;
      }

    return import_text_statistics();
  }

  bool
  SqliteStatisticsStore::execute(const char *sql)
  {
    char *error = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &error) != SQLITE_OK)
      {
        spdlog::error("statistics database error: {}", error != nullptr ? error : "unknown");
        sqlite3_free(error);
        return false;
      }
    return true;
  }

  bool
  SqliteStatisticsStore::create_schema()
  {
    const bool ok = execute("CREATE TABLE IF NOT EXISTS schema_info ("
                            "  key   TEXT PRIMARY KEY,"
                            "  value TEXT NOT NULL);"
                            // start_time and stop_time count seconds from local midnight of
                            // their own day, so that no time zone is needed to read them back.
                            "CREATE TABLE IF NOT EXISTS stats_day ("
                            "  day        TEXT    PRIMARY KEY,"
                            "  start_time INTEGER NOT NULL,"
                            "  stop_day   TEXT    NOT NULL,"
                            "  stop_time  INTEGER NOT NULL);"
                            "CREATE TABLE IF NOT EXISTS stats_break ("
                            "  day      TEXT    NOT NULL REFERENCES stats_day(day) ON DELETE CASCADE,"
                            "  break_id INTEGER NOT NULL,"
                            "  counter  INTEGER NOT NULL,"
                            "  value    INTEGER NOT NULL,"
                            "  PRIMARY KEY (day, break_id, counter)) WITHOUT ROWID;"
                            "CREATE TABLE IF NOT EXISTS stats_misc ("
                            "  day     TEXT    NOT NULL REFERENCES stats_day(day) ON DELETE CASCADE,"
                            "  counter INTEGER NOT NULL,"
                            "  value   INTEGER NOT NULL,"
                            "  PRIMARY KEY (day, counter)) WITHOUT ROWID;");
    if (!ok)
      {
        return false;
      }

    if (!get_property(PROPERTY_SCHEMA_VERSION).has_value())
      {
        return set_property(PROPERTY_SCHEMA_VERSION, std::to_string(SCHEMA_VERSION));
      }

    return true;
  }

  //! Imports the statistics of a Workrave that still used the text format.
  /*!
   *  The text files are left untouched, under their original names, so that
   *  going back to an older Workrave keeps working exactly as before. Because
   *  they are never renamed away, this re-checks their mtime on every open():
   *  if someone downgrades, lets an older Workrave add to them, and then
   *  upgrades again, that new data gets imported too. A day already present in
   *  the database is left alone rather than overwritten: it can only be there
   *  because this Workrave has already counted it, live, more accurately than
   *  whatever a legacy text snapshot has to offer.
   */
  bool
  SqliteStatisticsStore::import_text_statistics()
  {
    const std::filesystem::path history_path = state_directory / "historystats";
    const std::filesystem::path today_path = state_directory / "todaystats";

    const std::optional<int64_t> history_mtime = mtime_ticks(history_path);
    const std::optional<int64_t> today_mtime = mtime_ticks(today_path);

    if (!history_mtime.has_value() && !today_mtime.has_value())
      {
        // Neither text file exists: nothing to import.
        return true;
      }

    // A file counts as changed if it exists and its mtime does not match what
    // was recorded the last time it was imported (including never having been
    // recorded at all).
    auto changed = [this](const char *property, std::optional<int64_t> current) {
      if (!current.has_value())
        {
          return false;
        }
      const std::optional<std::string> stored = get_property(property);
      return !stored.has_value() || std::stoll(stored.value()) != current.value();
    };

    if (!changed(PROPERTY_IMPORTED_HISTORY_MTIME, history_mtime) && !changed(PROPERTY_IMPORTED_TODAY_MTIME, today_mtime))
      {
        return true;
      }

    FileStatisticsStore text_store(state_directory);
    const std::vector<DailyStatsRecord> history = text_store.load_history();
    const std::optional<DailyStatsRecord> today = text_store.load_today();

    {
      Transaction transaction(db);
      if (!transaction)
        {
          return false;
        }

      for (const DailyStatsRecord &record: history)
        {
          if (has_day(record.date()))
            {
              continue;
            }
          if (!store_day(record))
            {
              return false;
            }
        }

      if (today.has_value() && !has_day(today->date()))
        {
          if (!store_day(today.value()) || !set_property(PROPERTY_TODAY, to_day(today->date())))
            {
              return false;
            }
        }

      const bool history_recorded = !history_mtime.has_value() || set_property(PROPERTY_IMPORTED_HISTORY_MTIME,
                                                                                std::to_string(history_mtime.value()));
      const bool today_recorded =
        !today_mtime.has_value() || set_property(PROPERTY_IMPORTED_TODAY_MTIME, std::to_string(today_mtime.value()));

      if (!history_recorded || !today_recorded || !transaction.commit())
        {
          return false;
        }
    }

    if (!history.empty() || today.has_value())
      {
        spdlog::info("imported {} days of statistics", history.size() + (today.has_value() ? 1 : 0));
      }

    return true;
  }

  std::optional<std::string>
  SqliteStatisticsStore::get_property(const std::string &key)
  {
    Statement statement(db, "SELECT value FROM schema_info WHERE key = ?");
    if (!statement)
      {
        return std::nullopt;
      }

    statement.bind(1, key);
    if (!statement.step())
      {
        return std::nullopt;
      }

    return statement.column_text(0);
  }

  bool
  SqliteStatisticsStore::set_property(const std::string &key, const std::string &value)
  {
    Statement statement(db, "INSERT INTO schema_info (key, value) VALUES (?, ?) ON CONFLICT (key) DO UPDATE SET value = ?2");
    if (!statement)
      {
        return false;
      }

    statement.bind(1, key);
    statement.bind(2, value);
    return statement.run();
  }

  //! Whether the given day already has a row in the database.
  bool
  SqliteStatisticsStore::has_day(const Date &date)
  {
    Statement statement(db, "SELECT 1 FROM stats_day WHERE day = ?");
    if (!statement)
      {
        return false;
      }

    statement.bind(1, to_day(date));
    return statement.step();
  }

  //! Writes a day and its counters, replacing whatever was stored for that date.
  bool
  SqliteStatisticsStore::store_day(const DailyStatsRecord &record)
  {
    const std::string day = to_day(record.date());

    {
      Statement statement(db,
                          "INSERT INTO stats_day (day, start_time, stop_day, stop_time)"
                          " VALUES (?, ?, ?, ?)"
                          " ON CONFLICT (day) DO UPDATE SET"
                          "   start_time = ?2, stop_day = ?3, stop_time = ?4");
      if (!statement)
        {
          return false;
        }

      statement.bind(1, day);
      statement.bind(2, time_of_day(record.start).count());
      statement.bind(3, to_day(record.stop));
      statement.bind(4, time_of_day(record.stop).count());
      if (!statement.run())
        {
          spdlog::error("failed to store statistics of {}: {}", day, sqlite3_errmsg(db));
          return false;
        }
    }

    // Replace rather than update, so that counters that are no longer reported
    // do not linger.
    for (const char *sql: {"DELETE FROM stats_break WHERE day = ?", "DELETE FROM stats_misc WHERE day = ?"})
      {
        Statement statement(db, sql);
        if (!statement)
          {
            return false;
          }
        statement.bind(1, day);
        if (!statement.run())
          {
            return false;
          }
      }

    {
      Statement statement(db, "INSERT INTO stats_break (day, break_id, counter, value) VALUES (?, ?, ?, ?)");
      if (!statement)
        {
          return false;
        }

      for (size_t break_id = 0; break_id < record.break_stats.size(); break_id++)
        {
          const std::vector<int64_t> &break_stats = record.break_stats[break_id];
          for (size_t counter = 0; counter < break_stats.size(); counter++)
            {
              statement.bind(1, day);
              statement.bind(2, static_cast<int64_t>(break_id));
              statement.bind(3, static_cast<int64_t>(counter));
              statement.bind(4, break_stats[counter]);
              if (!statement.run())
                {
                  return false;
                }
            }
        }
    }

    {
      Statement statement(db, "INSERT INTO stats_misc (day, counter, value) VALUES (?, ?, ?)");
      if (!statement)
        {
          return false;
        }

      for (size_t counter = 0; counter < record.misc_stats.size(); counter++)
        {
          statement.bind(1, day);
          statement.bind(2, static_cast<int64_t>(counter));
          statement.bind(3, record.misc_stats[counter]);
          if (!statement.run())
            {
              return false;
            }
        }
    }

    return true;
  }

  //! Loads a single day and its counters.
  std::optional<DailyStatsRecord>
  SqliteStatisticsStore::load_day(const std::string &day)
  {
    DailyStatsRecord record;

    {
      Statement statement(db, "SELECT start_time, stop_day, stop_time FROM stats_day WHERE day = ?");
      if (!statement)
        {
          return std::nullopt;
        }

      statement.bind(1, day);
      if (!statement.step())
        {
          return std::nullopt;
        }

      record.start = to_local_time(to_date(day), std::chrono::seconds{statement.column_int(0)});
      record.stop = to_local_time(to_date(statement.column_text(1)), std::chrono::seconds{statement.column_int(2)});
    }

    {
      Statement statement(db, "SELECT break_id, counter, value FROM stats_break WHERE day = ? ORDER BY break_id, counter");
      if (!statement)
        {
          return std::nullopt;
        }

      statement.bind(1, day);
      while (statement.step())
        {
          const auto break_id = static_cast<size_t>(statement.column_int(0));
          const auto counter = static_cast<size_t>(statement.column_int(1));

          if (break_id >= record.break_stats.size())
            {
              record.break_stats.resize(break_id + 1);
            }
          if (counter >= record.break_stats[break_id].size())
            {
              record.break_stats[break_id].resize(counter + 1, 0);
            }
          record.break_stats[break_id][counter] = statement.column_int(2);
        }
    }

    {
      Statement statement(db, "SELECT counter, value FROM stats_misc WHERE day = ? ORDER BY counter");
      if (!statement)
        {
          return std::nullopt;
        }

      statement.bind(1, day);
      while (statement.step())
        {
          const auto counter = static_cast<size_t>(statement.column_int(0));
          if (counter >= record.misc_stats.size())
            {
              record.misc_stats.resize(counter + 1, 0);
            }
          record.misc_stats[counter] = statement.column_int(1);
        }
    }

    return record;
  }

  std::optional<DailyStatsRecord>
  SqliteStatisticsStore::load_today()
  {
    const std::optional<std::string> today = get_property(PROPERTY_TODAY);
    if (!today.has_value())
      {
        return std::nullopt;
      }

    return load_day(today.value());
  }

  //! Returns the days that are not the day in progress, oldest first.
  std::vector<std::string>
  SqliteStatisticsStore::load_days()
  {
    std::vector<std::string> days;

    Statement statement(db, "SELECT day FROM stats_day ORDER BY day");
    if (!statement)
      {
        return days;
      }

    while (statement.step())
      {
        days.push_back(statement.column_text(0));
      }

    return days;
  }

  std::vector<DailyStatsRecord>
  SqliteStatisticsStore::load_history()
  {
    std::vector<DailyStatsRecord> records;

    const std::optional<std::string> today = get_property(PROPERTY_TODAY);

    for (const std::string &day: load_days())
      {
        if (today.has_value() && day == today.value())
          {
            continue;
          }

        std::optional<DailyStatsRecord> record = load_day(day);
        if (record.has_value())
          {
            records.push_back(record.value());
          }
      }

    return records;
  }

  void
  SqliteStatisticsStore::save_today(const DailyStatsRecord &record)
  {
    Transaction transaction(db);
    if (!transaction)
      {
        return;
      }

    if (store_day(record) && set_property(PROPERTY_TODAY, to_day(record.date())))
      {
        transaction.commit();
      }
  }

  void
  SqliteStatisticsStore::set_break_counter(BreakId break_id, int counter, int64_t value)
  {
    const std::optional<std::string> today = get_property(PROPERTY_TODAY);
    if (!today.has_value())
      {
        return;
      }

    Statement statement(db,
                        "INSERT INTO stats_break (day, break_id, counter, value) VALUES (?, ?, ?, ?)"
                        " ON CONFLICT (day, break_id, counter) DO UPDATE SET value = ?4");
    if (!statement)
      {
        return;
      }

    statement.bind(1, today.value());
    statement.bind(2, static_cast<int64_t>(break_id));
    statement.bind(3, static_cast<int64_t>(counter));
    statement.bind(4, value);
    if (!statement.run())
      {
        spdlog::error("failed to update break counter of {}: {}", today.value(), sqlite3_errmsg(db));
      }
  }

  void
  SqliteStatisticsStore::append_history(const DailyStatsRecord &record)
  {
    Transaction transaction(db);
    if (!transaction)
      {
        return;
      }

    // The day is archived by storing it and no longer calling it today; the
    // next save_today() names the new day.
    if (store_day(record))
      {
        transaction.commit();
      }
  }

  std::optional<DailyStatsRecord>
  SqliteStatisticsStore::load_date(const Date &date)
  {
    return load_day(to_day(date));
  }

  std::vector<Date>
  SqliteStatisticsStore::get_dates(const Date &from, const Date &to)
  {
    std::vector<Date> dates;

    Statement statement(db, "SELECT day FROM stats_day WHERE day BETWEEN ? AND ? ORDER BY day");
    if (!statement)
      {
        return dates;
      }

    statement.bind(1, to_day(from));
    statement.bind(2, to_day(to));
    while (statement.step())
      {
        dates.push_back(to_date(statement.column_text(0)));
      }

    return dates;
  }

  //! Runs a query that selects at most one day, optionally relative to another.
  std::optional<Date>
  SqliteStatisticsStore::query_date(const char *sql, const std::optional<std::string> &day)
  {
    Statement statement(db, sql);
    if (!statement)
      {
        return std::nullopt;
      }

    if (day.has_value())
      {
        statement.bind(1, day.value());
      }

    if (!statement.step())
      {
        return std::nullopt;
      }

    return to_date(statement.column_text(0));
  }

  std::optional<Date>
  SqliteStatisticsStore::get_previous_date(const Date &date)
  {
    return query_date("SELECT day FROM stats_day WHERE day < ? ORDER BY day DESC LIMIT 1", to_day(date));
  }

  std::optional<Date>
  SqliteStatisticsStore::get_next_date(const Date &date)
  {
    return query_date("SELECT day FROM stats_day WHERE day > ? ORDER BY day LIMIT 1", to_day(date));
  }

  std::optional<Date>
  SqliteStatisticsStore::get_first_date()
  {
    return query_date("SELECT day FROM stats_day ORDER BY day LIMIT 1", std::nullopt);
  }

  std::optional<Date>
  SqliteStatisticsStore::get_last_date()
  {
    return query_date("SELECT day FROM stats_day ORDER BY day DESC LIMIT 1", std::nullopt);
  }

  int64_t
  SqliteStatisticsStore::get_total_misc(int counter, const Date &from, const Date &to)
  {
    Statement statement(db, "SELECT SUM(value) FROM stats_misc WHERE counter = ? AND day BETWEEN ? AND ?");
    if (!statement)
      {
        return 0;
      }

    statement.bind(1, counter);
    statement.bind(2, to_day(from));
    statement.bind(3, to_day(to));

    // SUM() over no rows is NULL, which reads back as zero.
    return statement.step() ? statement.column_int(0) : 0;
  }

  bool
  SqliteStatisticsStore::delete_all()
  {
    Transaction transaction(db);
    if (!transaction)
      {
        return false;
      }

    // stats_break and stats_misc follow through the foreign key.
    if (!execute("DELETE FROM stats_day") || !execute("DELETE FROM schema_info WHERE key = 'today'"))
      {
        return false;
      }

    return transaction.commit();
  }
} // namespace workrave::stats
