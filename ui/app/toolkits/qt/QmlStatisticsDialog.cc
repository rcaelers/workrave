// Copyright (C) 2025 Rob Caelers <rob.caelers@gmail.com>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "QmlStatisticsDialog.hh"

#include <algorithm>
#include <array>
#include <cstring>

#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QObject>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QUrl>

#include "debug.hh"

using namespace workrave;

// ── StatisticsBridge ──────────────────────────────────────────────────────────

StatisticsBridge::StatisticsBridge(workrave::IStatistics::Ptr statistics,
                                   std::shared_ptr<IApplicationContext> app,
                                   QObject *parent)
  : QObject(parent)
  , statistics_(std::move(statistics))
  , app_(std::move(app))
{
  statistics_->update();

  QDate today = QDate::currentDate();
  calendar_year_  = today.year();
  calendar_month_ = today.month();

  // Seed empty cached values
  clearStats();

  // Navigate to the most recent history entry
  goLast();
}

// ── calendarMonthYearText ─────────────────────────────────────────────────────

QString
StatisticsBridge::calendarMonthYearText() const
{
  QLocale locale;
  QDate d(calendar_year_, calendar_month_, 1);
  return locale.toString(d, QStringLiteral("MMMM yyyy"));
}

// ── calendarCells ─────────────────────────────────────────────────────────────

QVariantList
StatisticsBridge::calendarCells() const
{
  QDate first(calendar_year_, calendar_month_, 1);
  int days_in_month = first.daysInMonth();
  // Qt: dayOfWeek() → 1=Mon … 7=Sun; we want Mon as column 0.
  int offset = first.dayOfWeek() - 1; // 0-based, Mon=0

  // Collect which days have data
  QSet<int> days_with_data;
  for (int d = 1; d <= days_in_month; d++)
    {
      int idx  = 0;
      int next = 0;
      int prev = 0;
      statistics_->get_day_index_by_date(calendar_year_, calendar_month_, d, idx, next, prev);
      if (idx >= 0)
        {
          days_with_data.insert(d);
        }
    }

  QVariantList cells;
  int total = offset + days_in_month;
  int rows  = (total + 6) / 7; // ceil to multiple of 7
  int count = rows * 7;

  for (int i = 0; i < count; i++)
    {
      QVariantMap cell;
      int day = (i < offset) ? 0 : (i - offset + 1);
      if (day > days_in_month)
        {
          day = 0;
        }
      cell[QStringLiteral("day")]        = day;
      cell[QStringLiteral("hasData")]    = (day > 0) && days_with_data.contains(day);
      cell[QStringLiteral("isSelected")] = (day > 0)
                                           && (calendar_year_  == selected_year_)
                                           && (calendar_month_ == selected_month_)
                                           && (day             == selected_day_);
      cells.append(cell);
    }

  return cells;
}

// ── Navigation ────────────────────────────────────────────────────────────────

void
StatisticsBridge::goBack()
{
  if (current_prev_ >= 0)
    {
      selectDayIndex(current_prev_);
    }
}

void
StatisticsBridge::goForward()
{
  if (current_next_ >= 0)
    {
      selectDayIndex(current_next_);
    }
}

void
StatisticsBridge::goFirst()
{
  int size = statistics_->get_history_size();
  if (size >= 0)
    {
      selectDayIndex(size);
    }
}

void
StatisticsBridge::goLast()
{
  int size = statistics_->get_history_size();
  if (size >= 0)
    {
      selectDayIndex(0);
    }
}

void
StatisticsBridge::selectDate(int y, int m, int d)
{
  int idx  = 0;
  int next = 0;
  int prev = 0;
  statistics_->get_day_index_by_date(y, m, d, idx, next, prev);
  if (idx >= 0)
    {
      selectDayIndex(idx);
    }
  else
    {
      selected_year_  = y;
      selected_month_ = m;
      selected_day_   = d;
      current_idx_    = -1;
      current_next_   = next;
      current_prev_   = prev;
      clearStats();
      Q_EMIT calendarChanged();
      Q_EMIT dataChanged();
      Q_EMIT navChanged();
    }
}

