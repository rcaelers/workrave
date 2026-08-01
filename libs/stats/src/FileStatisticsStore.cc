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

#include "FileStatisticsStore.hh"

#include <fstream>
#include <sstream>
#include <string>
#include <utility>

#include <spdlog/spdlog.h>

namespace
{
  const char *const WORKRAVESTATS = "WorkRaveStats";
  const int STATSVERSION = 4;

  const size_t STATS_VALUE_TOTAL_ACTIVE_TIME = 0;

  //! Sanity limits, so that a corrupt file cannot make us allocate wildly.
  const int MAX_BREAKS = 64;
  const int MAX_COUNTERS = 256;

} // namespace

namespace workrave::stats
{
  FileStatisticsStore::FileStatisticsStore(std::filesystem::path state_directory)
    : state_directory(std::move(state_directory))
  {
  }

  std::filesystem::path FileStatisticsStore::today_path() const
  {
    return state_directory / "todaystats";
  }

  std::filesystem::path FileStatisticsStore::history_path() const
  {
    return state_directory / "historystats";
  }

  std::optional<DailyStatsRecord> FileStatisticsStore::load_today()
  {
    std::vector<DailyStatsRecord> records = load(today_path(), false);
    if (records.empty())
      {
        return std::nullopt;
      }
    return records.front();
  }

  std::vector<DailyStatsRecord> FileStatisticsStore::load_history()
  {
    return load(history_path(), true);
  }

  //! Verifies the "WorkRaveStats <version>" header and consumes it.
  bool FileStatisticsStore::read_header(std::istream &file)
  {
    if (!file.good())
      {
        return false;
      }

    std::string tag;
    file >> tag;
    if (tag != WORKRAVESTATS)
      {
        return false;
      }

    int version = 0;
    file >> version;

    return (version == STATSVERSION) || (version == 3);
  }

  //! Parses a 'D' record, the start and stop time that opens each day.
  DailyStatsRecord FileStatisticsStore::parse_day(std::istream &args)
  {
    std::tm start{};
    std::tm stop{};

    args >> start.tm_mday >> start.tm_mon >> start.tm_year >> start.tm_hour >> start.tm_min >> stop.tm_mday >> stop.tm_mon
      >> stop.tm_year >> stop.tm_hour >> stop.tm_min;

    DailyStatsRecord record;
    record.start = to_local_time(start);
    record.stop = to_local_time(stop);

    return record;
  }

  //! Parses a 'B' record, the counters of a single break.
  void FileStatisticsStore::parse_break(std::istream &args, DailyStatsRecord &record)
  {
    int break_id = 0;
    int size = 0;
    args >> break_id >> size;

    if (break_id < 0 || break_id >= MAX_BREAKS || size < 0 || size > MAX_COUNTERS)
      {
        spdlog::warn("ignoring malformed break statistics");
        return;
      }

    if (static_cast<size_t>(break_id) >= record.break_stats.size())
      {
        record.break_stats.resize(break_id + 1);
      }

    std::vector<int64_t> &break_stats = record.break_stats[break_id];
    break_stats.assign(size, 0);

    for (int i = 0; i < size; i++)
      {
        args >> break_stats[i];
      }
  }

  //! Parses an 'm' record, or the broken 'M' record that preceded it.
  void FileStatisticsStore::parse_misc(std::istream &args, DailyStatsRecord &record, bool valid)
  {
    int size = 0;
    args >> size;

    if (size < 0 || size > MAX_COUNTERS)
      {
        spdlog::warn("ignoring malformed statistics");
        return;
      }

    if (static_cast<size_t>(size) > record.misc_stats.size())
      {
        record.misc_stats.resize(size, 0);
      }

    for (int i = 0; i < size; i++)
      {
        int64_t value = 0;
        args >> value;

        // Ignore older 'M' stats, they are broken....
        record.misc_stats[i] = valid ? value : 0;
      }
  }

  //! Parses a legacy 'G' record, holding only the total active time.
  void FileStatisticsStore::parse_total_active(std::istream &args, DailyStatsRecord &record)
  {
    int64_t total_active = 0;
    args >> total_active;

    if (record.misc_stats.size() <= STATS_VALUE_TOTAL_ACTIVE_TIME)
      {
        record.misc_stats.resize(STATS_VALUE_TOTAL_ACTIVE_TIME + 1, 0);
      }
    record.misc_stats[STATS_VALUE_TOTAL_ACTIVE_TIME] = total_active;
  }

  //! Loads days from the specified file.
  std::vector<DailyStatsRecord> FileStatisticsStore::load(const std::filesystem::path &path, bool history) const
  {
    std::vector<DailyStatsRecord> records;

    std::ifstream file(path.string());

    if (!read_header(file))
      {
        return records;
      }

    std::string line;
    while (std::getline(file, line))
      {
        if (line.length() <= 1)
          {
            continue;
          }

        const char cmd = line[0];
        std::istringstream args(line.substr(1));

        if (cmd == 'D')
          {
            if (!history && !records.empty())
              {
                // Corrupt today stats.
                break;
              }

            records.push_back(parse_day(args));
          }
        else if (!records.empty())
          {
            DailyStatsRecord &record = records.back();

            if (cmd == 'B')
              {
                parse_break(args, record);
              }
            else if (cmd == 'M' || cmd == 'm')
              {
                parse_misc(args, record, cmd == 'm');
              }
            else if (cmd == 'G')
              {
                parse_total_active(args, record);
              }
          }
      }

    return records;
  }

