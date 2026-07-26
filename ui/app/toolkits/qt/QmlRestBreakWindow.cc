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

#include "QmlRestBreakWindow.hh"

#include <random>

#include <QQmlContext>
#include <QScreen>
#include <QStringList>
#include <QGuiApplication>
#include <QTimer>
#include <QUrl>

#include "core/ICore.hh"
#include "core/IBreak.hh"
#include "core/CoreTypes.hh"
#include "session/System.hh"
#include "utils/AssetPath.hh"
#include "debug.hh"
#include "UiUtil.hh"
#include <fmt/format.h>

#if defined(HAVE_WAYLAND)
#  include "IToolkitUnixPrivate.hh"
#endif

#if defined(PLATFORM_OS_WINDOWS)
#  include "TaskManagerWatcher.hh"
#  include "ui/windows/WindowsCompat.hh"
#endif

using namespace workrave;
using namespace workrave::utils;

// ── RestBreakBridge ───────────────────────────────────────────────────────────

RestBreakBridge::RestBreakBridge(std::shared_ptr<IApplicationContext> app,
                                 BlockMode block_mode,
                                 BreakFlags break_flags,
                                 QObject *parent)
  : QObject(parent)
  , app(std::move(app))
  , block_mode(block_mode)
  , break_flags(break_flags)
  , classic_(!GUIConfig::sanctuary_ui_enabled()())
{
  GUIConfig::sanctuary_ui_enabled().connect(this, [this](bool enabled) {
    bool new_classic = !enabled;
    if (new_classic != classic_)
      {
        classic_ = new_classic;
        Q_EMIT classicChanged();
      }
  });
}

int
RestBreakBridge::blockMode() const
{
  return static_cast<int>(block_mode);
}

bool
RestBreakBridge::lockable() const
{
  return System::is_lockable();
}

bool
RestBreakBridge::shutdownable() const
{
  return GUIConfig::break_enable_shutdown(BREAK_ID_REST_BREAK)() && System::is_shutdownable();
}

bool
RestBreakBridge::sleepable() const
{
  return GUIConfig::break_enable_shutdown(BREAK_ID_REST_BREAK)() && System::is_sleepable();
}

bool
RestBreakBridge::isNatural() const
{
  return (break_flags & BREAK_FLAGS_NATURAL) != 0;
}

bool
RestBreakBridge::hasExercises() const
{
  return !shuffled_exercises.empty() && ex_count > 0;
}

int
RestBreakBridge::exerciseCount() const
{
  return ex_count;
}

bool
RestBreakBridge::canPostpone() const
{
  return (break_flags & BREAK_FLAGS_POSTPONABLE) != 0 && !postpone_locked;
}

bool
RestBreakBridge::canSkip() const
{
  return (break_flags & BREAK_FLAGS_SKIPPABLE) != 0 && !skip_locked;
}

double
RestBreakBridge::lockProgress() const
{
  return lock_progress_val;
}

bool
RestBreakBridge::isLocked() const
{
  return postpone_locked || skip_locked;
}

double
RestBreakBridge::breakProgress() const
{
  if (break_max <= 0)
    {
      return 1.0;
    }
  double remaining = static_cast<double>(break_max - break_value) / break_max;
  return qBound(0.0, remaining, 1.0);
}

QString
RestBreakBridge::breakTime() const
{
  time_t t = static_cast<time_t>(std::max(0, break_max - break_value));
  return QString::fromStdString(
    fmt::format(fmt::runtime(tr("Rest break for {}").toStdString()), UiUtil::time_to_string(t).toStdString()));
}

QString
RestBreakBridge::breakTimeShort() const
{
  time_t t = static_cast<time_t>(std::max(0, break_max - break_value));
  return UiUtil::time_to_string(t);
}

QString
RestBreakBridge::breakMaxStr() const
{
  return UiUtil::time_to_string(static_cast<time_t>(break_max));
}

