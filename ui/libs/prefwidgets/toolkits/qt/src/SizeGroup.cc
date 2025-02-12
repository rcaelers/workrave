// Copyright (C) 2022 Rob Caelers <robc@krandor.nl>
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

#include <QWidget>

#include "SizeGroup.hh"

#include "ui/prefwidgets/Widget.hh"
#include "ui/prefwidgets/Widgets.hh"

using namespace ui::prefwidgets::qt;

SizeGroup::SizeGroup(ui::prefwidgets::Orientation orientation)
  : orientation(orientation)
{
  timer = new QTimer(this);
  timer->setSingleShot(true);
  timer->setInterval(0);

  QObject::connect(timer, &QTimer::timeout, this, &SizeGroup::update);
}

SizeGroup::SizeGroup(std::shared_ptr<ui::prefwidgets::SizeGroup> def)
  : SizeGroup(def->get_orientation())
{
}

void
SizeGroup::add_widget(QWidget *widget)
{
  widgets.append(widget);
  widget->installEventFilter(this);

  timer->start();
}

auto
SizeGroup::eventFilter(QObject *o, QEvent *event) -> bool
{
  if (event->type() == QEvent::Resize)
    {
      timer->start();
    }
  return false;
}

void
SizeGroup::update()
{
  int width = 0;
  int height = 0;

  Q_FOREACH (QWidget *widget, widgets)
    {
      width = qMax(widget->sizeHint().width(), width);
      height = qMax(widget->sizeHint().height(), height);
    }

  Q_FOREACH (QWidget *widget, widgets)
    {
      if (orientation == Orientation::Horizontal)
        {
          widget->setMinimumWidth(width);
        }
      if (orientation == Orientation::Vertical)
        {
          widget->setMinimumHeight(height);
        }
      widget->updateGeometry();
    }
}
