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

#include "QmlMicroBreakWindow.hh"

#include <QQmlContext>
#include <QScreen>
#include <QGuiApplication>
#include <QStyle>
#include <QTimer>

#include "core/ICore.hh"
#include "core/IBreak.hh"
#include "core/CoreTypes.hh"
#include "session/System.hh"
#include "debug.hh"
#include "UiUtil.hh"
#include "qformat.hh"
#include "Ui.hh"
#include "ui/GUIConfig.hh"

#if defined(HAVE_WAYLAND)
#  include "IToolkitUnixPrivate.hh"
#endif

using namespace workrave;

// ── MicroBreakBridge ──────────────────────────────────────────────────────────

MicroBreakBridge::MicroBreakBridge(std::shared_ptr<IApplicationContext> app,
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
MicroBreakBridge::blockMode() const
{
  return static_cast<int>(block_mode);
}

bool
MicroBreakBridge::lockable() const
{
  return System::is_lockable();
}

bool
MicroBreakBridge::restBreakEnabled() const
{
  auto core = app->get_core();
  return core->get_break(BREAK_ID_REST_BREAK)->is_enabled();
}

double
MicroBreakBridge::ringProgress() const
{
  if (progress_max <= 0)
    return 1.0;
  // Ring shows fraction of break time REMAINING (full at start, empty at end)
  double remaining = static_cast<double>(progress_max - progress_value) / progress_max;
  return qBound(0.0, remaining, 1.0);
}

QString
MicroBreakBridge::timeRemaining() const
{
  time_t t = static_cast<time_t>(progress_max - progress_value);
  return UiUtil::time_to_string(t);
}

QString
MicroBreakBridge::breakName() const
{
  return Ui::get_break_name(BREAK_ID_MICRO_BREAK);
}

bool
MicroBreakBridge::canPostpone() const
{
  return (break_flags & BREAK_FLAGS_POSTPONABLE) != 0 && !postpone_locked;
}

bool
MicroBreakBridge::canSkip() const
{
  return (break_flags & BREAK_FLAGS_SKIPPABLE) != 0 && !skip_locked;
}

double
MicroBreakBridge::lockProgress() const
{
  return lock_progress;
}

QString
MicroBreakBridge::restBreakInfo() const
{
  return rest_break_info;
}

void
MicroBreakBridge::setProgress(int value, int max_value)
{
  progress_value = value;
  progress_max = max_value;
  Q_EMIT progressChanged();
}

void
MicroBreakBridge::setBreakButtonState(const BreakButtonState &state)
{
  bool changed = (state.can_postpone == postpone_locked) || (state.can_skip == skip_locked)
                 || (state.lock_progress() != lock_progress);
  postpone_locked = !state.can_postpone;
  skip_locked = !state.can_skip;
  lock_progress = state.lock_progress();
  if (changed)
    {
      Q_EMIT lockStateChanged();
    }
}

bool
MicroBreakBridge::isLocked() const
{
  return postpone_locked || skip_locked;
}

void
MicroBreakBridge::updateUserActivity()
{
  bool active = app->get_core()->is_user_active();
  if (active != user_active)
    {
      user_active = active;
      Q_EMIT userActivityChanged();
    }
}

void
MicroBreakBridge::updateRestBreakInfo()
{
  auto core = app->get_core();
  auto restbreak = core->get_break(BREAK_ID_REST_BREAK);
  auto daily = core->get_break(BREAK_ID_DAILY_LIMIT);

  QString info;

  if (restbreak->is_enabled())
    {
      int64_t rb = restbreak->get_limit() - restbreak->get_elapsed_time();
      if (rb >= 0)
        {
          info = qstr(qformat(tr("Next rest break in %s")) % UiUtil::time_to_string(rb, true));
        }
      else
        {
          info = qstr(qformat(tr("Rest break %s overdue")) % UiUtil::time_to_string(-rb, true));
        }
    }
  else if (daily->is_enabled())
    {
      int64_t dl = daily->get_limit() - daily->get_elapsed_time();
      if (dl >= 0)
        {
          info = qstr(qformat(tr("Daily limit in %s")) % UiUtil::time_to_string(dl, true));
        }
    }

  if (info != rest_break_info)
    {
      rest_break_info = info;
      Q_EMIT restBreakInfoChanged();
    }
}

void
MicroBreakBridge::requestPostpone()
{
  QTimer::singleShot(0, this, [this]() {
    app->get_core()->get_break(BREAK_ID_MICRO_BREAK)->postpone_break();
    if (on_dismiss_)
      on_dismiss_();
  });
}

void
MicroBreakBridge::requestSkip()
{
  QTimer::singleShot(0, this, [this]() {
    app->get_core()->get_break(BREAK_ID_MICRO_BREAK)->skip_break();
    if (on_dismiss_)
      on_dismiss_();
  });
}

void
MicroBreakBridge::requestRestBreak()
{
  QTimer::singleShot(0, this, [this]() {
    app->get_core()->force_break(BREAK_ID_REST_BREAK, BreakHint::Normal);
  });
}

void
MicroBreakBridge::requestLock()
{
  if (System::is_lockable())
    {
      auto locker = app->get_toolkit()->get_locker();
      locker->unlock();
      locker->lock();
      System::lock_screen();
    }
}

// ── QmlMicroBreakWindow ───────────────────────────────────────────────────────

QmlMicroBreakWindow::QmlMicroBreakWindow(std::shared_ptr<IApplicationContext> app,
                                         QScreen *screen,
                                         BreakFlags break_flags)
  : app(app)
  , screen(screen)
  , break_flags(break_flags)
  , block_mode(GUIConfig::block_mode()())
{
#if defined(HAVE_WAYLAND)
  window_manager = std::dynamic_pointer_cast<IToolkitUnixPrivate>(app->get_toolkit())->get_wayland_window_manager();
#endif
}

QmlMicroBreakWindow::~QmlMicroBreakWindow()
{
  delete view;
}

void
QmlMicroBreakWindow::init()
{
  TRACE_ENTRY();

  bridge = new MicroBreakBridge(app, block_mode, break_flags);
  bridge->setDismissHandler([this]() { stop(); });

  view = new QQuickView();
  view->setResizeMode(QQuickView::SizeRootObjectToView);
  view->rootContext()->setContextProperty("bridge", bridge);

  // Log QML errors so missing assets or syntax problems are visible in the trace log
  QObject::connect(view, &QQuickView::statusChanged, view, [this](QQuickView::Status status) {
    if (status == QQuickView::Error)
      {
        for (const auto &err : view->errors())
          {
            spdlog::error("MicroBreakOverlay QML error: {}", err.toString().toStdString());
          }
      }
  });

  view->setSource(QUrl("qrc:/sanctuary/MicroBreakShell.qml"));

  configure_view_for_block_mode();
}

void
QmlMicroBreakWindow::configure_view_for_block_mode()
{
  // Qt::SplashScreen maps to a high NSWindowLevel on macOS, matching what the
  // classic BreakWindow uses, so the window appears above the input blocker.
  if (block_mode == BlockMode::All)
    {
      view->setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SplashScreen);
      view->setColor(QColor("#1B1D1A"));
    }
  else
    {
      // Input and Off both use a full-screen transparent overlay; QML positions
      // the card (centered for Input, top-right corner for Off).
      view->setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SplashScreen | Qt::WindowDoesNotAcceptFocus);
      view->setColor(Qt::transparent);
    }
}

void
QmlMicroBreakWindow::start()
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

  bridge->updateRestBreakInfo();

  if (block_mode == BlockMode::All)
    {
      view->showFullScreen();
    }
  else
    {
      // Input and Off: full-screen transparent overlay; QML positions the card
      // (centered for Input, top-right corner for Off).
      QRect geo = (screen != nullptr) ? screen->geometry() : QGuiApplication::primaryScreen()->geometry();
      view->setGeometry(geo);
      view->show();
    }

  view->raise();
}

void
QmlMicroBreakWindow::stop()
{
  TRACE_ENTRY();
  view->hide();

#if defined(HAVE_WAYLAND)
  if (window_manager)
    window_manager->clear_surfaces();
#endif
}

void
QmlMicroBreakWindow::refresh()
{
  TRACE_ENTRY();
  bridge->updateRestBreakInfo();
  bridge->updateUserActivity();
}

void
QmlMicroBreakWindow::set_progress(int value, int max_value)
{
  bridge->setProgress(value, max_value);
}

void
QmlMicroBreakWindow::set_break_button_state(const BreakButtonState &state)
{
  bridge->setBreakButtonState(state);
}
