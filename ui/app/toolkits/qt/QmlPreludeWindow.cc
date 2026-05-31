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

#include "QmlPreludeWindow.hh"

#include <QQmlContext>
#include <QGuiApplication>
#include <QTimer>

#include "utils/Platform.hh"
#include <fmt/format.h>

#if defined(HAVE_WAYLAND)
#  include "IToolkitUnixPrivate.hh"
#endif

using namespace workrave;

// ── PreludeBridge ─────────────────────────────────────────────────────────────

PreludeBridge::PreludeBridge(BreakId break_id, QObject *parent)
  : QObject(parent)
  , break_id(break_id)
  , classic_(!GUIConfig::sanctuary_ui_enabled()())
{
  progress_label = tr("Break in {}");

  // Live-switch between Sanctuary and Classic designs without restarting.
  GUIConfig::sanctuary_ui_enabled().connect(this, [this](bool enabled) {
    bool new_classic = !enabled;
    if (new_classic != classic_)
      {
        classic_ = new_classic;
        Q_EMIT classicChanged();
      }
  });
}

QString
PreludeBridge::heading() const
{
  switch (break_id)
    {
    case BREAK_ID_MICRO_BREAK: return tr("Time for a micro-break?");
    case BREAK_ID_REST_BREAK:  return tr("You need a rest break...");
    case BREAK_ID_DAILY_LIMIT: return tr("You should stop for today...");
    default:                   return tr("Time for a break?");
    }
}

double
PreludeBridge::ringProgress() const
{
  if (progress_max <= 0)
    return 1.0;
  double remaining = static_cast<double>(progress_max - progress_value) / progress_max;
  return qBound(0.0, remaining, 1.0);
}

QString
PreludeBridge::timeLabel() const
{
  int remaining = std::max(progress_max - progress_value, 0);
  return UiUtil::time_to_string(static_cast<time_t>(remaining));
}

QString
PreludeBridge::timeShort() const
{
  int remaining = std::max(progress_max - progress_value, 0);
  if (remaining < 60)
    return QString("%1s").arg(remaining);
  return UiUtil::time_to_string(static_cast<time_t>(remaining));
}

QString
PreludeBridge::countdownText() const
{
  int remaining = std::max(progress_max - progress_value, 0);
  auto time_str = UiUtil::time_to_string(static_cast<time_t>(remaining)).toStdString();
  return QString::fromStdString(fmt::format(fmt::runtime(progress_label.toStdString()), time_str));
}

void
PreludeBridge::requestSkip()
{
  Q_EMIT skipRequested();
}

void
PreludeBridge::setProgress(int value, int max_value)
{
  progress_value = value;
  progress_max   = max_value;
  Q_EMIT progressChanged();
}

void
PreludeBridge::setStage(IApp::PreludeStage stage)
{
  int s = 0;
  switch (stage)
    {
    case IApp::PreludeStage::Initial: s = 0; break;
    case IApp::PreludeStage::Warn:    s = 1; break;
    case IApp::PreludeStage::Alert:   s = 2; break;
    case IApp::PreludeStage::MoveOut: s = 3; break;
    }
  if (stage_ != s)
    {
      stage_ = s;
      Q_EMIT stageChanged();
    }
}

void
PreludeBridge::setProgressText(IApp::PreludeProgressText text)
{
  switch (text)
    {
    case IApp::PreludeProgressText::BreakIn:
      progress_label = tr("Break in {}");
      break;
    case IApp::PreludeProgressText::DisappearsIn:
      progress_label = tr("Disappears in {}");
      break;
    case IApp::PreludeProgressText::SilentIn:
      progress_label = tr("Silent in {}");
      break;
    }
  Q_EMIT progressChanged();
}

// ── QmlPreludeWindow ──────────────────────────────────────────────────────────

