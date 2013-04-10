// Frame.cc
//
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
#include "config.h"
#endif

#include "Frame.hh"

#include <QTimer>
#include <QPainter>

Frame::Frame(QWidget* parent)
  : QWidget(parent),
    heartbeat_timer(new QTimer(this)),
    frame_width(0),
    border_width(0),
    frame_color(QColor("black")),
    frame_style(STYLE_SOLID),
    frame_visible(true),
    flash_delay(-1)
{
  connect(heartbeat_timer.get(), SIGNAL(timeout()), this, SLOT(on_timer()));
}


Frame::~Frame()
{
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
    case STYLE_BREAK_WINDOW:
      dfw = 3;
      break;
    case STYLE_SOLID:
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
  QRect fr = contentsRect();
  fr.adjust(-frame_width - border_width,
            -frame_width - border_width,
            frame_width + border_width,
            frame_width + border_width);

  frame_width = frame;
  border_width = border;
  
  QRect cr = fr.isValid() ? fr : rect();

  cr.adjust(frame_width + border_width,
            frame_width + border_width,
            -frame_width - border_width,
            -frame_width - border_width);
  
  setContentsMargins(cr.left(), cr.top(), rect().right() - cr.right(), rect().bottom() - cr.bottom());
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
  set_frame_visible(! frame_visible);
  update();
  flash_signal(frame_visible);
}

void
Frame::paintEvent(QPaintEvent *)
{
  QPainter paint(this);

  switch (frame_style)
    {
    case STYLE_BREAK_WINDOW:
    case STYLE_SOLID:
      
      if (frame_visible)
        {
          paint.fillRect(0, 0, frame_width, height(), frame_color);
          paint.fillRect(0+width()-frame_width, 0, frame_width, height(), frame_color);
          paint.fillRect(0+frame_width, 0, width()-2*frame_width, frame_width, frame_color);
          paint.fillRect(0+frame_width, 0+height()-frame_width, width()-2*frame_width, frame_width, frame_color);
        }
      break;

      //case STYLE_BREAK_WINDOW:
      // style_context->context_save();

      // style_context->add_class(GTK_STYLE_CLASS_FRAME);
      // style_context->set_state((Gtk::StateFlags)0);

      // style_context->render_background(cr, 0, 0, width, height);
      // style_context->render_frame(cr, 0, 0, width, height);

      // style_context->render_background(cr, 1, 1, width - 2, height -2);
      // style_context->render_frame(cr, 1, 1, width - 2, height -2);

      // style_context->context_restore();
      //break;
    }
}

boost::signals2::signal<void(bool)> &
Frame::signal_flash()
{
  return flash_signal;
}
 
