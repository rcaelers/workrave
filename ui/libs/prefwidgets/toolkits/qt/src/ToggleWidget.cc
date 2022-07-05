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

#include "ToggleWidget.hh"

using namespace ui::prefwidgets::qt;

ToggleWidget::ToggleWidget(std::shared_ptr<ui::prefwidgets::Toggle> def,
                           std::shared_ptr<ContainerWidget> container,
                           BuilderRegistry *registry)
  : Widget(registry)
  , def(def)
{
  init_ui(container);
}

void
ToggleWidget::init_ui(std::shared_ptr<ContainerWidget> container)
{
  auto label = def->get_label();
  widget = new QCheckBox();
  if (!label.empty())
    {
      widget->setText(QString::fromStdString(label));
    }
  add_to_size_groups(def, widget);

  bool idx = def->get_value();
  widget->setCheckState(idx ? Qt::Checked : Qt::Unchecked);

  def->init([this](bool on) { widget->setEnabled(on); });

  connect(widget, &QCheckBox::stateChanged, [this]() {
    bool on = widget->checkState() == Qt::Checked;
    def->set_value(on);
  });

  container->add_widget(widget);
}
