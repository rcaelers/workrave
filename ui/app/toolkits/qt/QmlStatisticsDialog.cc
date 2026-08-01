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

namespace
{
  QDate to_qdate(const workrave::stats::IStatistics::Date &date)
  {
    return {static_cast<int>(date.year()),
            static_cast<int>(static_cast<unsigned>(date.month())),
            static_cast<int>(static_cast<unsigned>(date.day()))};
  }

  workrave::stats::IStatistics::Date to_date(const QDate &date)
  {
    return std::chrono::year{date.year()} / date.month() / date.day();
  }

  QTime to_qtime(workrave::stats::LocalTime time)
  {
    const std::chrono::hh_mm_ss clock{workrave::stats::time_of_day(time)};
    return {static_cast<int>(clock.hours().count()),
            static_cast<int>(clock.minutes().count()),
            static_cast<int>(clock.seconds().count())};
  }
} // namespace

// ── StatisticsBridge ──────────────────────────────────────────────────────────

StatisticsBridge::StatisticsBridge(workrave::stats::IStatistics::Ptr statistics,
                                   std::shared_ptr<IApplicationContext> app,
                                   QObject *parent)
  : QObject(parent)
  , statistics_(std::move(statistics))
  , app_(std::move(app))
{
  statistics_->update();

  QDate today = QDate::currentDate();
  calendar_year_ = today.year();
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
  for (const Date &date: statistics_->get_dates(to_date(first), to_date(first.addDays(days_in_month - 1))))
    {
      days_with_data.insert(static_cast<int>(static_cast<unsigned>(date.day())));
    }

  QVariantList cells;
  int total = offset + days_in_month;
  int rows = (total + 6) / 7; // ceil to multiple of 7
  int count = rows * 7;

  for (int i = 0; i < count; i++)
    {
      QVariantMap cell;
      int day = (i < offset) ? 0 : (i - offset + 1);
      if (day > days_in_month)
        {
          day = 0;
        }
      cell[QStringLiteral("day")] = day;
      cell[QStringLiteral("hasData")] = (day > 0) && days_with_data.contains(day);
      cell[QStringLiteral("isSelected")] = (day > 0) && (calendar_year_ == selected_year_) && (calendar_month_ == selected_month_)
                                           && (day == selected_day_);
      cells.append(cell);
    }

  return cells;
}

// ── Navigation ────────────────────────────────────────────────────────────────

workrave::stats::IStatistics::Date
StatisticsBridge::selectedDate() const
{
  return to_date(QDate(selected_year_, selected_month_, selected_day_));
}

void
StatisticsBridge::goBack()
{
  std::optional<Date> date = statistics_->get_previous_date(selectedDate());
  if (date.has_value())
    {
      selectDate(date.value());
    }
}

void
StatisticsBridge::goForward()
{
  std::optional<Date> date = statistics_->get_next_date(selectedDate());
  if (date.has_value())
    {
      selectDate(date.value());
    }
}

void
StatisticsBridge::goFirst()
{
  std::optional<Date> date = statistics_->get_first_date();
  if (date.has_value())
    {
      selectDate(date.value());
    }
}

void
StatisticsBridge::goLast()
{
  std::optional<Date> date = statistics_->get_last_date();
  if (date.has_value())
    {
      selectDate(date.value());
    }
}

void
StatisticsBridge::selectDate(int y, int m, int d)
{
  selectDate(to_date(QDate(y, m, d)));
}

//! Shows the given date, whether or not it has statistics.
void
StatisticsBridge::selectDate(const Date &date)
{
  const QDate selected = to_qdate(date);

  selected_year_ = selected.year();
  selected_month_ = selected.month();
  selected_day_ = selected.day();

  // Flip the calendar page if needed
  calendar_year_ = selected.year();
  calendar_month_ = selected.month();

  updateStats();
  updateNavigation();

  Q_EMIT calendarChanged();
  Q_EMIT dataChanged();
  Q_EMIT navChanged();
}

