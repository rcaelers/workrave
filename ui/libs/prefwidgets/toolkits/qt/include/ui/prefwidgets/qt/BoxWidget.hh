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

#ifndef WORKRAVE_UI_PREFWIDGETS_QT_BOXWIDGET_HH
#define WORKRAVE_UI_PREFWIDGETS_QT_BOXWIDGET_HH

#include <memory>
#include <string>
#include <list>

#include <QBoxLayout>

#include "ui/prefwidgets/Box.hh"
#include "ContainerWidget.hh"

namespace ui::prefwidgets::qt
{
  class BoxWidget : public ContainerWidget
  {
    Q_OBJECT

  public:
    BoxWidget(std::shared_ptr<ui::prefwidgets::Box> def, std::shared_ptr<ContainerWidget> container, BuilderRegistry *registry);
    explicit BoxWidget(QLayout *layout);
    ~BoxWidget() override = default;

    void add(std::shared_ptr<Widget> widget) override;
    void add_label(const std::string &label, QWidget *widget, bool expand = false, bool fill = false) override;
    void add_widget(QWidget *widget, bool expand = false, bool fill = false) override;

  private:
    std::shared_ptr<ui::prefwidgets::Box> def;
    QWidget *widget{};
    QLayout *layout{};
    std::list<std::shared_ptr<Widget>> children;
  };
} // namespace ui::prefwidgets::qt

#endif // WORKRAVE_UI_PREFWIDGETS_QT_BOXWIDGET_HH
