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

#ifndef WORKRAVE_UI_PREFWIDGETS_QT_TIMEENTRYWIDGET_HH
#define WORKRAVE_UI_PREFWIDGETS_QT_TIMEENTRYWIDGET_HH

#include <memory>

#include "ui/prefwidgets/Time.hh"
#include "ContainerWidget.hh"

namespace ui::prefwidgets::qt
{
  class TimeEntryWidget : public Widget
  {
    Q_OBJECT

  public:
    TimeEntryWidget(std::shared_ptr<ui::prefwidgets::Time> def,
                    std::shared_ptr<ContainerWidget> container,
                    BuilderRegistry *registry);
    ~TimeEntryWidget() override = default;

  private:
    void init_ui(std::shared_ptr<ContainerWidget> container);
    void on_changed();

    int32_t get_value();
    void set_value(int32_t time);

  private:
    std::shared_ptr<ui::prefwidgets::Time> def;

    QWidget *widget{};
    QSpinBox *hrs{nullptr};
    QSpinBox *mins{nullptr};
    QSpinBox *secs{nullptr};
  };
} // namespace ui::prefwidgets::qt

#endif // WORKRAVE_UI_PREFWIDGETS_QT_TIMEENTRYWIDGET_HH
