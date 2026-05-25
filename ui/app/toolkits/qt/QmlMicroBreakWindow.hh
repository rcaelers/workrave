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

// Data bridge exposed to the QML scene as the "bridge" context property.
class MicroBreakBridge : public QObject
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

  void setProgress(int value, int max_value);
  void setBreakButtonState(const BreakButtonState &state);
  void updateRestBreakInfo();
  void updateUserActivity();

Q_SIGNALS:
  void progressChanged();
  void lockStateChanged();
  void restBreakInfoChanged();
  void userActivityChanged();

public Q_SLOTS:
  void requestPostpone();
  void requestSkip();
  void requestRestBreak();
  void requestLock();

private:
  std::shared_ptr<IApplicationContext> app;
  BlockMode block_mode;
  BreakFlags break_flags;

  int progress_value{0};
  int progress_max{1};
  bool postpone_locked{false};
  bool skip_locked{false};
  double lock_progress{0.0};
  bool user_active{false};
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
};

#endif // QMLMICROBREAKWINDOW_HH
