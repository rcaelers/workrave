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

using namespace workrave;

// ── PreludeBridge ─────────────────────────────────────────────────────────────

PreludeBridge::PreludeBridge(BreakId break_id, QObject *parent)
  : QObject(parent)
  , break_id(break_id)
{
  progress_label = tr("Break in");
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
  return progress_label;
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
    case IApp::PreludeStage::Initial:  s = 0; break;
    case IApp::PreludeStage::Warn:     s = 1; break;
    case IApp::PreludeStage::Alert:    s = 2; break;
    case IApp::PreludeStage::MoveOut:  s = 3; break;
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
      progress_label = tr("Break in");
      break;
    case IApp::PreludeProgressText::DisappearsIn:
      progress_label = tr("Disappears in");
      break;
    case IApp::PreludeProgressText::SilentIn:
      progress_label = tr("Silent in");
      break;
    }
  Q_EMIT progressChanged();
}

// ── QmlPreludeWindow ──────────────────────────────────────────────────────────

QmlPreludeWindow::QmlPreludeWindow(QScreen *screen, BreakId break_id)
  : screen(screen)
{
  bridge = new PreludeBridge(break_id);

  view = new QQuickView();
  view->setResizeMode(QQuickView::SizeRootObjectToView);
  // Qt::SplashScreen maps to a high NSWindowLevel on macOS so the card appears
  // above other applications' windows even when Workrave is not the active app.
  view->setFlags(Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);
  view->setColor(Qt::transparent);
  view->rootContext()->setContextProperty("bridge", bridge);
  view->setSource(QUrl("qrc:/sanctuary/PreludeOverlay.qml"));

  QObject::connect(bridge, &PreludeBridge::skipRequested, view, [this]() { view->hide(); });
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
  view->setGeometry(top_rect());
  view->show();
  view->raise();
}

void
QmlPreludeWindow::stop()
{
  view->hide();
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

  if (stage == IApp::PreludeStage::MoveOut)
    {
      at_bottom = !at_bottom;
      view->setGeometry(at_bottom ? bottom_rect() : top_rect());
    }
}

void
QmlPreludeWindow::set_progress_text(IApp::PreludeProgressText text)
{
  bridge->setProgressText(text);
}
