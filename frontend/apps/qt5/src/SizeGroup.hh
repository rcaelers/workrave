// SizeGroup.hh
//
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
  SizeGroup(Qt::Orientation orientation, QObject* parent = 0);
  ~SizeGroup();

  void addWidget(QWidget* widget);

private:
  bool eventFilter(QObject*, QEvent* event);
  void update();
  
private:
  Qt::Orientation orientation;
  QTimer* timer;
  QList<QWidget*> widgets;
};

#endif // SIZEGROUP_HH
