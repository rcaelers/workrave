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

#include <functional>
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
#include "utils/Signals.hh"

#if defined(HAVE_WAYLAND)
#  include "WaylandWindowManager.hh"
#endif

// Data bridge exposed to DailyLimitOverlay.qml as "bridge" context property.
class DailyLimitBridge
  : public QObject
  , public workrave::utils::Trackable
{
  Q_OBJECT

  Q_PROPERTY(int blockMode READ blockMode CONSTANT)
  Q_PROPERTY(bool lockable READ lockable CONSTANT)
  Q_PROPERTY(bool shutdownable READ shutdownable CONSTANT)
  Q_PROPERTY(bool sleepable READ sleepable CONSTANT)
  Q_PROPERTY(bool canPostpone READ canPostpone NOTIFY lockStateChanged)
  Q_PROPERTY(bool canSkip READ canSkip NOTIFY lockStateChanged)
  Q_PROPERTY(double lockProgress READ lockProgress NOTIFY lockStateChanged)
  Q_PROPERTY(bool isLocked READ isLocked NOTIFY lockStateChanged)
  Q_PROPERTY(bool userActive READ userActive NOTIFY userActivityChanged)
  Q_PROPERTY(bool classic READ isClassic NOTIFY classicChanged)

public:
  explicit DailyLimitBridge(std::shared_ptr<IApplicationContext> app,
                            BlockMode block_mode,
                            BreakFlags break_flags,
                            QObject *parent = nullptr);

  int blockMode() const;
  bool lockable() const;
  bool shutdownable() const;
  bool sleepable() const;
  bool canPostpone() const;
  bool canSkip() const;
  double lockProgress() const;
  bool isLocked() const;
  bool userActive() const
  {
    return user_active_;
  }
  bool isClassic() const
  {
    return classic_;
  }

  void setBreakButtonState(const BreakButtonState &state);
  void setDismissHandler(std::function<void()> fn)
  {
    on_dismiss_ = std::move(fn);
  }
  void updateUserActivity();

Q_SIGNALS:
  void lockStateChanged();
  void userActivityChanged();
  void classicChanged();

public Q_SLOTS:
  void requestPostpone();
  void requestSkip();
  void requestLock();
  void requestShutdown();
  void requestSleep();

private:
  std::shared_ptr<IApplicationContext> app;
  BlockMode block_mode;
  BreakFlags break_flags;

  std::function<void()> on_dismiss_;

  bool postpone_locked{false};
  bool skip_locked{false};
  double lock_progress_val{0.0};
  bool user_active_{false};
  bool classic_{false};
};

// IBreakWindow implementation that hosts a QQuickView with DailyLimitOverlay.qml.
class QmlDailyLimitWindow : public IBreakWindow
{
public:
  QmlDailyLimitWindow(std::shared_ptr<IApplicationContext> app, QScreen *screen, BreakFlags break_flags);
  ~QmlDailyLimitWindow() override;

  void init() override;
  void start() override;
  void stop() override;
  void refresh() override;
  void set_progress(int value, int max_value) override;
  void set_break_button_state(const BreakButtonState &state) override;

private:
  void configure_view_for_block_mode();
  void refresh_topmost_state();

  std::shared_ptr<IApplicationContext> app;
  QScreen *screen;
  BreakFlags break_flags;
  BlockMode block_mode;

  QQuickView *view{nullptr};
  DailyLimitBridge *bridge{nullptr};
  std::shared_ptr<bool> alive_{std::make_shared<bool>(true)};
  bool topmost_enabled_{true};
  QTimer *topmost_timer_{nullptr};

#if defined(HAVE_WAYLAND)
  std::shared_ptr<WaylandWindowManager> window_manager;
#endif
};

#endif // QMLDAILYLIMITWINDOW_HH