  //! Writes the fields of a local time as the 'D' record stores them.
  void FileStatisticsStore::write_time(std::ostream &file, LocalTime time)
  {
    const Date date = date_of(time);
    const std::chrono::hh_mm_ss clock{time_of_day(time)};

    file << static_cast<unsigned>(date.day()) << " " << (static_cast<unsigned>(date.month()) - 1) << " "
         << (static_cast<int>(date.year()) - 1900) << " " << clock.hours().count() << " " << clock.minutes().count();
  }

  //! Writes a single day, in the format understood by Workrave 1.10 and later.
  void FileStatisticsStore::write_day(std::ostream &file, const DailyStatsRecord &record)
  {
    file << "D ";
    write_time(file, record.start);
    file << " ";
    write_time(file, record.stop);
    file << std::endl;

    for (size_t i = 0; i < record.break_stats.size(); i++)
      {
        const std::vector<int64_t> &break_stats = record.break_stats[i];

        file << "B " << i << " " << break_stats.size() << " ";
        for (int64_t value: break_stats)
          {
            file << value << " ";
          }
        file << std::endl;
      }

    file << "m " << record.misc_stats.size() << " ";
    for (int64_t value: record.misc_stats)
      {
        file << value << " ";
      }
    file << std::endl;
  }

  void FileStatisticsStore::save_today(const DailyStatsRecord &record)
  {
    std::error_code ec;
    std::filesystem::create_directories(state_directory, ec);

    const std::filesystem::path path = today_path();
    const std::filesystem::path tmp_path = std::filesystem::path(path) += ".tmp";

    {
      std::ofstream file(tmp_path.string());
      if (!file)
        {
          spdlog::warn("failed to write statistics to {}", tmp_path.string());
          return;
        }

      file << WORKRAVESTATS << " " << STATSVERSION << std::endl;
      write_day(file, record);
    }

    std::filesystem::rename(tmp_path, path, ec);
    if (ec)
      {
        spdlog::warn("failed to store statistics in {}: {}", path.string(), ec.message());
      }
  }

  void FileStatisticsStore::append_history(const DailyStatsRecord &record)
  {
    std::error_code ec;
    std::filesystem::create_directories(state_directory, ec);

    const std::filesystem::path path = history_path();
    const bool exists = std::filesystem::is_regular_file(path);

    std::ofstream file(path.string(), std::ios::app);
    if (!file)
      {
        spdlog::warn("failed to archive statistics in {}", path.string());
        return;
      }

    if (!exists)
      {
        file << WORKRAVESTATS << " " << STATSVERSION << std::endl;
      }

    write_day(file, record);
  }

  //! Every day this store holds, today included, by date.
  std::map<Date, DailyStatsRecord> FileStatisticsStore::load_all()
  {
    std::map<Date, DailyStatsRecord> days;

    for (const DailyStatsRecord &record: load_history())
      {
        days[record.date()] = record;
      }

    // A day can be in both files if Workrave stopped between archiving it and
    // starting the next day. The day in progress is the newer of the two.
    std::optional<DailyStatsRecord> today = load_today();
    if (today.has_value())
      {
        days[today->date()] = today.value();
      }

    return days;
  }

  std::optional<DailyStatsRecord> FileStatisticsStore::load_date(const Date &date)
  {
    const std::map<Date, DailyStatsRecord> days = load_all();

    auto it = days.find(date);
    if (it == days.end())
      {
        return std::nullopt;
      }

    return it->second;
  }

  std::vector<Date> FileStatisticsStore::get_dates(const Date &from, const Date &to)
  {
    std::vector<Date> dates;

    for (const auto &[date, record]: load_all())
      {
        if (date >= from && date <= to)
          {
            dates.push_back(date);
          }
      }

    return dates;
  }

  std::optional<Date> FileStatisticsStore::get_previous_date(const Date &date)
  {
    const std::map<Date, DailyStatsRecord> days = load_all();

    auto it = days.lower_bound(date);
    if (it == days.begin())
      {
        return std::nullopt;
      }

    return std::prev(it)->first;
  }

  std::optional<Date> FileStatisticsStore::get_next_date(const Date &date)
  {
    const std::map<Date, DailyStatsRecord> days = load_all();

    auto it = days.upper_bound(date);
    if (it == days.end())
      {
        return std::nullopt;
      }

    return it->first;
  }

  std::optional<Date> FileStatisticsStore::get_first_date()
  {
    const std::map<Date, DailyStatsRecord> days = load_all();
    if (days.empty())
      {
        return std::nullopt;
      }

    return days.begin()->first;
  }

  std::optional<Date> FileStatisticsStore::get_last_date()
  {
    const std::map<Date, DailyStatsRecord> days = load_all();
    if (days.empty())
      {
        return std::nullopt;
      }

    return days.rbegin()->first;
  }

  int64_t FileStatisticsStore::get_total_misc(int counter, const Date &from, const Date &to)
  {
    int64_t total = 0;

    for (const auto &[date, record]: load_all())
      {
        if (date >= from && date <= to && static_cast<size_t>(counter) < record.misc_stats.size())
          {
            total += record.misc_stats[counter];
          }
      }

    return total;
  }

  bool FileStatisticsStore::delete_all()
  {
    bool ok = true;

    for (const std::filesystem::path &path: {history_path(), today_path()})
      {
        std::error_code ec;
        if (std::filesystem::is_regular_file(path) && !std::filesystem::remove(path, ec))
          {
            spdlog::warn("failed to remove {}: {}", path.string(), ec.message());
            ok = false;
          }
      }

    return ok;
  }
} // namespace workrave::stats
