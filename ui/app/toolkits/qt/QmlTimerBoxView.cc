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

#include "QmlTimerBoxView.hh"

#include <array>
#include <cstdlib>

#include <QCursor>
#include <QGuiApplication>
#include <QQmlContext>
#include <QScreen>
#include <QVBoxLayout>

#include "core/CoreTypes.hh"
#include "debug.hh"
#include "UiUtil.hh"

using namespace workrave;

// ── StatusWindowBridge ────────────────────────────────────────────────────────

StatusWindowBridge::StatusWindowBridge(std::shared_ptr<workrave::ICore> core, QWidget *owner_window, QObject *parent)
  : QObject(parent)
  , core(std::move(core))
  , owner_window(owner_window)
{
  slot_to_id.fill(BREAK_ID_NONE);

  GUIConfig::display_style().connect(this, [this](DisplayStyle) {
    Q_EMIT displayStyleChanged();
  });
}

void
StatusWindowBridge::setSlot(BreakId id, int slot)
{
  if (slot >= 0 && slot < static_cast<int>(slot_to_id.size()))
    {
      slot_to_id[slot] = static_cast<int>(id);
    }
  // visibility is derived from slot_to_id in commitUpdate()
}

void
StatusWindowBridge::setTimeBar(BreakId id,
                                int value,
                                TimerColorId primary_color,
                                int primary_val,
                                int primary_max,
                                int secondary_val,
                                int secondary_max)
{
  if (id < 0 || id >= BREAK_ID_SIZEOF)
    return;

  auto &t = timers[id];

  // Outer ring: remaining time (full = just reset, empty = break due)
  t.progress = (primary_max > 0)
                 ? qBound(0.0, static_cast<double>(primary_max - primary_val) / primary_max, 1.0)
                 : 0.0;

  // Inner ring: idle/rest progress (0 = not resting, 1 = break fully satisfied)
  t.idle_progress = (secondary_max > 0)
                      ? qBound(0.0, static_cast<double>(secondary_val) / secondary_max, 1.0)
                      : 0.0;

  t.timeStr = UiUtil::time_to_string(static_cast<time_t>(value));
  t.overdue  = (primary_color == TimerColorId::Overdue) || (primary_color == TimerColorId::InactiveOverOverdue);
}

void
StatusWindowBridge::commitUpdate()
{
  // Recompute visibility from current slot assignments
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      timers[i].visible = false;
    }
  for (int i = 0; i < static_cast<int>(slot_to_id.size()); i++)
    {
      int id = slot_to_id[i];
      if (id >= 0 && id < BREAK_ID_SIZEOF)
        {
          timers[id].visible = true;
        }
    }
  Q_EMIT dataChanged();
}

int
StatusWindowBridge::displayStyle() const
{
  return static_cast<int>(GUIConfig::display_style()());
}

void
StatusWindowBridge::requestClose()
{
  if (owner_window != nullptr)
    {
      owner_window->close();
    }
}

void
StatusWindowBridge::forceRestBreak()
{
  if (core)
    {
      core->force_break(BREAK_ID_REST_BREAK, BreakHint::UserInitiated);
    }
}

void
StatusWindowBridge::startWindowDrag()
{
  last_cursor_pos = QCursor::pos();
}

void
StatusWindowBridge::continueWindowDrag()
{
  if (owner_window == nullptr)
    return;
  QPoint current = QCursor::pos();
  owner_window->move(owner_window->pos() + current - last_cursor_pos);
  last_cursor_pos = current;
}

