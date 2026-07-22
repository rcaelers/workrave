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

#include "TimeBar.hh"

#include <QStylePainter>
#include <algorithm>

#include "UiUtil.hh"
#include "debug.hh"

const int MARGINX = 4;
const int MARGINY = 2;
const int MINIMUM_HEIGHT = 20;
const int BORDER_SIZE = 1;

std::map<TimerColorId, QColor> TimeBar::bar_colors{
  {TimerColorId::Active, QColor("lightblue")},
  {TimerColorId::Inactive, QColor("lightgreen")},
  {TimerColorId::Overdue, QColor("orange")},
  {TimerColorId::InactiveOverActive, QColor("#00d4b2")},
  {TimerColorId::InactiveOverOverdue, QColor("lightgreen")},
  {TimerColorId::Bg, QColor("#777777")},
};

TimeBar::TimeBar(QWidget *parent)
  : QWidget(parent)
{
  setAttribute(Qt::WA_OpaquePaintEvent);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void
TimeBar::set_progress(int value, int max_value)
{
  value = std::min(value, max_value);

  bar_value = value;
  bar_max_value = max_value;
}

void
TimeBar::set_secondary_progress(int value, int max_value)
{
  value = std::min(value, max_value);

  secondary_bar_value = value;
  secondary_bar_max_value = max_value;
}

void
TimeBar::set_text(const QString &text)
{
  bar_text = text;
}

void
TimeBar::set_text_alignment(int align)
{
  bar_text_align = align;
}

void
TimeBar::set_bar_color(TimerColorId color)
{
  bar_color = color;
}

void
TimeBar::set_secondary_bar_color(TimerColorId color)
{
  secondary_bar_color = color;
}

void
TimeBar::update()
{
  QWidget::update();
}

auto
TimeBar::minimumSizeHint() const -> QSize
{
  int width = fontMetrics().horizontalAdvance(bar_text);
  int height = fontMetrics().height();

  QString full_text = UiUtil::time_to_string(-(59 + (59 * 60) + (9 * 60 * 60)));
  int full_width = fontMetrics().horizontalAdvance(full_text);

  width = std::max(full_width, width);

  width = width + 2 * MARGINX;
  height = std::max(height + (2 * MARGINY), MINIMUM_HEIGHT);

  return QSize(width, height);
}

auto
TimeBar::sizeHint() const -> QSize
{
  return minimumSizeHint();
}

void
TimeBar::paintEvent(QPaintEvent * /* event */)
{
  TRACE_ENTRY();
  QStylePainter painter(this);

  const QRect frame_rect = rect().adjusted(0, 0, -1, -1);
  const QRect text_rect = frame_rect.adjusted(MARGINX + BORDER_SIZE, BORDER_SIZE, -(MARGINX + BORDER_SIZE), -BORDER_SIZE);
  const int bar_height = std::max(frame_rect.height(), 0);

  // Draw background
  painter.fillRect(frame_rect, QColor("white"));

  // Bar
  int bar_width = 0;
  if (bar_max_value > 0 && frame_rect.width() > 0)
    {
      bar_width = (bar_value * frame_rect.width()) / bar_max_value;
    }

  // Secondary bar
  int sbar_width = 0;
  if (secondary_bar_max_value > 0 && frame_rect.width() > 0)
    {
      sbar_width = (secondary_bar_value * frame_rect.width()) / secondary_bar_max_value;
    }

  if (sbar_width > 0)
    {
      // Overlap

      // We should assert() because this is not supported
      // but there are some weird boundary cases
      // in which this still happens.. need to check
      // this out some time.
      // assert(secondary_bar_color == TimerColorId::Inactive);
      TimerColorId overlap_color{TimerColorId::InactiveOverActive};
      switch (bar_color)
        {
        case TimerColorId::Active:
          overlap_color = TimerColorId::InactiveOverActive;
          break;
        case TimerColorId::Overdue:
          overlap_color = TimerColorId::InactiveOverOverdue;
          break;
        default:
          // We could abort() because this is not supported
          // but there are some weird boundary cases
          // in which this still happens.. need to check
          // this out some time.
          overlap_color = TimerColorId::InactiveOverActive;
        }

      if (sbar_width >= bar_width)
        {
          if (bar_width != 0)
            {
              painter.fillRect(frame_rect.x(), frame_rect.y(), bar_width, bar_height, bar_colors[overlap_color]);
            }
          if (sbar_width > bar_width)
            {
              painter.fillRect(frame_rect.x() + bar_width,
                               frame_rect.y(),
                               sbar_width - bar_width,
                               bar_height,
                               bar_colors[secondary_bar_color]);
            }
        }
      else
        {
          if (sbar_width != 0)
            {
              painter.fillRect(frame_rect.x(), frame_rect.y(), sbar_width, bar_height, bar_colors[overlap_color]);
            }
          painter.fillRect(frame_rect.x() + sbar_width,
                           frame_rect.y(),
                           bar_width - sbar_width,
                           bar_height,
                           bar_colors[bar_color]);
        }
    }
  else
    {
      // No overlap
      painter.fillRect(frame_rect.x(), frame_rect.y(), bar_width, bar_height, bar_colors[bar_color]);
    }

  painter.setPen(QColor("#8f8f8f"));
  painter.drawRect(frame_rect);

  Qt::Alignment alignment = Qt::AlignVCenter;
  if (bar_text_align > 0)
    {
      alignment |= Qt::AlignRight;
    }
  else if (bar_text_align < 0)
    {
      alignment |= Qt::AlignLeft;
    }
  else
    {
      alignment |= Qt::AlignHCenter;
    }

  painter.setPen(QColor("black"));
  painter.drawText(text_rect, alignment, bar_text);

  int text_width = painter.fontMetrics().horizontalAdvance(bar_text);
  int text_height = painter.fontMetrics().height();
  TRACE_MSG("width = {} height = {}", text_width, text_height);
}
