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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "QmlExercisesDialog.hh"

#include <algorithm>
#include <random>

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QUrl>
#include <QLibraryInfo>
#include <QDir>
#include <QCoreApplication>

#include "utils/AssetPath.hh"
#include "ui/SoundTheme.hh"
#include "debug.hh"

using namespace workrave::utils;

// ── ExercisesBridge ───────────────────────────────────────────────────────────

ExercisesBridge::ExercisesBridge(std::shared_ptr<IApplicationContext> app, QObject *parent)
  : QObject(parent)
  , app(std::move(app))
  , sound_theme(this->app->get_sound_theme())
{
  auto exercises_obj = this->app->get_exercises();
  auto list          = exercises_obj->get_exercises();
  for (auto &e : list)
    {
      shuffled_exercises.push_back(e);
    }

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(shuffled_exercises.begin(), shuffled_exercises.end(), g);

  if (!shuffled_exercises.empty())
    {
      ex_timer = new QTimer(this);
      ex_timer->setInterval(1000);
      connect(ex_timer, &QTimer::timeout, this, &ExercisesBridge::onTick);
      ex_timer->start();
      startExercise();
    }
}

QString
ExercisesBridge::exerciseName() const
{
  if (shuffled_exercises.empty())
    {
      return {};
    }
  return QString::fromStdString(shuffled_exercises[ex_index].title);
}

QString
ExercisesBridge::exerciseDescription() const
{
  if (shuffled_exercises.empty())
    {
      return {};
    }
  return QString::fromStdString(shuffled_exercises[ex_index].description);
}

QString
ExercisesBridge::exerciseImage() const
{
  if (shuffled_exercises.empty())
    {
      return {};
    }
  const auto &seq = shuffled_exercises[ex_index].sequence;
  if (seq.empty() || ex_image_it == seq.end())
    {
      return {};
    }
  std::string path = AssetPath::complete_directory(ex_image_it->image, SearchPathId::Exercises);
  if (path.empty())
    {
      return {};
    }
  return QUrl::fromLocalFile(QString::fromStdString(path)).toString();
}

bool
ExercisesBridge::exerciseImageMirror() const
{
  if (shuffled_exercises.empty())
    {
      return false;
    }
  const auto &seq = shuffled_exercises[ex_index].sequence;
  if (seq.empty() || ex_image_it == seq.end())
    {
      return false;
    }
  return ex_image_it->mirror_x;
}

double
ExercisesBridge::exerciseProgress() const
{
  if (shuffled_exercises.empty())
    {
      return 1.0;
    }
  const Exercise &ex = shuffled_exercises[ex_index];
  if (ex.duration <= 0)
    {
      return 1.0;
    }
  double remaining = static_cast<double>(ex.duration - ex_time) / ex.duration;
  return qBound(0.0, remaining, 1.0);
}

QString
ExercisesBridge::exerciseTimeStr() const
{
  if (shuffled_exercises.empty())
    {
      return QStringLiteral("0:00");
    }
  const Exercise &ex = shuffled_exercises[ex_index];
  int t              = std::max(0, ex.duration - ex_time);
  return QString("%1:%2").arg(t / 60).arg(t % 60, 2, 10, QChar('0'));
}

void
ExercisesBridge::startExercise()
{
  ex_time     = 0;
  ex_seq_time = 0;
  const auto &seq = shuffled_exercises[ex_index].sequence;
  ex_image_it     = seq.end();
  advanceImage();
  Q_EMIT exerciseChanged();
  Q_EMIT exerciseTimerChanged();
}

void
ExercisesBridge::advanceImage()
{
  const auto &seq = shuffled_exercises[ex_index].sequence;
  if (seq.empty())
    {
      return;
    }
  if (ex_image_it == seq.end())
    {
      ex_image_it = seq.begin();
    }
  else
    {
      ++ex_image_it;
      if (ex_image_it == seq.end())
        {
          ex_image_it = seq.begin();
        }
    }
  ex_seq_time += ex_image_it->duration;
  Q_EMIT exerciseImageChanged();
}