//! Works out which of the navigation buttons have somewhere to go.
void
StatisticsBridge::updateNavigation()
{
  const Date date = selectedDate();

  std::optional<Date> first = statistics_->get_first_date();
  std::optional<Date> last = statistics_->get_last_date();

  can_go_back_ = statistics_->get_previous_date(date).has_value();
  can_go_forward_ = statistics_->get_next_date(date).has_value();
  can_go_first_ = first.has_value() && first.value() != date;
  can_go_last_ = last.has_value() && last.value() != date;
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
  std::optional<workrave::stats::IStatistics::DailyStats> day = (selected_year_ > 0) ? statistics_->get_day(selectedDate())
                                                                                     : std::nullopt;

  if (!day.has_value() || day->is_empty())
    {
      clearStats();
      return;
    }

  const workrave::stats::IStatistics::DailyStats *stats = &day.value();

  // ── Date text ──────────────────────────────────────────────────────────────
  {
    QLocale locale;
    QDate start_date = to_qdate(workrave::stats::date_of(*stats->start));
    QTime start_time = to_qtime(*stats->start);
    QTime stop_time = to_qtime(*stats->stop);

    QString date_str = locale.toString(start_date, QLocale::ShortFormat);
    QString start_str = locale.toString(start_time, QLocale::ShortFormat);
    QString stop_str = locale.toString(stop_time, QLocale::ShortFormat);

    selected_date_text_ = QObject::tr("%1, from %2 to %3").arg(date_str).arg(start_str).arg(stop_str);
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
    {.label = QT_TR_NOOP("Repeated prompts"), .tooltip = QT_TR_NOOP("The number of times you were repeatedly prompted to break")},
    {.label = QT_TR_NOOP("Prompted breaks taken"),
     .tooltip = QT_TR_NOOP("The number of times you took a break when being prompted")},
    {.label = QT_TR_NOOP("Natural breaks taken"),
     .tooltip = QT_TR_NOOP("The number of times you took a break without being prompted")},
    {.label = QT_TR_NOOP("Breaks skipped"), .tooltip = QT_TR_NOOP("The number of breaks you skipped")},
    {.label = QT_TR_NOOP("Breaks postponed"), .tooltip = QT_TR_NOOP("The number of breaks you postponed")},
    {.label = QT_TR_NOOP("Overdue time"), .tooltip = QT_TR_NOOP("The total time this break was overdue")},
  }};

  QVariantList rows;
  for (int row = 0; row < 7; row++)
    {
      QVariantMap r;
      r[QStringLiteral("label")] = QObject::tr(break_row_defs.at(row).label);
      r[QStringLiteral("tooltip")] = QObject::tr(break_row_defs.at(row).tooltip);

      // micro=0, rest=1, daily=2
      auto cell_value = [&](int break_id) -> QString {
        const auto &bs = stats->break_stats[break_id];
        int64_t value = 0;
        switch (row)
          {
          case 0:
            value = bs[workrave::stats::BreakStatValue::UniqueBreaks].get();
            break;
          case 1:
            value = bs[workrave::stats::BreakStatValue::Prompted].get() - bs[workrave::stats::BreakStatValue::UniqueBreaks].get();
            break;
          case 2:
            value = bs[workrave::stats::BreakStatValue::Taken].get();
            break;
          case 3:
            value = bs[workrave::stats::BreakStatValue::NaturalTaken].get();
            break;
          case 4:
            value = bs[workrave::stats::BreakStatValue::Skipped].get();
            break;
          case 5:
            value = bs[workrave::stats::BreakStatValue::Postponed].get();
            break;
          case 6:
            return formatTime(stats->total_overdue[break_id].get().count());
          default:
            break;
          }
        return QString::number(value);
      };

      r[QStringLiteral("micro")] = cell_value(BREAK_ID_MICRO_BREAK);
      r[QStringLiteral("rest")] = cell_value(BREAK_ID_REST_BREAK);
      r[QStringLiteral("daily")] = cell_value(BREAK_ID_DAILY_LIMIT);
      rows.append(r);
    }
  break_stats_ = rows;

  // ── Daily usage ────────────────────────────────────────────────────────────
  int64_t daily = stats->total_active_time.get().count();
  daily_usage_ = daily > 0 ? formatTime(daily) : QString{};

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

  QLocale locale;
  const QDate selected(selected_year_, selected_month_, selected_day_);

  // Qt: dayOfWeek() and firstDayOfWeek() are both 1=Mon … 7=Sun.
  const int offset = (selected.dayOfWeek() - locale.firstDayOfWeek() + 7) % 7;
  const QDate first = selected.addDays(-offset);
  const QDate last = first.addDays(6);

  const int64_t total = statistics_->get_total_active_time(to_date(first), to_date(last)).count();

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

  const int days_in_month = QDate(selected_year_, selected_month_, 1).daysInMonth();

  const QDate first(selected_year_, selected_month_, 1);
  const int64_t total = statistics_->get_total_active_time(to_date(first), to_date(first.addDays(days_in_month - 1))).count();

  monthly_usage_ = (total > 0) ? formatTime(total) : QString{};
}

// ── tick ──────────────────────────────────────────────────────────────────────

void
StatisticsBridge::tick()
{
  const QDate today = QDate::currentDate();
  if (selected_year_ == today.year() && selected_month_ == today.month() && selected_day_ == today.day())
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
      clearStats();

      // Deleting starts a new day, so there is normally something to show; fall
      // back to today if there is not.
      std::optional<Date> last = statistics_->get_last_date();
      const QDate today = QDate::currentDate();
      selectDate(last.value_or(to_date(today)));
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
      return QStringLiteral("%1:%2:%3").arg(h).arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0'));
    }
  return QStringLiteral("%1:%2").arg(m).arg(s, 2, 10, QChar('0'));
}

// ── QmlStatisticsDialog ───────────────────────────────────────────────────────

QmlStatisticsDialog::QmlStatisticsDialog(std::shared_ptr<IApplicationContext> app)
{
  TRACE_ENTRY();

  auto core = app->get_core();
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
        for (const auto &err: view_->errors())
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
