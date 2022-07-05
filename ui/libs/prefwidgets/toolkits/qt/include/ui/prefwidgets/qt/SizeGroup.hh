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

#ifndef WORKRAVE_UI_PREFWIDGETS_QT_SIZEGROUP_HH
#define WORKRAVE_UI_PREFWIDGETS_QT_SIZEGROUP_HH

#include <QtGui>
#include <QtWidgets>

#include "ui/prefwidgets/SizeGroup.hh"

namespace ui::prefwidgets::qt
{
  class SizeGroup : public QObject
  {
    Q_OBJECT

  public:
    explicit SizeGroup(std::shared_ptr<ui::prefwidgets::SizeGroup> def);
    ~SizeGroup() override = default;

    void add_widget(QWidget *widget);

  private:
    auto eventFilter(QObject * /*watched*/, QEvent *event) -> bool override;
    void update();

  private:
    std::shared_ptr<ui::prefwidgets::SizeGroup> def;
    ui::prefwidgets::Orientation orientation;
    QTimer *timer{nullptr};
    QList<QWidget *> widgets;
  };
} // namespace ui::prefwidgets::qt

#endif // WORKRAVE_UI_PREFWIDGETS_QT_SIZEGROUP_HH
