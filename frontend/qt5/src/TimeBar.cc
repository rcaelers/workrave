// TimeBar.cc --- The WorkRave GUI
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

#include "TimeBar.hh"

#include <QStylePainter>
#include <QStyleOptionProgressBar>

#include "Text.hh"
#include "debug.hh"

const int MARGINX = 4;
const int MARGINY = 2;

QColor TimeBar::bar_colors[TimeBar::COLOR_ID_SIZEOF] =
  {
    QColor("lightblue"),
    QColor("lightgreen"),
    QColor("orange"),
    QColor("red"),
    QColor("#e00000"),
    QColor("#00d4b2"),
    QColor("lightgreen"),
  };

TimeBar::TimeBar(QWidget *parent) :
  QWidget(parent),
  bar_color(COLOR_ID_ACTIVE),
  secondary_bar_color(COLOR_ID_ACTIVE),
  bar_value(0),
  bar_max_value(0),
  secondary_bar_value(0),
  secondary_bar_max_value(0),
  bar_text_align(0)

{
  setBackgroundRole(QPalette::Base);
  setAutoFillBackground(true);
}

TimeBar::~TimeBar()
{
}


//! Sets the time progress to be displayed.
void
TimeBar::set_progress(int value, int max_value)
{
  if (value > max_value)
    {
      value = max_value;
    }

  bar_value = value;
  bar_max_value = max_value;
}


//! Sets the secondary time progress to be displayed.
void
TimeBar::set_secondary_progress(int value, int max_value)
{
  if (value > max_value)
    {
      value = max_value;
    }

  secondary_bar_value = value;
  secondary_bar_max_value = max_value;
}


//! Sets the text to be displayed.
void
TimeBar::set_text(std::string text)
{
  bar_text = text;
}


//! Sets text alignment
void
TimeBar::set_text_alignment(int align)
{
  bar_text_align = align;
}

//! Sets the color of the bar.
void
TimeBar::set_bar_color(ColorId color)
{
  bar_color = color;
}


//! Sets the color of the secondary bar.
void
TimeBar::set_secondary_bar_color(ColorId color)
{
  secondary_bar_color = color;
}


//! Updates the screen.
void
TimeBar::update()
{
  QWidget::update();
}


QSize
TimeBar::minimumSizeHint() const
{
  TRACE_ENTER("TimeBar::minimumSizeHint");
  QString text = QString::fromStdString(bar_text);
  int width = fontMetrics().width(text);
  int height = fontMetrics().height();

  QString full_text = QString::fromStdString(Text::time_to_string(-(59+59*60+9*60*60)));
  int full_width = fontMetrics().width(full_text);

  if (full_width > width)
    {
      width = full_width;
    }

  width = width + 2 * MARGINX;
  height = std::max(height + 2 * MARGINY, 20);
  
  TRACE_MSG("width = " << width << "height = " << height);
  TRACE_EXIT();
  return QSize(width, height);
}

QSize
TimeBar::sizeHint() const
{
  return minimumSizeHint();
}


void TimeBar::paintEvent(QPaintEvent * /* event */)
{
  TRACE_ENTER("TimeBar::paintEvent");
  QStylePainter painter(this);
  //painter.setPen(pen);
  //painter.setBrush(brush);

  const int border_size = 1;

  // Draw background
  painter.fillRect(0, 0, width() - 1, height() - 1, QColor("white"));
  painter.setPen(QColor("black"));
  painter.drawRect(0, 0, width() - 1, height() - 1);


  QStyleOptionFrame option;
  option.initFrom(this);
  option.features = QStyleOptionFrame::Flat;
  option.frameShape = QFrame::Panel;
  option.lineWidth = 2;
  option.midLineWidth = 0;

  painter.drawPrimitive(QStyle::PE_Frame, option);  

  
  // Bar
  int bar_width = 0;
  if (bar_max_value > 0)
    {
      bar_width = (bar_value * (width() - 2 * border_size - 1)) / bar_max_value;
    }

  // Secondary bar
  int sbar_width = 0;
  if (secondary_bar_max_value >  0)
    {
      sbar_width = (secondary_bar_value * (width() - 2 * border_size - 1)) / secondary_bar_max_value;
    }

  int bar_height = height() - 2 * border_size - 1;

  if (sbar_width > 0)
    {
      // Overlap

      // We should assert() because this is not supported
      // but there are some weird boundary cases
      // in which this still happens.. need to check
      // this out some time.
      // assert(secondary_bar_color == COLOR_ID_INACTIVE);
      ColorId overlap_color;
      switch (bar_color)
        {
        case COLOR_ID_ACTIVE:
          overlap_color = COLOR_ID_INACTIVE_OVER_ACTIVE;
          break;
        case COLOR_ID_OVERDUE:
          overlap_color = COLOR_ID_INACTIVE_OVER_OVERDUE;
          break;
        default:
          // We could abort() because this is not supported
          // but there are some weird boundary cases
          // in which this still happens.. need to check
          // this out some time.
          overlap_color = COLOR_ID_INACTIVE_OVER_ACTIVE;
        }

      if (sbar_width >= bar_width)
        {
          if (bar_width)
            {
              painter.fillRect(border_size, border_size,
                               bar_width, bar_height,
                               bar_colors[overlap_color]);
            }
          if (sbar_width > bar_width)
            {
              painter.fillRect(border_size + bar_width, border_size,
                               sbar_width - bar_width, bar_height,
                               bar_colors[secondary_bar_color]);
            }
        }
      else
        {
          if (sbar_width)
            {
              painter.fillRect(border_size, border_size,
                               sbar_width, bar_height,
                               bar_colors[overlap_color]);
            }
          painter.fillRect(border_size + sbar_width, border_size,
                           bar_width - sbar_width, bar_height,
                           bar_colors[bar_color]);
        }
    }
  else
    {
      // No overlap
      painter.fillRect(border_size, border_size,
                       bar_width, bar_height,
                       bar_colors[bar_color]);
    }


  QString text = QString::fromStdString(bar_text);
  
  int text_width = painter.fontMetrics().width(text);
  int text_height = painter.fontMetrics().height();

  int text_x;
  if (bar_text_align > 0)
    text_x = std::max(width() - text_width - MARGINX, 0);
  else if (bar_text_align < 0)
    text_x = MARGINX;
  else
    text_x = (width() - text_width) / 2;
  
  int text_y = (height() + text_height ) / 2 - MARGINY;

  TRACE_MSG("x = " << text_x << "y = " << text_y);
  
  int left_width = (bar_width > sbar_width) ? bar_width : sbar_width;
  left_width += border_size;

  QRegion left_rect(0, 0, left_width, height());
  QRegion right_rect(left_width, 0, width() - left_width, height());

  //painter.setClipping(true);
  //painter.setClipRegion(left_rect);

  painter.setPen(QColor("black"));
  painter.drawText(text_x, text_y, text);

  //painter.setClipRegion(right_rect);
  
  //painter.setPen(QColor("white"));
  //painter.drawText(text_x, text_y, text);

  TRACE_MSG("width = " << text_width << "height = " << text_height);
  TRACE_EXIT();
}