void
StatisticsBridge::selectDayIndex(int idx)
{
  IStatistics::DailyStats *stats = statistics_->get_day(idx);
  if (stats == nullptr)
    {
      return;
    }

  int next = 0;
  int prev = 0;
  statistics_->get_day_index_by_date(stats->start.tm_year + 1900,
                                      stats->start.tm_mon + 1,
                                      stats->start.tm_mday,
                                      idx,
                                      next,
                                      prev);

  current_idx_  = idx;
  current_next_ = next;
  current_prev_ = prev;

  selected_year_  = stats->start.tm_year + 1900;
  selected_month_ = stats->start.tm_mon + 1;
  selected_day_   = stats->start.tm_mday;

  // Flip the calendar page if needed
  bool page_changed = (calendar_year_ != selected_year_) || (calendar_month_ != selected_month_);
  if (page_changed)
    {
      calendar_year_  = selected_year_;
      calendar_month_ = selected_month_;
    }

  updateStats();

  Q_EMIT calendarChanged();
  Q_EMIT dataChanged();
  Q_EMIT navChanged();
}

// ── prevMonth / nextMonth ─────────────────────────────────────────────────────

void
StatisticsBridge::prevMonth()
{
  calendar_month_--;
  if (calendar_month_ < 1)
    {
      calendar_month_ = 12;
      calendar_year_--;
    }
  Q_EMIT calendarChanged();
}

void
StatisticsBridge::nextMonth()
{
  calendar_month_++;
  if (calendar_month_ > 12)
    {
      calendar_month_ = 1;
      calendar_year_++;
    }
  Q_EMIT calendarChanged();
}

// ── updateStats ───────────────────────────────────────────────────────────────

void
StatisticsBridge::updateStats()
{
  IStatistics::DailyStats *stats = (current_idx_ >= 0) ? statistics_->get_day(current_idx_) : nullptr;

  if (stats == nullptr || stats->start.tm_year == 0)
    {
      clearStats();
      return;
    }

  // ── Date text ──────────────────────────────────────────────────────────────
  {
    QLocale locale;
    QDate start_date(stats->start.tm_year + 1900, stats->start.tm_mon + 1, stats->start.tm_mday);
    QTime start_time(stats->start.tm_hour, stats->start.tm_min, stats->start.tm_sec);
    QTime stop_time(stats->stop.tm_hour, stats->stop.tm_min, stats->stop.tm_sec);

    QString date_str  = locale.toString(start_date, QLocale::ShortFormat);
    QString start_str = locale.toString(start_time, QLocale::ShortFormat);
    QString stop_str  = locale.toString(stop_time, QLocale::ShortFormat);

    selected_date_text_ = QObject::tr("%1, from %2 to %3")
                            .arg(date_str)
                            .arg(start_str)
                            .arg(stop_str);
  }

  // ── Break stats (7 rows × 3 break types) ──────────────────────────────────
  struct BreakRowDef
  {
    const char *label;
    const char *tooltip;
  };

  static const std::array<BreakRowDef, 7> break_row_defs = {{
    {.label = QT_TR_NOOP("Break prompts"),
     .tooltip = QT_TR_NOOP("The number of times you were prompted to break, excluding repeated prompts for the same break")},
    {.label = QT_TR_NOOP("Repeated prompts"),
     .tooltip = QT_TR_NOOP("The number of times you were repeatedly prompted to break")},
    {.label = QT_TR_NOOP("Prompted breaks taken"),
     .tooltip = QT_TR_NOOP("The number of times you took a break when being prompted")},
    {.label = QT_TR_NOOP("Natural breaks taken"),
     .tooltip = QT_TR_NOOP("The number of times you took a break without being prompted")},
    {.label = QT_TR_NOOP("Breaks skipped"),
     .tooltip = QT_TR_NOOP("The number of breaks you skipped")},
    {.label = QT_TR_NOOP("Breaks postponed"),
     .tooltip = QT_TR_NOOP("The number of breaks you postponed")},
    {.label = QT_TR_NOOP("Overdue time"),
     .tooltip = QT_TR_NOOP("The total time this break was overdue")},
  }};

  QVariantList rows;
  for (int row = 0; row < 7; row++)
    {
      QVariantMap r;
      r[QStringLiteral("label")]   = QObject::tr(break_row_defs.at(row).label);
      r[QStringLiteral("tooltip")] = QObject::tr(break_row_defs.at(row).tooltip);

      // micro=0, rest=1, daily=2
      auto cell_value = [&](int break_id) -> QString {
        const auto &bs = stats->break_stats[break_id];
        int64_t value  = 0;
        switch (row)
          {
          case 0: value = bs[IStatistics::STATS_BREAKVALUE_UNIQUE_BREAKS]; break;
          case 1: value = bs[IStatistics::STATS_BREAKVALUE_PROMPTED] - bs[IStatistics::STATS_BREAKVALUE_UNIQUE_BREAKS]; break;
          case 2: value = bs[IStatistics::STATS_BREAKVALUE_TAKEN]; break;
          case 3: value = bs[IStatistics::STATS_BREAKVALUE_NATURAL_TAKEN]; break;
          case 4: value = bs[IStatistics::STATS_BREAKVALUE_SKIPPED]; break;
          case 5: value = bs[IStatistics::STATS_BREAKVALUE_POSTPONED]; break;
          case 6: value = bs[IStatistics::STATS_BREAKVALUE_TOTAL_OVERDUE]; return formatTime(value);
          default: break;
          }
        return QString::number(value);
      };

      r[QStringLiteral("micro")] = cell_value(BREAK_ID_MICRO_BREAK);
      r[QStringLiteral("rest")]  = cell_value(BREAK_ID_REST_BREAK);
      r[QStringLiteral("daily")] = cell_value(BREAK_ID_DAILY_LIMIT);
      rows.append(r);
    }
  break_stats_ = rows;

  // ── Daily usage ────────────────────────────────────────────────────────────
  int64_t daily = stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_ACTIVE_TIME];
  daily_usage_  = daily > 0 ? formatTime(daily) : QString{};

  // ── Week / month usage ─────────────────────────────────────────────────────
  updateWeekUsage();
  updateMonthUsage();
}

