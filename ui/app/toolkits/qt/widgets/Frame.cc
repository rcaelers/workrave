// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#include "Frame.hh"

#include <QTimer>
#include <QPainter>
#include <QStylePainter>
#include <QStyleOptionFrame>

Frame::Frame(QWidget *parent)
  : QWidget(parent)
  , heartbeat_timer(new QTimer(this))
{
  connect(heartbeat_timer.get(), SIGNAL(timeout()), this, SLOT(on_timer()));
}

void
Frame::set_frame_visible(bool visible)
{
  frame_visible = visible;
  update();
}

void
Frame::set_frame_style(const Style style)
{
  frame_style = style;
  int dfw = 1;
  switch (style)
    {
    case Style::BreakWindow:
      dfw = 3;
      break;
    case Style::Solid:
      dfw = 1;
      break;
    }
  set_frame_width(dfw, border_width);
}

void
Frame::set_frame_color(const QColor &col)
{
  frame_color = col;
}

void
Frame::set_frame_width(int frame, int border)
{
  QRect fr = get_frame_rect();

  frame_width = frame;
  border_width = border;

  QRect cr = fr.isValid() ? fr : rect();

  cr.adjust(frame_width + border_width, frame_width + border_width, -frame_width - border_width, -frame_width - border_width);

  setContentsMargins(cr.left(), cr.top(), rect().right() - cr.right(), rect().bottom() - cr.bottom());
}

auto
Frame::get_frame_rect() const -> QRect
{
  QRect fr = contentsRect();
  fr.adjust(-frame_width - border_width, -frame_width - border_width, frame_width + border_width, frame_width + border_width);

  return fr;
}

void
Frame::set_frame_flashing(int delay)
{
  if (delay != 0)
    {
      if (flash_delay != delay)
        {
          heartbeat_timer->start(delay);
        }
    }
  else
    {
      heartbeat_timer->stop();
      set_frame_visible(true);
    }
  flash_delay = delay;
}

void
Frame::on_timer()
{
  set_frame_visible(!frame_visible);
  update();
  flash_signal(frame_visible);
}

void
Frame::paintEvent(QPaintEvent *pe)
{
  QStylePainter paint(this);

  QRect fr = get_frame_rect();

  QStyleOptionFrame opt;
  opt.initFrom(this);

  opt.rect = fr;
  opt.lineWidth = 1;
  opt.midLineWidth = 0;
  opt.state |= QStyle::State_Raised;
  opt.frameShape = QFrame::Panel;

  paint.drawControl(QStyle::CE_ShapedFrame, opt);

  if (frame_visible)
    {
      paint.fillRect(border_width, border_width, frame_width, height() - 2 * border_width, frame_color);
      paint.fillRect(width() - border_width - frame_width, border_width, frame_width, height() - 2 * border_width, frame_color);
      paint.fillRect(border_width + frame_width,
                     border_width,
                     width() - 2 * frame_width - 2 * border_width,
                     frame_width,
                     frame_color);
      paint.fillRect(border_width + frame_width,
                     height() - frame_width - border_width,
                     width() - 2 * frame_width - 2 * border_width,
                     frame_width,
                     frame_color);
    }
}

auto
Frame::signal_flash() -> boost::signals2::signal<void(bool)> &
{
  return flash_signal;
}
