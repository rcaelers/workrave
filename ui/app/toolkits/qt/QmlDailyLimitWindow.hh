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

#ifndef QMLDAILYLIMITWINDOW_HH
#define QMLDAILYLIMITWINDOW_HH

#include <memory>
#include <QObject>
#include <QScreen>
#include <QQuickView>
#include <QString>

#include "ui/IBreakWindow.hh"
#include "ui/GUIConfig.hh"
#include "ui/UiTypes.hh"
#include "ui/IApplicationContext.hh"
#include "core/CoreTypes.hh"

// Data bridge exposed to DailyLimitOverlay.qml as "bridge" context property.
class DailyLimitBridge : public QObject
{
  Q_OBJECT

  Q_PROPERTY(int  blockMode   READ blockMode   CONSTANT)
  Q_PROPERTY(bool lockable    READ lockable    CONSTANT)
  Q_PROPERTY(bool canPostpone  READ canPostpone  NOTIFY lockStateChanged)
  Q_PROPERTY(bool canSkip      READ canSkip      NOTIFY lockStateChanged)
  Q_PROPERTY(double lockProgress READ lockProgress NOTIFY lockStateChanged)
  Q_PROPERTY(bool isLocked     READ isLocked     NOTIFY lockStateChanged)

public:
  explicit DailyLimitBridge(std::shared_ptr<IApplicationContext> app,
                             BlockMode block_mode,
                             BreakFlags break_flags,
                             QObject *parent = nullptr);

  int  blockMode()   const;
  bool lockable()    const;
  bool canPostpone()   const;
  bool canSkip()       const;
  double lockProgress() const;
  bool isLocked()      const;

  void setBreakButtonState(const BreakButtonState &state);

Q_SIGNALS:
  void lockStateChanged();

public Q_SLOTS:
  void requestPostpone();
  void requestSkip();
  void requestLock();

private:
  std::shared_ptr<IApplicationContext> app;
  BlockMode block_mode;
  BreakFlags break_flags;

  bool postpone_locked{false};
  bool skip_locked{false};
  double lock_progress_val{0.0};
};

// IBreakWindow implementation that hosts a QQuickView with DailyLimitOverlay.qml.
class QmlDailyLimitWindow : public IBreakWindow
{
public:
  QmlDailyLimitWindow(std::shared_ptr<IApplicationContext> app,
                      QScreen *screen,
                      BreakFlags break_flags);
  ~QmlDailyLimitWindow() override;

  void init() override;
  void start() override;
  void stop() override;
  void refresh() override {}
  void set_progress(int value, int max_value) override;
  void set_break_button_state(const BreakButtonState &state) override;

private:
  void configure_view_for_block_mode();

  std::shared_ptr<IApplicationContext> app;
  QScreen *screen;
  BreakFlags break_flags;
  BlockMode block_mode;

  QQuickView *view{nullptr};
  DailyLimitBridge *bridge{nullptr};
};

#endif // QMLDAILYLIMITWINDOW_HH
