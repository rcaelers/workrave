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

#include "TimeEntryWidget.hh"

using namespace ui::prefwidgets::qt;

TimeEntryWidget::TimeEntryWidget(std::shared_ptr<ui::prefwidgets::Time> def,
                                 std::shared_ptr<ContainerWidget> container,
                                 BuilderRegistry *registry)
  : Widget(registry)
  , def(def)
{
  init_ui(container);
}

void
TimeEntryWidget::set_value(int32_t t)
{
  hrs->setValue(t / (60 * 60));
  mins->setValue((t / 60) % 60);
  secs->setValue(t % 60);
}

int32_t
TimeEntryWidget::get_value()
{
  int s = secs->value();
  int h = hrs->value();
  int m = mins->value();
  return (h * 60 * 60) + (m * 60) + s;
}

void
TimeEntryWidget::init_ui(std::shared_ptr<ContainerWidget> container)
{
  widget = new QWidget;
  auto *layout = new QHBoxLayout;
  widget->setLayout(layout);

  add_to_size_groups(def, widget);

  secs = new QSpinBox;
  secs->setWrapping(true);
  secs->setMinimum(0);
  secs->setMaximum(59);

  hrs = new QSpinBox;
  hrs->setWrapping(true);
  hrs->setMinimum(0);
  hrs->setMaximum(59);

  mins = new QSpinBox;
  mins->setWrapping(true);
  mins->setMinimum(0);
  mins->setMaximum(59);

  void (QSpinBox::*signal)(int) = &QSpinBox::valueChanged;
  connect(secs, signal, this, &TimeEntryWidget::on_changed);
  connect(hrs, signal, this, &TimeEntryWidget::on_changed);
  connect(mins, signal, this, &TimeEntryWidget::on_changed);

  auto *semi1 = new QLabel(":");
  auto *semi2 = new QLabel(":");

  layout->addWidget(hrs, 1);
  layout->addWidget(semi1);
  layout->addWidget(mins, 1);
  layout->addWidget(semi2);
  layout->addWidget(secs, 1);

  layout->setContentsMargins(1, 1, 1, 1);
  layout->setSpacing(2);

  widget->setEnabled(def->get_sensitive());
  set_value(def->get_value());

  def->init([this](int v) {
    set_value(v);
    widget->setEnabled(def->get_sensitive());
  });

  container->add_label(def->get_label(), widget);
}

void
TimeEntryWidget::on_changed()
{
  int v = get_value();
  def->set_value(v);
}
