// Copyright (C) 2024 Rob Caelers <robc@krandor.nl>
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

#ifndef QMLTIMERBOXVIEW_HH
#define QMLTIMERBOXVIEW_HH

#include <array>
#include <memory>
#include <QObject>
#include <QWidget>
#include <QQuickItem>
#include <QQuickWidget>

#include "core/ICore.hh"
#include "ui/ITimerBoxView.hh"
#include "ui/UiTypes.hh"
#include "ui/GUIConfig.hh"
#include "utils/Signals.hh"

// Per-timer data snapshot kept in the bridge.
struct TimerData
{
  double progress{0.0};      // remaining fraction (1=just reset, 0=break due)
  double idle_progress{0.0}; // idle/rest fraction (0=just started resting, 1=break done)
  QString timeStr;
  bool visible{false};
  bool overdue{false};
};

// QObject bridge exposed as "bridge" context property to StatusWindow.qml.
class StatusWindowBridge
  : public QObject
  , public workrave::utils::Trackable
{
  Q_OBJECT

  Q_PROPERTY(int    displayStyle  READ displayStyle  NOTIFY displayStyleChanged)

  Q_PROPERTY(double microProgress     READ microProgress     NOTIFY dataChanged)
  Q_PROPERTY(double microIdleProgress READ microIdleProgress NOTIFY dataChanged)
  Q_PROPERTY(QString microTime        READ microTime         NOTIFY dataChanged)
  Q_PROPERTY(bool   microVisible      READ microVisible      NOTIFY dataChanged)
  Q_PROPERTY(bool   microOverdue      READ microOverdue      NOTIFY dataChanged)

  Q_PROPERTY(double restProgress      READ restProgress      NOTIFY dataChanged)
  Q_PROPERTY(double restIdleProgress  READ restIdleProgress  NOTIFY dataChanged)
  Q_PROPERTY(QString restTime         READ restTime          NOTIFY dataChanged)
  Q_PROPERTY(bool   restVisible       READ restVisible       NOTIFY dataChanged)
  Q_PROPERTY(bool   restOverdue       READ restOverdue       NOTIFY dataChanged)

  Q_PROPERTY(double dailyProgress     READ dailyProgress     NOTIFY dataChanged)
  Q_PROPERTY(double dailyIdleProgress READ dailyIdleProgress NOTIFY dataChanged)
  Q_PROPERTY(QString dailyTime        READ dailyTime         NOTIFY dataChanged)
  Q_PROPERTY(bool   dailyVisible      READ dailyVisible      NOTIFY dataChanged)
  Q_PROPERTY(bool   dailyOverdue      READ dailyOverdue      NOTIFY dataChanged)

public:
  explicit StatusWindowBridge(std::shared_ptr<workrave::ICore> core,
                              QWidget *owner_window,
                              QObject *parent = nullptr);

  // ITimerBoxView data setters — called from QmlTimerBoxView
  void setSlot(workrave::BreakId id, int slot);
  void setTimeBar(workrave::BreakId id,
                  int value,
                  TimerColorId primary_color,
                  int primary_val,
                  int primary_max,
                  int secondary_val,
                  int secondary_max);
  void commitUpdate();

  // Property accessors
  int     displayStyle()       const;
  double  microProgress()      const { return timers[0].progress; }
  double  microIdleProgress()  const { return timers[0].idle_progress; }
  QString microTime()          const { return timers[0].timeStr; }
  bool    microVisible()       const { return timers[0].visible; }
  bool    microOverdue()       const { return timers[0].overdue; }
  double  restProgress()       const { return timers[1].progress; }
  double  restIdleProgress()   const { return timers[1].idle_progress; }
  QString restTime()           const { return timers[1].timeStr; }
  bool    restVisible()        const { return timers[1].visible; }
  bool    restOverdue()        const { return timers[1].overdue; }
  double  dailyProgress()      const { return timers[2].progress; }
  double  dailyIdleProgress()  const { return timers[2].idle_progress; }
  QString dailyTime()          const { return timers[2].timeStr; }
  bool    dailyVisible()       const { return timers[2].visible; }
  bool    dailyOverdue()       const { return timers[2].overdue; }

Q_SIGNALS:
  void dataChanged();
  void displayStyleChanged();

public Q_SLOTS:
  Q_INVOKABLE void requestClose();
  Q_INVOKABLE void startWindowDrag();
  Q_INVOKABLE void continueWindowDrag();
  Q_INVOKABLE void stopWindowDrag();
  Q_INVOKABLE void forceRestBreak();

private:
  std::shared_ptr<workrave::ICore> core;
  QWidget *owner_window{nullptr};
  QPoint   last_cursor_pos;
  std::array<TimerData, workrave::BREAK_ID_SIZEOF> timers;
  // slot assignments: slot_to_id[slot] = break_id currently in that slot
  std::array<int, workrave::BREAK_ID_SIZEOF> slot_to_id;
};

// QWidget that hosts a QQuickWidget running StatusWindow.qml.
// Implements ITimerBoxView so it plugs into the existing TimerBoxControl.
class QmlTimerBoxView
  : public QWidget
  , public ITimerBoxView
  , public workrave::utils::Trackable
{
  Q_OBJECT

public:
  explicit QmlTimerBoxView(std::shared_ptr<workrave::ICore> core,
                           QWidget *owner_window,
                           QWidget *parent = nullptr);
  ~QmlTimerBoxView() override = default;

  // ITimerBoxView
  void set_slot(workrave::BreakId id, int slot) override;
  void set_time_bar(workrave::BreakId id,
                    int value,
                    TimerColorId primary_color,
                    int primary_value,
                    int primary_max,
                    TimerColorId secondary_color,
                    int secondary_value,
                    int secondary_max) override;
  void set_icon(OperationModeIcon icon) override {}
  void update_view() override;

private Q_SLOTS:
  void on_qml_size_changed();

private:
  std::shared_ptr<workrave::ICore> core;
  QQuickWidget *qml_widget{nullptr};
  StatusWindowBridge *bridge{nullptr};
};

#endif // QMLTIMERBOXVIEW_HH
