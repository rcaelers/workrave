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

#ifndef WORKRAVE_UI_PREFWIDGETS_QT_FRAMEWIDGET_HH
#define WORKRAVE_UI_PREFWIDGETS_QT_FRAMEWIDGET_HH

#include <memory>

#include "ui/prefwidgets/Widgets.hh"

#include "Widget.hh"
#include "ContainerWidget.hh"

namespace ui::prefwidgets::qt
{
  class FrameWidget : public ContainerWidget
  {
    Q_OBJECT

  public:
    FrameWidget(std::shared_ptr<ui::prefwidgets::Frame> def,
                std::shared_ptr<ContainerWidget> container,
                BuilderRegistry *registry);
    ~FrameWidget() override = default;

    void add(std::shared_ptr<Widget> w) override;
    void add_label(const std::string &label, QWidget *widget, bool expand = false, bool fill = false) override;
    void add_widget(QWidget *widget, bool expand = false, bool fill = false) override;

  private:
    std::shared_ptr<ui::prefwidgets::Frame> def;
    QGroupBox *panel{};
    QVBoxLayout *layout{};
    std::list<std::shared_ptr<Widget>> children;
  };
} // namespace ui::prefwidgets::qt

#endif // WORKRAVE_UI_PREFWIDGETS_QT_FRAMEWIDGET_HH
