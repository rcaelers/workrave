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

#include "QmlDailyLimitWindow.hh"

#include <QQmlContext>
#include <QScreen>
#include <QGuiApplication>
#include <QTimer>
#include <QUrl>

#include "core/ICore.hh"
#include "core/IBreak.hh"
#include "core/CoreTypes.hh"
#include "session/System.hh"
#include "debug.hh"

#if defined(HAVE_WAYLAND)
#  include "IToolkitUnixPrivate.hh"
#endif

using namespace workrave;
using namespace workrave::utils;

// ── DailyLimitBridge ──────────────────────────────────────────────────────────

DailyLimitBridge::DailyLimitBridge(std::shared_ptr<IApplicationContext> app,
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
DailyLimitBridge::blockMode() const
{
  return static_cast<int>(block_mode);
}

bool
DailyLimitBridge::lockable() const
{
  return System::is_lockable();
}

bool
DailyLimitBridge::canPostpone() const
{
  return (break_flags & BREAK_FLAGS_POSTPONABLE) != 0 && !postpone_locked;
}

bool
DailyLimitBridge::canSkip() const
{
  return (break_flags & BREAK_FLAGS_SKIPPABLE) != 0 && !skip_locked;
}

double
DailyLimitBridge::lockProgress() const
{
  return lock_progress_val;
}

bool
DailyLimitBridge::isLocked() const
{
  return postpone_locked || skip_locked;
}

void
DailyLimitBridge::setBreakButtonState(const BreakButtonState &state)
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
DailyLimitBridge::updateUserActivity()
{
  bool active = app->get_core()->is_user_active();
  if (active != user_active_)
    {
      user_active_ = active;
      Q_EMIT userActivityChanged();
    }
}

void
DailyLimitBridge::requestPostpone()
{
  QTimer::singleShot(0, this, [this]() {
    app->get_core()->get_break(BREAK_ID_DAILY_LIMIT)->postpone_break();
    if (on_dismiss_)
      on_dismiss_();
  });
}

void
DailyLimitBridge::requestSkip()
{
  QTimer::singleShot(0, this, [this]() {
    app->get_core()->get_break(BREAK_ID_DAILY_LIMIT)->skip_break();
    if (on_dismiss_)
      on_dismiss_();
  });
}

void
DailyLimitBridge::requestLock()
{
  if (System::is_lockable())
    {
      auto locker = app->get_toolkit()->get_locker();
      locker->unlock();
      locker->lock();
      System::lock_screen();
    }
}

// ── QmlDailyLimitWindow ───────────────────────────────────────────────────────

QmlDailyLimitWindow::QmlDailyLimitWindow(std::shared_ptr<IApplicationContext> app,
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

QmlDailyLimitWindow::~QmlDailyLimitWindow()
{
  delete view;
}

void
QmlDailyLimitWindow::init()
{
  TRACE_ENTRY();

  bridge = new DailyLimitBridge(app, block_mode, break_flags);
  bridge->setDismissHandler([this]() { stop(); });

  view = new QQuickView();
  view->setResizeMode(QQuickView::SizeRootObjectToView);
  view->rootContext()->setContextProperty("bridge", bridge);

  QObject::connect(view, &QQuickView::statusChanged, view, [this](QQuickView::Status status) {
    if (status == QQuickView::Error)
      {
        for (const auto &err : view->errors())
          {
            spdlog::error("DailyLimitOverlay QML error: {}", err.toString().toStdString());
          }
      }
  });

  view->setSource(QUrl("qrc:/sanctuary/DailyLimitShell.qml"));

  configure_view_for_block_mode();
}

void
QmlDailyLimitWindow::configure_view_for_block_mode()
{
  view->setFlags(Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
  view->setColor(Qt::transparent);
}

void
QmlDailyLimitWindow::start()
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
      QRect geo = (screen != nullptr) ? screen->geometry() : QGuiApplication::primaryScreen()->geometry();
      const int card_w = 560;
      const int card_h = 360;
      int x = geo.x() + (geo.width() - card_w) / 2;
      int y = geo.y() + (geo.height() - card_h) / 2;
      view->setGeometry(x, y, card_w, card_h);
      view->show();
    }

  view->raise();
}

void
QmlDailyLimitWindow::stop()
{
  TRACE_ENTRY();
  view->hide();

#if defined(HAVE_WAYLAND)
  if (window_manager)
    window_manager->clear_surfaces();
#endif
}

void
QmlDailyLimitWindow::refresh()
{
  bridge->updateUserActivity();
}

void
QmlDailyLimitWindow::set_progress(int value, int max_value)
{
  (void)value;
  (void)max_value;
}

void
QmlDailyLimitWindow::set_break_button_state(const BreakButtonState &state)
{
  bridge->setBreakButtonState(state);
}
