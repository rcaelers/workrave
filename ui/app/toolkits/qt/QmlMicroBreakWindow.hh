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

#ifndef QMLMICROBREAKWINDOW_HH
#define QMLMICROBREAKWINDOW_HH

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

// Data bridge exposed to the QML scene as the "bridge" context property.
class MicroBreakBridge
  : public QObject
  , public workrave::utils::Trackable
{
  Q_OBJECT

  Q_PROPERTY(int blockMode READ blockMode CONSTANT)
  Q_PROPERTY(bool lockable READ lockable CONSTANT)
  Q_PROPERTY(bool restBreakEnabled READ restBreakEnabled CONSTANT)

  Q_PROPERTY(double ringProgress READ ringProgress NOTIFY progressChanged)
  Q_PROPERTY(QString timeRemaining READ timeRemaining NOTIFY progressChanged)
  Q_PROPERTY(QString breakName READ breakName CONSTANT)

  Q_PROPERTY(bool canPostpone READ canPostpone NOTIFY lockStateChanged)
  Q_PROPERTY(bool canSkip READ canSkip NOTIFY lockStateChanged)
  Q_PROPERTY(double lockProgress READ lockProgress NOTIFY lockStateChanged)
  Q_PROPERTY(bool isLocked READ isLocked NOTIFY lockStateChanged)
  Q_PROPERTY(QString restBreakInfo READ restBreakInfo NOTIFY restBreakInfoChanged)
  Q_PROPERTY(bool userActive READ userActive NOTIFY userActivityChanged)
  Q_PROPERTY(bool classic READ isClassic NOTIFY classicChanged)

public:
  explicit MicroBreakBridge(std::shared_ptr<IApplicationContext> app,
                             BlockMode block_mode,
                             BreakFlags break_flags,
                             QObject *parent = nullptr);

  int blockMode() const;
  bool lockable() const;
  bool restBreakEnabled() const;

  double ringProgress() const;
  QString timeRemaining() const;
  QString breakName() const;

  bool canPostpone() const;
  bool canSkip() const;
  double lockProgress() const;
  bool isLocked() const;
  QString restBreakInfo() const;
  bool userActive() const { return user_active; }
  bool isClassic() const { return classic_; }

  void setProgress(int value, int max_value);
  void setBreakButtonState(const BreakButtonState &state);
  void setDismissHandler(std::function<void()> fn) { on_dismiss_ = std::move(fn); }
  void updateRestBreakInfo();
  void updateUserActivity();

Q_SIGNALS:
  void progressChanged();
  void lockStateChanged();
  void restBreakInfoChanged();
  void userActivityChanged();
  void classicChanged();

public Q_SLOTS:
  void requestPostpone();
  void requestSkip();
  void requestRestBreak();
  void requestLock();

private:
  std::shared_ptr<IApplicationContext> app;
  BlockMode block_mode;
  BreakFlags break_flags;

  std::function<void()> on_dismiss_;

  int progress_value{0};
  int progress_max{1};
  bool postpone_locked{false};
  bool skip_locked{false};
  double lock_progress{0.0};
  bool user_active{false};
  bool classic_{false};
  QString rest_break_info;
};

// IBreakWindow implementation that hosts a QQuickView with MicroBreakOverlay.qml.
class QmlMicroBreakWindow : public IBreakWindow
{
public:
  QmlMicroBreakWindow(std::shared_ptr<IApplicationContext> app,
                      QScreen *screen,
                      BreakFlags break_flags);
  ~QmlMicroBreakWindow() override;

  void init() override;
  void start() override;
  void stop() override;
  void refresh() override;
  void set_progress(int value, int max_value) override;
  void set_break_button_state(const BreakButtonState &state) override;

private:
  void configure_view_for_block_mode();

  std::shared_ptr<IApplicationContext> app;
  QScreen *screen;
  BreakFlags break_flags;
  BlockMode block_mode;

  QQuickView *view{nullptr};
  MicroBreakBridge *bridge{nullptr};

#if defined(HAVE_WAYLAND)
  std::shared_ptr<WaylandWindowManager> window_manager;
#endif
};

#endif // QMLMICROBREAKWINDOW_HH
