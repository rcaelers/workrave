// Frame.hh
//
// Copyright (C) 2001 -2013 Rob Caelers <robc@krandor.nl>
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

#ifndef FRAME_HH
#define FRAME_HH

#include <memory>
#include <boost/signals2.hpp>

#include <QtWidgets/qwidget.h>

class Frame : public QWidget
{
  Q_OBJECT

public:
  explicit Frame(QWidget* parent = 0);
  ~Frame() override;

  enum Style
    {
      STYLE_SOLID,
      STYLE_BREAK_WINDOW
    };

  boost::signals2::signal<void(bool)> &signal_flash();

  void set_frame_width(int frame, int border);
  void set_frame_style(Style style);
  void set_frame_color(const QColor &color);
  void set_frame_flashing(int delay);
  void set_frame_visible(bool visible);

public slots:
  void on_timer();

protected:
  void paintEvent(QPaintEvent *) override;

private:
  QRect get_frame_rect() const;

  std::shared_ptr<QTimer> heartbeat_timer;


  //! Frame border width
  int frame_width;

  //! Border width
  int border_width;

  //! Color of the frame.
  QColor frame_color;

  //! Style of the frame.
  Style frame_style;

  //! Visible;
  bool frame_visible;

  //! Flash delay;
  int flash_delay;

  //! Flash signal.
  boost::signals2::signal<void(bool)> flash_signal;
};

#endif // FRAME_HH