void
StatisticsBridge::clearStats()
{
  selected_date_text_ = QString{};
  break_stats_.clear();
  daily_usage_.clear();
  weekly_usage_.clear();
  monthly_usage_.clear();
}

// ── updateWeekUsage ───────────────────────────────────────────────────────────

void
StatisticsBridge::updateWeekUsage()
{
  if (selected_year_ == 0)
    {
      weekly_usage_.clear();
      return;
    }

  std::tm timeinfo{};
  std::memset(&timeinfo, 0, sizeof(timeinfo));
  timeinfo.tm_mday = selected_day_;
  timeinfo.tm_mon  = selected_month_ - 1;
  timeinfo.tm_year = selected_year_ - 1900;

  std::time_t t        = std::mktime(&timeinfo);
  std::tm const *tloc  = std::localtime(&t);

  QLocale locale;
  int week_start = locale.firstDayOfWeek() % 7; // Qt: 1=Mon…7=Sun → 0-based Sun=0

  int offset     = (tloc->tm_wday - week_start + 7) % 7;
  int64_t total  = 0;

  for (int i = 0; i < 7; i++)
    {
      std::tm day_tm{};
      std::memset(&day_tm, 0, sizeof(day_tm));
      day_tm.tm_mday = selected_day_ - offset + i;
      day_tm.tm_mon  = selected_month_ - 1;
      day_tm.tm_year = selected_year_ - 1900;
      std::time_t dt   = std::mktime(&day_tm);
      std::tm const *dl = std::localtime(&dt);

      int idx  = 0;
      int next = 0;
      int prev = 0;
      statistics_->get_day_index_by_date(dl->tm_year + 1900, dl->tm_mon + 1, dl->tm_mday, idx, next, prev);
      if (idx >= 0)
        {
          IStatistics::DailyStats *stats = statistics_->get_day(idx);
          if (stats != nullptr)
            {
              total += stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_ACTIVE_TIME];
            }
        }
    }

  weekly_usage_ = (total > 0) ? formatTime(total) : QString{};
}

// ── updateMonthUsage ──────────────────────────────────────────────────────────

void
StatisticsBridge::updateMonthUsage()
{
  if (selected_year_ == 0)
    {
      monthly_usage_.clear();
      return;
    }

  int max_mday = QDate(selected_year_, selected_month_, 1).daysInMonth();
  int64_t total = 0;

  for (int d = 1; d <= max_mday; d++)
    {
      int idx  = 0;
      int next = 0;
      int prev = 0;
      statistics_->get_day_index_by_date(selected_year_, selected_month_, d, idx, next, prev);
      if (idx >= 0)
        {
          IStatistics::DailyStats *stats = statistics_->get_day(idx);
          if (stats != nullptr)
            {
              total += stats->misc_stats[IStatistics::STATS_VALUE_TOTAL_ACTIVE_TIME];
            }
        }
    }

  monthly_usage_ = (total > 0) ? formatTime(total) : QString{};
}

