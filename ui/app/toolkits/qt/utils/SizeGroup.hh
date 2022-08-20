// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef SIZEGROUP_HH
#define SIZEGROUP_HH

#include <QtGui>
#include <QtWidgets>

class SizeGroup : public QObject
{
  Q_OBJECT

public:
  explicit SizeGroup(Qt::Orientations orientation, QObject *parent = nullptr);

  void add_widget(QWidget *widget);

private:
  auto eventFilter(QObject * /*watched*/, QEvent *event) -> bool override;
  void update();

private:
  Qt::Orientations orientation;
  QTimer *timer{nullptr};
  QList<QWidget *> widgets;
};

#endif // SIZEGROUP_HH
