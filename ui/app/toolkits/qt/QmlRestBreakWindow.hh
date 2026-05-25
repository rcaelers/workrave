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

#ifndef QMLRESTBREAKWINDOW_HH
#define QMLRESTBREAKWINDOW_HH

#include <list>
#include <memory>
#include <vector>

#include <QObject>
#include <QScreen>
#include <QQuickView>
#include <QString>
#include <QTimer>

#include <QStringList>
#include "commonui/Exercise.hh"
#include "ui/IBreakWindow.hh"
#include "ui/GUIConfig.hh"
#include "ui/UiTypes.hh"
#include "ui/IApplicationContext.hh"
#include "core/CoreTypes.hh"

// Data bridge exposed to the QML scene as the "bridge" context property.
class RestBreakBridge : public QObject
{
  Q_OBJECT

  // CONSTANT properties
  Q_PROPERTY(int blockMode READ blockMode CONSTANT)
  Q_PROPERTY(bool lockable READ lockable CONSTANT)
  Q_PROPERTY(bool isNatural READ isNatural CONSTANT)
  Q_PROPERTY(bool hasExercises READ hasExercises CONSTANT)
  Q_PROPERTY(int exerciseCount READ exerciseCount CONSTANT)
  Q_PROPERTY(QStringList exerciseNames READ exerciseNames CONSTANT)

  // Notified via lockStateChanged
  Q_PROPERTY(bool canPostpone READ canPostpone NOTIFY lockStateChanged)
  Q_PROPERTY(bool canSkip READ canSkip NOTIFY lockStateChanged)

  // Notified via breakProgressChanged
  Q_PROPERTY(double breakProgress READ breakProgress NOTIFY breakProgressChanged)
  Q_PROPERTY(QString breakTime READ breakTime NOTIFY breakProgressChanged)
  Q_PROPERTY(QString breakTimeShort READ breakTimeShort NOTIFY breakProgressChanged)
  Q_PROPERTY(QString breakMaxStr READ breakMaxStr NOTIFY breakProgressChanged)

  // Notified via exerciseChanged
  Q_PROPERTY(int exerciseIndex READ exerciseIndex NOTIFY exerciseChanged)
  Q_PROPERTY(QString exerciseName READ exerciseName NOTIFY exerciseChanged)
  Q_PROPERTY(QString exerciseDescription READ exerciseDescription NOTIFY exerciseChanged)
  Q_PROPERTY(bool exercisesDone READ exercisesDone NOTIFY exerciseChanged)

  // Notified via exerciseImageChanged
  Q_PROPERTY(QString exerciseImage READ exerciseImage NOTIFY exerciseImageChanged)
  Q_PROPERTY(bool exerciseImageMirror READ exerciseImageMirror NOTIFY exerciseImageChanged)

  // Notified via exerciseTimerChanged
  Q_PROPERTY(double exerciseProgress READ exerciseProgress NOTIFY exerciseTimerChanged)
  Q_PROPERTY(QString exerciseTimeStr READ exerciseTimeStr NOTIFY exerciseTimerChanged)

  // Notified via pauseStateChanged
  Q_PROPERTY(bool isPaused READ isPaused NOTIFY pauseStateChanged)

public:
  explicit RestBreakBridge(std::shared_ptr<IApplicationContext> app,
                            BlockMode block_mode,
                            BreakFlags break_flags,
                            QObject *parent = nullptr);

  // CONSTANT accessors
  int blockMode() const;
  bool lockable() const;
  bool isNatural() const;
  bool hasExercises() const;
  int exerciseCount() const;
  QStringList exerciseNames() const;

  // Lock state
  bool canPostpone() const;
  bool canSkip() const;

  // Break total time
  QString breakTimeShort() const;
  QString breakMaxStr() const;

  // Break progress
  double breakProgress() const;
  QString breakTime() const;

  // Exercise state
  int exerciseIndex() const;
  QString exerciseName() const;
  QString exerciseDescription() const;
  bool exercisesDone() const;

  // Exercise image
  QString exerciseImage() const;
  bool exerciseImageMirror() const;

  // Exercise timer
  double exerciseProgress() const;
  QString exerciseTimeStr() const;

  // Pause
  bool isPaused() const;

  // Called from QmlRestBreakWindow
  void setProgress(int value, int max_value);
  void updateLockState();
  void initExercises();

Q_SIGNALS:
  void lockStateChanged();
  void breakProgressChanged();
  void exerciseChanged();
  void exerciseImageChanged();
  void exerciseTimerChanged();
  void pauseStateChanged();

public Q_SLOTS:
  void requestPostpone();
  void requestSkip();
  void requestLock();
  void nextExercise();
  void prevExercise();
  void togglePause();

private Q_SLOTS:
  void onExerciseTick();

private:
  void startExercise();
  void advanceImage();

  std::shared_ptr<IApplicationContext> app;
  BlockMode block_mode;
  BreakFlags break_flags;

  int break_value{0};
  int break_max{1};
  bool postpone_locked{false};
  bool skip_locked{false};

  std::vector<Exercise> shuffled_exercises;
  int ex_index{0};
  int ex_count{0};
  int ex_time{0};
  int ex_seq_time{0};
  bool ex_done{false};
  bool ex_paused{false};
  std::list<Exercise::Image>::const_iterator ex_image_it;
  QTimer *ex_timer{nullptr};
};

// IBreakWindow implementation that hosts a QQuickView with RestBreakOverlay.qml.
class QmlRestBreakWindow : public IBreakWindow
{
public:
  QmlRestBreakWindow(std::shared_ptr<IApplicationContext> app,
                     QScreen *screen,
                     BreakFlags break_flags);
  ~QmlRestBreakWindow() override;

  void init() override;
  void start() override;
  void stop() override;
  void refresh() override {}
  void set_progress(int value, int max_value) override;

private:
  void configure_view_for_block_mode();

  std::shared_ptr<IApplicationContext> app;
  QScreen *screen;
  BreakFlags break_flags;
  BlockMode block_mode;

  QQuickView *view{nullptr};
  RestBreakBridge *bridge{nullptr};
};

#endif // QMLRESTBREAKWINDOW_HH