void
StatusWindowBridge::stopWindowDrag()
{
  if (owner_window == nullptr)
    {
      return;
    }

  const QRect frame = owner_window->frameGeometry();
  QScreen *screen = QGuiApplication::screenAt(frame.center());
  if (screen == nullptr)
    {
      screen = QGuiApplication::primaryScreen();
    }
  if (screen == nullptr)
    {
      return;
    }

  // geo  = full screen including dock/menubar area
  // avail = available area (excludes menubar on top; we still respect that for top snaps)
  const QRect geo = screen->geometry();
  const QRect avail = screen->availableGeometry();

  // Snap only if a window corner is within snap_zone of the corresponding screen corner.
  // Bottom/right targets use the full screen geometry so the window slides behind the dock.
  // Top targets use availableGeometry.top() so the window stays below the menu bar.
  // snap_zone is large enough to cover a typical macOS dock height (~80 px).
  const int snap_zone = 120;

  struct Candidate
  {
    QPoint frame_corner;  // the window corner being tested
    QPoint screen_corner; // the reference corner for proximity test
    QPoint target;        // where to move frameGeometry().topLeft()
  };

  const std::array<Candidate, 4> candidates{{
    {frame.topLeft(), QPoint(geo.left(), avail.top()), QPoint(geo.left(), avail.top())},
    {frame.topRight(),
     QPoint(geo.right(), avail.top()),
     QPoint(geo.right() - frame.width() + 1, avail.top())},
    {frame.bottomLeft(),
     geo.bottomLeft(),
     QPoint(geo.left(), geo.bottom() - frame.height() + 1)},
    {frame.bottomRight(),
     geo.bottomRight(),
     QPoint(geo.right() - frame.width() + 1, geo.bottom() - frame.height() + 1)},
  }};

  for (const auto &c : candidates)
    {
      const QPoint diff = c.frame_corner - c.screen_corner;
      if (std::abs(diff.x()) <= snap_zone && std::abs(diff.y()) <= snap_zone)
        {
          owner_window->move(c.target);
          return;
        }
    }
  // No corner nearby — leave the window at its current position.
}

// ── QmlTimerBoxView ───────────────────────────────────────────────────────────

QmlTimerBoxView::QmlTimerBoxView(std::shared_ptr<ICore> core, QWidget *owner_window, QWidget *parent)
  : QWidget(parent)
  , core(std::move(core))
{
  // Make this container widget itself transparent so the QML rounded corners
  // are not clipped by an opaque widget background.
  setAttribute(Qt::WA_TranslucentBackground);
  setAutoFillBackground(false);

  bridge = new StatusWindowBridge(this->core, owner_window, this);

  qml_widget = new QQuickWidget(this);
  qml_widget->setResizeMode(QQuickWidget::SizeViewToRootObject);
  // WA_AlwaysStackOnTop makes the QQuickWidget composite directly against the
  // window compositor rather than baking into the backing store — without this
  // the alpha channel from QML rounded corners is lost and corners appear opaque.
  qml_widget->setAttribute(Qt::WA_AlwaysStackOnTop);
  qml_widget->setAttribute(Qt::WA_TranslucentBackground);
  qml_widget->setClearColor(Qt::transparent);
  qml_widget->rootContext()->setContextProperty("bridge", bridge);
  qml_widget->setSource(QUrl("qrc:/sanctuary/StatusWindow.qml"));

  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(qml_widget);
  setLayout(layout);

  // Once QML finishes loading, connect to the root object's implicit size signals
  // so that any change in visible timers or display style propagates to C++.
  connect(qml_widget, &QQuickWidget::statusChanged, this, [this](QQuickWidget::Status status) {
    if (status == QQuickWidget::Ready)
      {
        auto *root = qml_widget->rootObject();
        if (root == nullptr)
          return;
        connect(root, &QQuickItem::implicitWidthChanged, this, &QmlTimerBoxView::on_qml_size_changed);
        connect(root, &QQuickItem::implicitHeightChanged, this, &QmlTimerBoxView::on_qml_size_changed);
        on_qml_size_changed();
      }
  });
}

void
QmlTimerBoxView::on_qml_size_changed()
{
  auto *root = qml_widget->rootObject();
  if (root == nullptr)
    return;
  int w = static_cast<int>(root->implicitWidth());
  int h = static_cast<int>(root->implicitHeight());
  if (w <= 0 || h <= 0)
    return;
  qml_widget->setFixedSize(w, h);
  setFixedSize(w, h);
  if (auto *p = parentWidget())
    {
      p->setFixedSize(p->sizeHint());
    }
}

void
QmlTimerBoxView::set_slot(BreakId id, int slot)
{
  bridge->setSlot(id, slot);
}

void
QmlTimerBoxView::set_time_bar(BreakId id,
                               int value,
                               TimerColorId primary_color,
                               int primary_value,
                               int primary_max,
                               TimerColorId /*secondary_color*/,
                               int secondary_value,
                               int secondary_max)
{
  bridge->setTimeBar(id, value, primary_color, primary_value, primary_max, secondary_value, secondary_max);
}

void
QmlTimerBoxView::update_view()
{
  bridge->commitUpdate();
}
