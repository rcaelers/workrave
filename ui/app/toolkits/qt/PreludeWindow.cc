// Copyright (C) 2006, 2007, 2013 Raymond Penners & Rob Caelers
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
//

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "PreludeWindow.hh"

#include <QtGui>
#include <QStyle>
#include <QApplication>

#include "debug.hh"

#include "core/IApp.hh"
#include "utils/AssetPath.hh"

#include "UiUtil.hh"
#include "qformat.hh"

using namespace workrave;
using namespace workrave::utils;

#if defined(PLATFORM_OS_MACOS)
#  import <Cocoa/Cocoa.h>
#endif

PreludeWindow::PreludeWindow(QScreen *screen, workrave::BreakId break_id)
  : QWidget(nullptr, Qt::Window)
  , break_id(break_id)
  , screen(screen)
{
  auto *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  timer->start(1000);

  layout = new QVBoxLayout();
  layout->setContentsMargins(1, 1, 1, 1);

  timebar = new TimeBar;

  QString text;
  switch (break_id)
    {
    case BREAK_ID_MICRO_BREAK:
      text = tr("Time for a micro-break?");
      break;

    case BREAK_ID_REST_BREAK:
      text = tr("You need a rest break...");
      break;

    case BREAK_ID_DAILY_LIMIT:
      text = tr("You should stop for today...");
      break;

    case BREAK_ID_NONE:
      break;
    }

  label = UiUtil::create_label(text, true);
  image = UiUtil::create_image_label("prelude-hint.png");

  frame = new Frame;
  frame->set_frame_style(Frame::Style::Solid);
  frame->set_frame_width(6, 6);
  frame->set_frame_visible(false);
  workrave::utils::connect(frame->signal_flash(), this, [this](auto visible) { on_frame_flash(visible); });

  auto *frameLayout = new QVBoxLayout;
  frame->setLayout(frameLayout);

  layout->addWidget(frame);

  auto *vlayout = new QVBoxLayout;
  vlayout->setSpacing(6);
  vlayout->addWidget(label);
  vlayout->addWidget(timebar);

  auto *hlayout = new QHBoxLayout;
  hlayout->setSpacing(6);

  hlayout->addWidget(image);
  hlayout->addLayout(vlayout);

  frameLayout->addLayout(hlayout);

  setLayout(layout);

  // NSView *nsview = (__bridge NSView *)reinterpret_cast<void *>(winId());
  // NSWindow *nswindow = [nsview window];
  // [nswindow setCollectionBehavior: (NSWindowCollectionBehaviorCanJoinAllSpaces)];

  setWindowFlags(
#if defined(PLATFORM_OS_MACOS)
    Qt::SubWindow |
#else
    Qt::Tool |
#endif
    Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus
    | Qt::X11BypassWindowManagerHint);

  setAttribute(Qt::WA_Hover);
  setAttribute(Qt::WA_ShowWithoutActivating);

#if defined(PLATFORM_OS_MACOS)
  mouse_monitor = std::make_shared<MouseMonitor>([this](auto x, auto y) { avoid_pointer(x, y); });
#endif
}

void
PreludeWindow::start()
{
  timebar->set_bar_color(TimerColorId::Overdue);
  refresh();
  show();

  QRect rect = screen->geometry();
  setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), rect));

#if defined(PLATFORM_OS_MACOS)
  mouse_monitor->start();
#endif
}

void
PreludeWindow::stop()
{
  frame->set_frame_flashing(0);
  hide();

#if defined(PLATFORM_OS_MACOS)
  mouse_monitor->stop();
#endif
}

void
PreludeWindow::refresh()
{
  QString s;

  timebar->set_progress(progress_value, progress_max_value);

  int tminus = progress_max_value - progress_value;
  if (tminus >= 0 || (tminus < 0 && flash_visible))
    {
      if (tminus < 0)
        {
          tminus = 0;
        }

      s = qstr(qformat(progress_text) % UiUtil::time_to_string(tminus));
    }
  timebar->set_text(s);
  timebar->update();
}

void
PreludeWindow::set_progress(int value, int max_value)
{
  progress_value = value;
  progress_max_value = max_value;
  refresh();
}

void
PreludeWindow::set_progress_text(IApp::PreludeProgressText text)
{
  switch (text)
    {
    case IApp::PROGRESS_TEXT_BREAK_IN:
      progress_text = tr("Break in %s");
      break;

    case IApp::PROGRESS_TEXT_DISAPPEARS_IN:
      progress_text = tr("Disappears in %s");
      break;

    case IApp::PROGRESS_TEXT_SILENT_IN:
      progress_text = tr("Silent in %s");
      break;
    }
}

void
PreludeWindow::set_stage(IApp::PreludeStage stage)
{
  const char *icon = nullptr;
  switch (stage)
    {
    case IApp::STAGE_INITIAL:
      frame->set_frame_flashing(0);
      frame->set_frame_visible(false);
      icon = "prelude-hint.png";
      break;

    case IApp::STAGE_WARN:
      frame->set_frame_visible(true);
      frame->set_frame_flashing(500);
      frame->set_frame_color(QColor("orange"));
      icon = "prelude-hint-sad.png";
      break;

    case IApp::STAGE_ALERT:
      frame->set_frame_flashing(500);
      frame->set_frame_color(QColor("red"));
      icon = "prelude-hint-sad.png";
      break;

    case IApp::STAGE_MOVE_OUT:
      if (!did_avoid)
        {
          const QRect rect = screen->geometry();
          move(x(), rect.y() + SCREEN_MARGIN);
        }
      break;
    }

  if (icon != nullptr)
    {
      std::string file = AssetPath::complete_directory(icon, SearchPathId::Images);
      image->setPixmap(QPixmap(file.c_str()));
    }
}

void
PreludeWindow::on_frame_flash(bool frame_visible)
{
  flash_visible = frame_visible;
  refresh();
}

auto
PreludeWindow::event(QEvent *event) -> bool
{
  if (event->type() == QEvent::HoverEnter)
    {
      auto *hoverEvent = dynamic_cast<QHoverEvent *>(event);
      avoid_pointer(hoverEvent->position().x(), hoverEvent->position().y());
    }
  bool res = QWidget::event(event);
  return res;
}

void
PreludeWindow::avoid_pointer(int px, int py)
{
  const QRect &geo = geometry();

  const QRect rect = screen->geometry();

  py = rect.height() - py;

  int screen_height = rect.height();
  int top_y = rect.y() + SCREEN_MARGIN;
  int bottom_y = rect.y() + screen_height - geo.height() - SCREEN_MARGIN;
  int winy = geo.y();
  int winx = geo.x();

#if defined(PLATFORM_OS_MACOS)
  if (!geo.contains(px, py))
    {
      return;
    }
#endif

  if (winy < top_y + SCREEN_MARGIN)
    {
      winy = bottom_y;
    }
  else if (winy > bottom_y - SCREEN_MARGIN)
    {
      winy = top_y;
    }
  else
    {
      if (py > winy + geo.height() / 2)
        {
          winy = top_y;
        }
      else
        {
          winy = bottom_y;
        }
    }

  move(winx, winy);
  did_avoid = true;
}
