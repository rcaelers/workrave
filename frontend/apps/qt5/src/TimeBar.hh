// TimeBar.hh
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

#ifndef TIMEBAR_HH
#define TIMEBAR_HH

#include "ITimeBar.hh"

#include <QWidget>

// namespace Ui {
//   class TimeBar;
//}

class TimeBar : public QWidget, public ITimeBar
{
  Q_OBJECT

public:
  explicit TimeBar(QWidget *parent = 0);
  ~TimeBar() override;

  QSize minimumSizeHint() const override;
  QSize sizeHint() const override;

  void set_progress(int value, int max_value) override;
  void set_secondary_progress(int value, int max_value) override;

  void set_text(std::string text) override;
  void set_text_alignment(int align) override;

  void update() override;
  void set_bar_color(ColorId color) override;
  void set_secondary_bar_color(ColorId color) override;

protected:
  void paintEvent(QPaintEvent *event) override;

private:

private:
  static QColor bar_colors[COLOR_ID_SIZEOF];

  //! Color of the time-bar.
  ColorId bar_color;

  //! Color of the time-bar.
  ColorId secondary_bar_color;

  //! Color of the text.
  //! The current value.
  int bar_value;

  //! The maximum value.
  int bar_max_value;

  //! The current value.
  int secondary_bar_value;

  //! The maximum value.
  int secondary_bar_max_value;

  //! Text to show;
  std::string bar_text;

  //! Text alignment
  int bar_text_align;
};

#endif // TIMEBAR_HH
