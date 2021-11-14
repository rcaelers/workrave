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

#include "ui/UiTypes.hh"

#include <QWidget>

class TimeBar : public QWidget
{
  Q_OBJECT

public:
  explicit TimeBar(QWidget *parent = nullptr);

  auto minimumSizeHint() const -> QSize override;
  auto sizeHint() const -> QSize override;

  void set_progress(int value, int max_value);
  void set_secondary_progress(int value, int max_value);

  void set_bar_color(TimerColorId color);
  void set_secondary_bar_color(TimerColorId color);

  void set_text(const QString &text);
  void set_text_alignment(int align);

  void update();

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  static std::map<TimerColorId, QColor> bar_colors;

  TimerColorId bar_color{TimerColorId::Active};
  TimerColorId secondary_bar_color{TimerColorId::Active};
  int bar_value{0};
  int bar_max_value{0};
  int secondary_bar_value{0};
  int secondary_bar_max_value{0};
  QString bar_text;
  int bar_text_align{0};
};

#endif // TIMEBAR_HH