QStringList
RestBreakBridge::exerciseNames() const
{
  QStringList list;
  list.reserve(ex_count);
  for (int i = 0; i < ex_count; i++)
    {
      list << QString::fromStdString(shuffled_exercises[i].title);
    }
  return list;
}

int
RestBreakBridge::exerciseIndex() const
{
  return ex_index;
}

QString
RestBreakBridge::exerciseName() const
{
  if (shuffled_exercises.empty() || ex_done || ex_index >= ex_count)
    {
      return {};
    }
  return QString::fromStdString(shuffled_exercises[ex_index].title);
}

QString
RestBreakBridge::exerciseDescription() const
{
  if (shuffled_exercises.empty() || ex_done || ex_index >= ex_count)
    {
      return {};
    }
  return QString::fromStdString(shuffled_exercises[ex_index].description);
}

bool
RestBreakBridge::exercisesDone() const
{
  return ex_done;
}

QString
RestBreakBridge::exerciseImage() const
{
  if (shuffled_exercises.empty() || ex_done || ex_index >= ex_count)
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
RestBreakBridge::exerciseImageMirror() const
{
  if (shuffled_exercises.empty() || ex_done || ex_index >= ex_count)
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
RestBreakBridge::exerciseProgress() const
{
  if (shuffled_exercises.empty() || ex_done || ex_index >= ex_count)
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
RestBreakBridge::exerciseTimeStr() const
{
  if (shuffled_exercises.empty() || ex_done || ex_index >= ex_count)
    {
      return {};
    }
  const Exercise &ex = shuffled_exercises[ex_index];
  int t = std::max(0, ex.duration - ex_time);
  return UiUtil::time_to_string(static_cast<time_t>(t));
}

bool
RestBreakBridge::isPaused() const
{
  return ex_paused;
}

void
RestBreakBridge::setProgress(int value, int max_value)
{
  break_value = value;
  break_max = max_value;
  Q_EMIT breakProgressChanged();
}

void
RestBreakBridge::setBreakButtonState(const BreakButtonState &state)
{
  bool changed = (state.can_postpone == postpone_locked) || (state.can_skip == skip_locked)
                 || (state.lock_progress() != lock_progress_val);
  postpone_locked = !state.can_postpone;
  skip_locked = !state.can_skip;
  lock_progress_val = state.lock_progress();
  if (changed)
    {
      Q_EMIT lockStateChanged();
    }
}

void
RestBreakBridge::initExercises()
{
  bool can_show = (break_flags & BREAK_FLAGS_NO_EXERCISES) == 0;
  auto exercises_obj = app->get_exercises();
  can_show = can_show && exercises_obj->has_exercises();
  int cfg_count = can_show ? GUIConfig::break_exercises(BREAK_ID_REST_BREAK)() : 0;
  can_show = can_show && (cfg_count > 0);

  if (!can_show)
    {
      app->get_core()->set_insist_policy(InsistPolicy::Halt);
      return;
    }

  auto list = exercises_obj->get_exercises();
  for (auto &e: list)
    {
      shuffled_exercises.push_back(e);
    }

  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(shuffled_exercises.begin(), shuffled_exercises.end(), g);

  ex_count = std::min(cfg_count, static_cast<int>(shuffled_exercises.size()));
  ex_index = 0;

  app->get_core()->set_insist_policy(InsistPolicy::Ignore);

  ex_timer = new QTimer(this);
  ex_timer->setInterval(1000);
  connect(ex_timer, &QTimer::timeout, this, &RestBreakBridge::onExerciseTick);
  ex_timer->start();
  startExercise();
}

void
RestBreakBridge::startExercise()
{
  ex_time = 0;
  ex_seq_time = 0;
  const auto &seq = shuffled_exercises[ex_index].sequence;
  ex_image_it = seq.end();
  advanceImage();
  Q_EMIT exerciseChanged();
  Q_EMIT exerciseTimerChanged();
}

void
RestBreakBridge::advanceImage()
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
RestBreakBridge::onExerciseTick()
{
  if (ex_paused || ex_done || shuffled_exercises.empty())
    {
      return;
    }
  const Exercise &ex = shuffled_exercises[ex_index];
  ex_time++;
  if (ex_time >= ex_seq_time)
    {
      advanceImage();
    }
  Q_EMIT exerciseTimerChanged();
  if (ex_time >= ex.duration)
    {
      ex_index++;
      if (ex_index >= ex_count)
        {
          ex_done = true;
          ex_timer->stop();
          app->get_core()->set_insist_policy(InsistPolicy::Halt);
          Q_EMIT exerciseChanged();
        }
      else
        {
          startExercise();
        }
    }
}

void
RestBreakBridge::updateUserActivity()
{
  bool active = app->get_core()->is_user_active();
  if (active != user_active_)
    {
      user_active_ = active;
      Q_EMIT userActivityChanged();
    }
}

void
RestBreakBridge::requestPostpone()
{
  QTimer::singleShot(0, this, [this]() {
    app->get_core()->get_break(BREAK_ID_REST_BREAK)->postpone_break();
    if (on_dismiss_)
      on_dismiss_();
  });
}

void
RestBreakBridge::requestSkip()
{
  QTimer::singleShot(0, this, [this]() {
    app->get_core()->get_break(BREAK_ID_REST_BREAK)->skip_break();
    if (on_dismiss_)
      on_dismiss_();
  });
}

void
RestBreakBridge::requestLock()
{
  if (System::is_lockable())
    {
      auto locker = app->get_toolkit()->get_locker();
      locker->unlock();
      locker->lock();
      System::lock_screen_by_id(GUIConfig::preferred_lock_method()());
    }
}

void
RestBreakBridge::requestShutdown()
{
  System::execute(System::SystemOperation::SYSTEM_OPERATION_SHUTDOWN);
}

void
RestBreakBridge::requestSleep()
{
  const std::string &pref = GUIConfig::preferred_sleep_operation()();
  System::SystemOperation::SystemOperationType type = System::SystemOperation::SYSTEM_OPERATION_SUSPEND;
  if (pref == "hibernate")
    {
      type = System::SystemOperation::SYSTEM_OPERATION_HIBERNATE;
    }
  else if (pref == "suspend_hybrid")
    {
      type = System::SystemOperation::SYSTEM_OPERATION_SUSPEND_HYBRID;
    }
  if (!System::execute(type))
    {
      for (const auto &op: System::get_sleep_operations())
        {
          if (System::execute(op.type))
            break;
        }
    }
}

void
RestBreakBridge::nextExercise()
{
  if (shuffled_exercises.empty())
    {
      return;
    }
  ex_index++;
  if (ex_index >= ex_count)
    {
      ex_done = true;
      if (ex_timer != nullptr)
        {
          ex_timer->stop();
        }
      app->get_core()->set_insist_policy(InsistPolicy::Halt);
      Q_EMIT exerciseChanged();
    }
  else
    {
      startExercise();
    }
}

void
RestBreakBridge::prevExercise()
{
  if (shuffled_exercises.empty())
    {
      return;
    }
  if (ex_index > 0)
    {
      ex_index--;
    }
  ex_done = false;
  if (ex_timer != nullptr && !ex_timer->isActive())
    {
      ex_timer->start();
    }
  app->get_core()->set_insist_policy(InsistPolicy::Ignore);
  startExercise();
}

void
RestBreakBridge::togglePause()
{
  ex_paused = !ex_paused;
  Q_EMIT pauseStateChanged();
}

void
RestBreakBridge::endExercises()
{
  ex_done = true;
  if (ex_timer != nullptr)
    {
      ex_timer->stop();
    }
  app->get_core()->set_insist_policy(InsistPolicy::Halt);
  Q_EMIT exerciseChanged();
}

// ── QmlRestBreakWindow ────────────────────────────────────────────────────────

QmlRestBreakWindow::QmlRestBreakWindow(std::shared_ptr<IApplicationContext> app, QScreen *screen, BreakFlags break_flags)
  : app(app)
  , screen(screen)
  , break_flags(break_flags)
  , block_mode(GUIConfig::block_mode()())
{
#if defined(HAVE_WAYLAND)
  window_manager = std::dynamic_pointer_cast<IToolkitUnixPrivate>(app->get_toolkit())->get_wayland_window_manager();
#endif
}

QmlRestBreakWindow::~QmlRestBreakWindow()
{
  *alive_ = false;
  delete topmost_timer_;
  delete view;
}

void
QmlRestBreakWindow::init()
{
  TRACE_ENTRY();

  bridge = new RestBreakBridge(app, block_mode, break_flags);
  bridge->setDismissHandler([this, alive = alive_]() {
    if (*alive)
      stop();
  });
  bridge->initExercises();

  topmost_timer_ = new QTimer();
  topmost_timer_->setInterval(100);
  QObject::connect(topmost_timer_, &QTimer::timeout, [this]() { refresh_topmost_state(); });

  view = new QQuickView();
  view->setResizeMode(QQuickView::SizeRootObjectToView);
  view->rootContext()->setContextProperty("bridge", bridge);

  QObject::connect(view, &QQuickView::statusChanged, view, [this](QQuickView::Status status) {
    if (status == QQuickView::Error)
      {
        for (const auto &err: view->errors())
          {
            spdlog::error("RestBreakOverlay QML error: {}", err.toString().toStdString());
          }
      }
  });

  view->setSource(QUrl("qrc:/sanctuary/RestBreakShell.qml"));

  configure_view_for_block_mode();
}

void
QmlRestBreakWindow::configure_view_for_block_mode()
{
  // Qt::SplashScreen maps to an elevated NSWindowLevel on macOS — stays visible
  // even when another application has focus.  Qt::Tool disappears when backgrounded.
  // Use the same flags for all block modes; QML handles the dim layer for mode 2.
  view->setFlags(Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
  view->setColor(Qt::transparent);
}

void
QmlRestBreakWindow::start()
{
  TRACE_ENTRY();

  if (screen != nullptr)
    {
      view->setScreen(screen);
    }

#if defined(HAVE_WAYLAND)
  if (window_manager)
    window_manager->init_surface(view, screen, true);
#endif

  if (block_mode == BlockMode::All)
    {
      view->showFullScreen();
    }
  else
    {
      // Input and Off: full-screen transparent view; QML draws a centered floating card.
      QRect geo = (screen != nullptr) ? screen->geometry() : QGuiApplication::primaryScreen()->geometry();
      view->setGeometry(geo);
      view->show();
    }

  view->raise();
  refresh_topmost_state();
  if (topmost_timer_ != nullptr)
    {
      topmost_timer_->start();
    }
}

void
QmlRestBreakWindow::stop()
{
  TRACE_ENTRY();
  if (topmost_timer_ != nullptr)
    {
      topmost_timer_->stop();
    }
  view->hide();

#if defined(HAVE_WAYLAND)
  if (window_manager)
    window_manager->clear_surfaces();
#endif
}

void
QmlRestBreakWindow::refresh()
{
  refresh_topmost_state();
  bridge->updateUserActivity();
}

void
QmlRestBreakWindow::refresh_topmost_state()
{
#if defined(PLATFORM_OS_WINDOWS)
  if (view == nullptr)
    {
      return;
    }

  bool should_be_topmost = !TaskManagerWatcher::is_running();
  if (topmost_enabled_ != should_be_topmost)
    {
      WindowsCompat::SetWindowOnTop(reinterpret_cast<HWND>(view->winId()), should_be_topmost ? TRUE : FALSE);
      topmost_enabled_ = should_be_topmost;
    }
#else
  (void)topmost_enabled_;
#endif
}

void
QmlRestBreakWindow::set_progress(int value, int max_value)
{
  bridge->setProgress(value, max_value);
}

void
QmlRestBreakWindow::set_break_button_state(const BreakButtonState &state)
{
  bridge->setBreakButtonState(state);
}