// ── tick ──────────────────────────────────────────────────────────────────────

void
StatisticsBridge::tick()
{
  if (current_idx_ == 0)
    {
      statistics_->update();
      updateStats();
      Q_EMIT dataChanged();
    }
}

// ── deleteAllHistory ──────────────────────────────────────────────────────────

void
StatisticsBridge::deleteAllHistory()
{
  bool success = statistics_->delete_all_history();
  if (success)
    {
      statistics_->update();
      current_idx_    = -1;
      current_next_   = -1;
      current_prev_   = -1;
      selected_year_  = 0;
      selected_month_ = 0;
      selected_day_   = 0;
      clearStats();

      // Navigate to last (most recent) entry, which may not exist after deletion
      int size = statistics_->get_history_size();
      if (size >= 0)
        {
          selectDayIndex(0);
        }
      else
        {
          QDate today      = QDate::currentDate();
          calendar_year_   = today.year();
          calendar_month_  = today.month();
          Q_EMIT calendarChanged();
          Q_EMIT dataChanged();
          Q_EMIT navChanged();
        }
    }
  Q_EMIT deleteCompleted(success);
}

// ── formatTime ────────────────────────────────────────────────────────────────

QString
StatisticsBridge::formatTime(int64_t secs)
{
  secs = std::max<int64_t>(secs, 0);
  int64_t h = secs / 3600;
  int64_t m = (secs % 3600) / 60;
  int64_t s = secs % 60;

  if (h > 0)
    {
      return QStringLiteral("%1:%2:%3")
        .arg(h)
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
    }
  return QStringLiteral("%1:%2")
    .arg(m)
    .arg(s, 2, 10, QChar('0'));
}

// ── QmlStatisticsDialog ───────────────────────────────────────────────────────

QmlStatisticsDialog::QmlStatisticsDialog(std::shared_ptr<IApplicationContext> app)
{
  TRACE_ENTRY();

  auto core       = app->get_core();
  auto statistics = core->get_statistics();

  bridge_ = new StatisticsBridge(statistics, app);

  view_ = new QQuickView;
  view_->setTitle(QObject::tr("Statistics"));
  view_->setResizeMode(QQuickView::SizeRootObjectToView);
  view_->setMinimumSize(QSize(860, 580));
  view_->resize(860, 580);

#ifdef Q_OS_MACOS
  {
    QDir bundleQml(QCoreApplication::applicationDirPath() + "/../Resources/qml");
    if (bundleQml.exists())
      {
        view_->engine()->addImportPath(bundleQml.canonicalPath());
      }
  }
#endif
#ifdef QT_QML_IMPORT_PATH
  view_->engine()->addImportPath(QStringLiteral(QT_QML_IMPORT_PATH));
#endif
  view_->engine()->addImportPath(QLibraryInfo::path(QLibraryInfo::QmlImportsPath));

  view_->rootContext()->setContextProperty("statsBridge", bridge_);

  QObject::connect(view_, &QQuickView::statusChanged, bridge_, [this](QQuickView::Status status) {
    if (status == QQuickView::Error)
      {
        for (const auto &err : view_->errors())
          {
            spdlog::error("StatisticsDialog QML error: {}", err.toString().toStdString());
          }
      }
    else if (status == QQuickView::Ready)
      {
        QQuickItem *root = view_->rootObject();
        if (root != nullptr)
          {
            QObject::connect(root, SIGNAL(closeRequested()), view_, SLOT(hide()));
          }
      }
  });

  view_->setSource(QUrl(QStringLiteral("qrc:/sanctuary/StatisticsDialog.qml")));

  timer_ = new QTimer(bridge_);
  timer_->setInterval(1000);
  QObject::connect(timer_, &QTimer::timeout, bridge_, [this]() { bridge_->tick(); });
  timer_->start();
}

QmlStatisticsDialog::~QmlStatisticsDialog()
{
  delete view_;
  delete bridge_;
}

void
QmlStatisticsDialog::show()
{
  view_->show();
}

void
QmlStatisticsDialog::raise()
{
  view_->raise();
}
