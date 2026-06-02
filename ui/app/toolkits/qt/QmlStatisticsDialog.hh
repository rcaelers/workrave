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

#ifndef QML_STATISTICS_DIALOG_HH
#define QML_STATISTICS_DIALOG_HH

#include <cstdint>
#include <memory>

#include <QObject>
#include <QQuickView>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

#include "core/IStatistics.hh"
#include "ui/IApplicationContext.hh"

// ── StatisticsBridge ──────────────────────────────────────────────────────────
// QObject bridge exposing statistics data and navigation state to the QML layer.

class StatisticsBridge : public QObject
{
  Q_OBJECT

  // ── Calendar ──────────────────────────────────────────────────────────────
  Q_PROPERTY(int calendarYear READ calendarYear NOTIFY calendarChanged)
  Q_PROPERTY(int calendarMonth READ calendarMonth NOTIFY calendarChanged)
  Q_PROPERTY(QString calendarMonthYearText READ calendarMonthYearText NOTIFY calendarChanged)
  Q_PROPERTY(QVariantList calendarCells READ calendarCells NOTIFY calendarChanged)

  // ── Selected day ──────────────────────────────────────────────────────────
  Q_PROPERTY(QString selectedDateText READ selectedDateText NOTIFY dataChanged)

  // ── Navigation ────────────────────────────────────────────────────────────
  Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY navChanged)
  Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY navChanged)
  Q_PROPERTY(bool canGoFirst READ canGoFirst NOTIFY navChanged)
  Q_PROPERTY(bool canGoLast READ canGoLast NOTIFY navChanged)

  // ── Break statistics ──────────────────────────────────────────────────────
  Q_PROPERTY(QVariantList breakStats READ breakStats NOTIFY dataChanged)

  // ── Usage ─────────────────────────────────────────────────────────────────
  Q_PROPERTY(QString dailyUsage READ dailyUsage NOTIFY dataChanged)
  Q_PROPERTY(QString weeklyUsage READ weeklyUsage NOTIFY dataChanged)
  Q_PROPERTY(QString monthlyUsage READ monthlyUsage NOTIFY dataChanged)

public:
  explicit StatisticsBridge(workrave::IStatistics::Ptr statistics,
                             std::shared_ptr<IApplicationContext> app,
                             QObject *parent = nullptr);

  // Property accessors
  int calendarYear() const { return calendar_year_; }
  int calendarMonth() const { return calendar_month_; }
  QString calendarMonthYearText() const;
  QVariantList calendarCells() const;

  QString selectedDateText() const { return selected_date_text_; }

  bool canGoBack() const { return current_prev_ >= 0; }
  bool canGoForward() const { return current_next_ >= 0; }
  bool canGoFirst() const { return current_idx_ != statistics_->get_history_size(); }
  bool canGoLast() const { return current_idx_ != 0; }

  QVariantList breakStats() const { return break_stats_; }

  QString dailyUsage() const { return daily_usage_; }
  QString weeklyUsage() const { return weekly_usage_; }
  QString monthlyUsage() const { return monthly_usage_; }

  // Invokable actions
  Q_INVOKABLE void goBack();
  Q_INVOKABLE void goForward();
  Q_INVOKABLE void goFirst();
  Q_INVOKABLE void goLast();
  Q_INVOKABLE void selectDate(int y, int m, int d);
  Q_INVOKABLE void prevMonth();
  Q_INVOKABLE void nextMonth();
  Q_INVOKABLE void deleteAllHistory();
  Q_INVOKABLE void tick();

Q_SIGNALS:
  void calendarChanged();
  void dataChanged();
  void navChanged();
  void deleteCompleted(bool success);

private:
  void selectDayIndex(int idx);
  void updateStats();
  void updateWeekUsage();
  void updateMonthUsage();
  void updateDaysWithData();
  void clearStats();

  static QString formatTime(int64_t secs);

  workrave::IStatistics::Ptr statistics_;
  std::shared_ptr<IApplicationContext> app_;

  // Calendar state
  int calendar_year_{0};
  int calendar_month_{0};

  // Currently displayed day
  int current_idx_{-1};
  int current_next_{-1};
  int current_prev_{-1};
  int selected_year_{0};
  int selected_month_{0};
  int selected_day_{0};

  // Cached property values
  QString selected_date_text_;
  QVariantList break_stats_;
  QString daily_usage_;
  QString weekly_usage_;
  QString monthly_usage_;
};

// ── QmlStatisticsDialog ───────────────────────────────────────────────────────
// Owns a QQuickView and a StatisticsBridge; presents the statistics window.

class QmlStatisticsDialog
{
public:
  explicit QmlStatisticsDialog(std::shared_ptr<IApplicationContext> app);
  ~QmlStatisticsDialog();

  void show();
  void raise();

private:
  StatisticsBridge *bridge_{nullptr};
  QQuickView *view_{nullptr};
  QTimer *timer_{nullptr};
};

#endif // QML_STATISTICS_DIALOG_HH