void
ExercisesBridge::onTick()
{
  if (ex_paused || shuffled_exercises.empty())
    {
      return;
    }

  const Exercise &ex = shuffled_exercises[ex_index];
  ex_time++;

  if (ex_time >= ex_seq_time)
    {
      if (ex_time > 0)
        {
          sound_theme->play_sound(SoundEvent::ExerciseStep);
        }
      advanceImage();
    }

  Q_EMIT exerciseTimerChanged();

  if (ex_time >= ex.duration)
    {
      int n     = static_cast<int>(shuffled_exercises.size());
      ex_index  = (ex_index + 1) % n;
      sound_theme->play_sound(SoundEvent::ExerciseEnded);
      startExercise();
    }
}

void
ExercisesBridge::prevExercise()
{
  if (shuffled_exercises.empty())
    {
      return;
    }
  int n    = static_cast<int>(shuffled_exercises.size());
  ex_index = (ex_index + n - 1) % n;
  startExercise();
}

void
ExercisesBridge::nextExercise()
{
  if (shuffled_exercises.empty())
    {
      return;
    }
  int n    = static_cast<int>(shuffled_exercises.size());
  ex_index = (ex_index + 1) % n;
  startExercise();
}

void
ExercisesBridge::togglePause()
{
  ex_paused = !ex_paused;
  Q_EMIT pauseStateChanged();
}

void
ExercisesBridge::stopExercises()
{
  if (ex_timer != nullptr)
    {
      ex_timer->stop();
    }
  if (on_close_)
    {
      on_close_();
    }
}

// ── QmlExercisesDialog ────────────────────────────────────────────────────────

QmlExercisesDialog::QmlExercisesDialog(std::shared_ptr<IApplicationContext> app, QObject *parent)
  : QObject(parent)
  , app(std::move(app))
{
  TRACE_ENTRY();

  bridge = new ExercisesBridge(this->app, this);
  bridge->setCloseHandler([this]() { view->hide(); });

  view = new QQuickView();
  view->setTitle(tr("Exercises"));
  view->setResizeMode(QQuickView::SizeRootObjectToView);
  view->setMinimumSize(QSize(600, 340));
  view->resize(680, 420);

#ifdef Q_OS_MACOS
  {
    QDir bundleQml(QCoreApplication::applicationDirPath() + "/../Resources/qml");
    if (bundleQml.exists())
      view->engine()->addImportPath(bundleQml.canonicalPath());
  }
#endif
#ifdef QT_QML_IMPORT_PATH
  view->engine()->addImportPath(QStringLiteral(QT_QML_IMPORT_PATH));
#endif
  view->engine()->addImportPath(QLibraryInfo::path(QLibraryInfo::QmlImportsPath));

  view->rootContext()->setContextProperty("exercisesBridge", bridge);

  QObject::connect(view, &QQuickView::statusChanged, this, [this](QQuickView::Status status) {
    if (status == QQuickView::Error)
      {
        for (const auto &err : view->errors())
          {
            spdlog::error("ExercisesDialog QML error: {}", err.toString().toStdString());
          }
      }
    else if (status == QQuickView::Ready)
      {
        QQuickItem *root = view->rootObject();
        if (root != nullptr)
          {
            QObject::connect(root, SIGNAL(closeRequested()), this, SLOT(onCloseRequested()));
          }
      }
  });

  QObject::connect(view, &QWindow::visibilityChanged, this, &QmlExercisesDialog::onVisibilityChanged);

  view->setSource(QUrl("qrc:/sanctuary/ExercisesDialog.qml"));
}

QmlExercisesDialog::~QmlExercisesDialog()
{
  delete view;
}

void
QmlExercisesDialog::show()
{
  view->show();
  view->raise();
  view->requestActivate();
}

void
QmlExercisesDialog::onCloseRequested()
{
  view->hide();
}

void
QmlExercisesDialog::onVisibilityChanged(QWindow::Visibility visibility)
{
  if (visibility == QWindow::Hidden)
    {
      deleteLater();
    }
}
