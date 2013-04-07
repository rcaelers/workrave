#include "TimeBar.hh"

#include <QStylePainter>
#include <QStyleOptionProgressBar>

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
  QWidget(parent)
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
    return QSize(100, 100);
}

QSize
TimeBar::sizeHint() const
{
    return QSize(400, 200);
}


void TimeBar::paintEvent(QPaintEvent * /* event */)
{
  QStylePainter painter(this);
  //painter.setPen(pen);
  //painter.setBrush(brush);

  const int border_size = 1;

  // Draw background
  painter.fillRect(0, 0, width() - 1, height() - 1, QColor("grey"));
  
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
  if (width() - text_width - MARGINX > 0)
    {
      text_x = (width() - text_width) / 2;
    }
  else
    {
      text_x = MARGINX;
    }
  int text_y = (height() - text_height) / 2;

  int left_width = (bar_width > sbar_width) ? bar_width : sbar_width;
  left_width += border_size;

  QRegion left_rect(0, 0, left_width, height());
  QRegion right_rect(left_width, 0, width() - left_width, height());

  painter.setClipping(true);
  painter.setClipRegion(left_rect);

  painter.setPen(QColor("white"));
  painter.drawText(text_x, text_y, text);

  painter.setClipRegion(right_rect);
  
  painter.setPen(QColor("black"));
  painter.drawText(text_x, text_y, text);
}
