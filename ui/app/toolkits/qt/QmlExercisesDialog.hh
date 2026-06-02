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

#ifndef QMLEXERCISESDIALOG_HH
#define QMLEXERCISESDIALOG_HH

#include <functional>
#include <list>
#include <memory>
#include <vector>

#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>
#include <QQuickView>

#include "commonui/Exercise.hh"
#include "ui/IApplicationContext.hh"
#include "utils/Signals.hh"

// Data bridge exposed to ExercisesDialog.qml as "exercisesBridge" context property.
class ExercisesBridge
  : public QObject
  , public workrave::utils::Trackable
{
  Q_OBJECT

  Q_PROPERTY(QString exerciseName        READ exerciseName        NOTIFY exerciseChanged)
  Q_PROPERTY(QString exerciseDescription READ exerciseDescription NOTIFY exerciseChanged)
  Q_PROPERTY(QString exerciseImage       READ exerciseImage       NOTIFY exerciseImageChanged)
  Q_PROPERTY(bool    exerciseImageMirror READ exerciseImageMirror NOTIFY exerciseImageChanged)
  Q_PROPERTY(double  exerciseProgress    READ exerciseProgress    NOTIFY exerciseTimerChanged)
  Q_PROPERTY(QString exerciseTimeStr     READ exerciseTimeStr     NOTIFY exerciseTimerChanged)
  Q_PROPERTY(bool    isPaused            READ isPaused            NOTIFY pauseStateChanged)

public:
  explicit ExercisesBridge(std::shared_ptr<IApplicationContext> app, QObject *parent = nullptr);

  QString exerciseName() const;
  QString exerciseDescription() const;
  QString exerciseImage() const;
  bool    exerciseImageMirror() const;
  double  exerciseProgress() const;
  QString exerciseTimeStr() const;
  bool    isPaused() const { return ex_paused; }

  void setCloseHandler(std::function<void()> fn) { on_close_ = std::move(fn); }

Q_SIGNALS:
  void exerciseChanged();
  void exerciseImageChanged();
  void exerciseTimerChanged();
  void pauseStateChanged();

public Q_SLOTS:
  void prevExercise();
  void nextExercise();
  void togglePause();
  void stopExercises();

private Q_SLOTS:
  void onTick();

private:
  void startExercise();
  void advanceImage();

  std::shared_ptr<IApplicationContext> app;
  SoundTheme::Ptr sound_theme;

  std::function<void()> on_close_;

  std::vector<Exercise> shuffled_exercises;
  std::list<Exercise::Image>::const_iterator ex_image_it;
  int  ex_index{0};
  int  ex_time{0};
  int  ex_seq_time{0};
  bool ex_paused{false};

  QTimer *ex_timer{nullptr};
};

// Standalone exercises player hosted in a QQuickView.
class QmlExercisesDialog : public QObject
{
  Q_OBJECT

public:
  explicit QmlExercisesDialog(std::shared_ptr<IApplicationContext> app, QObject *parent = nullptr);
  ~QmlExercisesDialog() override;

  void show();

private Q_SLOTS:
  void onCloseRequested();

private:
  std::shared_ptr<IApplicationContext> app;
  QQuickView *view{nullptr};
  ExercisesBridge *bridge{nullptr};
};

#endif // QMLEXERCISESDIALOG_HH
