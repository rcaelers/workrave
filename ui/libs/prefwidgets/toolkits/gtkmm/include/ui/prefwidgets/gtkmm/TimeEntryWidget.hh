// Copyright (C) 2022 Rob Caelers <robc@krandor.nl>
// All rights reserved.timeentry
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

#ifndef WORKRAVE_UI_PREFWIDGETS_GTKMM_TIMEENTRYWIDGET_HH
#define WORKRAVE_UI_PREFWIDGETS_GTKMM_TIMEENTRYWIDGET_HH

#include <memory>

#include <gtkmm.h>

#include "ui/prefwidgets/Time.hh"
#include "ContainerWidget.hh"
#include "commonui/GtkCompat.hh"

namespace ui::prefwidgets::gtkmm
{
  class TimeEntryWidget : public Widget
  {
  public:
    TimeEntryWidget(std::shared_ptr<ui::prefwidgets::Time> def,
                    std::shared_ptr<ContainerWidget> container,
                    BuilderRegistry *registry);
    ~TimeEntryWidget() override = default;

  private:
    void init_ui(std::shared_ptr<ContainerWidget> container);
    void on_changed();
    void on_value_changed();

    int32_t get_value();
    void set_value(int32_t time);

  private:
    std::shared_ptr<ui::prefwidgets::Time> def;

    GtkCompat::Box *box{nullptr};

    Gtk::SpinButton *hrs{nullptr};
    Gtk::SpinButton *mins{nullptr};
    Gtk::SpinButton *secs{nullptr};

    Glib::RefPtr<Gtk::Adjustment> hours_adjustment{Gtk::Adjustment::create(0, 0, 23)};
    Glib::RefPtr<Gtk::Adjustment> mins_adjustment{Gtk::Adjustment::create(0, 0, 59)};
    Glib::RefPtr<Gtk::Adjustment> secs_adjustment{Gtk::Adjustment::create(0, 0, 59)};
  };
} // namespace ui::prefwidgets::gtkmm

#endif // WORKRAVE_UI_PREFWIDGETS_GTKMM_TIMEENTRYWIDGET_HH