QmlPreludeWindow::QmlPreludeWindow(std::shared_ptr<IApplicationContext> app, QScreen *screen, BreakId break_id)
  : screen(screen)
  , position_windows(workrave::utils::Platform::can_position_windows())
{
#if defined(HAVE_WAYLAND)
  window_manager = std::dynamic_pointer_cast<IToolkitUnixPrivate>(app->get_toolkit())->get_wayland_window_manager();
#else
  (void)app;
#endif

  bridge = new PreludeBridge(break_id);
  if (!position_windows)
    bridge->setFullscreen(true);

  view = new QQuickView();
  view->setResizeMode(QQuickView::SizeRootObjectToView);
  // Qt::SplashScreen maps to a high NSWindowLevel on macOS so the card appears
  // above other applications' windows even when Workrave is not the active app.
  view->setFlags(Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
  view->setColor(Qt::transparent);
  view->rootContext()->setContextProperty("bridge", bridge);
  view->setSource(QUrl("qrc:/sanctuary/PreludeShell.qml"));

  QObject::connect(bridge, &PreludeBridge::skipRequested, view, [this]() { view->hide(); });

#if defined(PLATFORM_OS_MACOS)
  // macOS: use a Cocoa global event monitor for continuous mouse tracking.
  // NSEvent.mouseLocation gives bottom-left-origin screen coordinates.
  mouse_monitor = std::make_shared<MouseMonitor>([this](auto x, auto y) { avoid_pointer(x, y); });
#else
  // Linux/Windows: install a QEvent::Enter filter so we're notified when the
  // mouse cursor enters the window and can move it out of the way.
  auto *filter = new ViewEventFilter([this]() { avoid_pointer(0, 0); }, bridge);
  view->installEventFilter(filter);
#endif
}

QmlPreludeWindow::~QmlPreludeWindow()
{
  delete view;
  delete bridge;
}

QRect
QmlPreludeWindow::top_rect() const
{
  QRect geo = screen ? screen->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();
  int x     = geo.left() + (geo.width() - CARD_W) / 2;
  int y     = geo.top() + MARGIN;
  return {x, y, CARD_W, CARD_H};
}

QRect
QmlPreludeWindow::bottom_rect() const
{
  QRect geo = screen ? screen->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();
  int x     = geo.left() + (geo.width() - CARD_W) / 2;
  int y     = geo.bottom() - CARD_H - MARGIN;
  return {x, y, CARD_W, CARD_H};
}

void
QmlPreludeWindow::start()
{
  at_bottom = false;
  did_avoid = false;

#if defined(HAVE_WAYLAND)
  if (window_manager)
    window_manager->init_surface(view, screen, false);
#endif

  if (position_windows)
    {
      view->setGeometry(top_rect());
    }
  else
    {
      // Wayland: make the view cover the full screen; the card is positioned
      // within the transparent fullscreen window via QML properties.
      QRect geo = screen ? screen->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();
      view->setGeometry(geo);
      bridge->setCardAtBottom(false);
    }

  view->show();
  view->raise();
  update_input_region();

#if defined(PLATFORM_OS_MACOS)
  mouse_monitor->start();
#endif
}

void
QmlPreludeWindow::stop()
{
  view->hide();

#if defined(HAVE_WAYLAND)
  if (window_manager)
    window_manager->clear_surfaces();
#endif

#if defined(PLATFORM_OS_MACOS)
  mouse_monitor->stop();
#endif
}

void
QmlPreludeWindow::set_progress(int value, int max_value)
{
  bridge->setProgress(value, max_value);
}

void
QmlPreludeWindow::set_stage(IApp::PreludeStage stage)
{
  bridge->setStage(stage);

  // MoveOut: if the user has not already caused pointer avoidance, move the
  // card/window to the top edge (matching the original PreludeWindow behavior).
  if (stage == IApp::PreludeStage::MoveOut && !did_avoid)
    {
      if (position_windows)
        {
          view->setGeometry(top_rect());
          at_bottom = false;
        }
      else
        {
          bridge->setCardAtBottom(false);
          at_bottom = false;
          QTimer::singleShot(0, view, [this] { update_input_region(); });
        }
    }
}

void
QmlPreludeWindow::set_progress_text(IApp::PreludeProgressText text)
{
  bridge->setProgressText(text);
}

void
QmlPreludeWindow::avoid_pointer(int px, int py)
{
  did_avoid = true;

  // Wayland fullscreen: toggle the card between top and bottom within the
  // transparent full-screen window rather than moving the window itself.
  if (!position_windows)
    {
      bool new_at_bottom = !bridge->isCardAtBottom();
      bridge->setCardAtBottom(new_at_bottom);
      at_bottom = new_at_bottom;
      QTimer::singleShot(0, view, [this] { update_input_region(); });
      return;
    }

#if defined(PLATFORM_OS_MACOS)
  // macOS NSEvent gives bottom-left-origin screen coordinates. Convert py to
  // Qt's top-left origin before checking whether the pointer is inside our window.
  {
    const QRect win = view->geometry();
    const QRect scr = screen ? screen->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();
    int qt_y        = scr.height() - py;
    if (!win.contains(px, qt_y))
      return;
  }
#else
  (void)px;
  (void)py;
#endif

  // Toggle between top and bottom based on where the window currently is.
  if (at_bottom)
    {
      view->setGeometry(top_rect());
      at_bottom = false;
    }
  else
    {
      view->setGeometry(bottom_rect());
      at_bottom = true;
    }
}

void
QmlPreludeWindow::update_input_region()
{
  if (!position_windows)
    {
      // Restrict mouse input to the card area so the transparent surroundings
      // do not steal events from the desktop.
      QRect geo = screen ? screen->availableGeometry() : QGuiApplication::primaryScreen()->availableGeometry();
      int card_x = (geo.width() - CARD_W) / 2;
      int card_y = at_bottom ? (geo.height() - CARD_H - MARGIN) : MARGIN;
      view->setMask(QRegion(card_x, card_y, CARD_W, CARD_H));
    }
}
