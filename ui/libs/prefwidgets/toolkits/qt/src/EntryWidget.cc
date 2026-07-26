// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
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

#include "EntryWidget.hh"

#include <QSignalBlocker>

using namespace ui::prefwidgets::qt;

EntryWidget::EntryWidget(std::shared_ptr<ui::prefwidgets::Entry> def,
                         std::shared_ptr<ContainerWidget> container,
                         BuilderRegistry *registry)
  : Widget(registry)
  , def(def)
{
  init_ui(container);
}

void
EntryWidget::init_ui(std::shared_ptr<ContainerWidget> container)
{
  widget = new QLineEdit();
  add_to_size_groups(def, widget);

  widget->setMinimumWidth(widget->fontMetrics().averageCharWidth() * 60);
  widget->setText(QString::fromStdString(def->get_content()));
  widget->setEnabled(def->get_sensitive());

  def->init([this](const std::string &txt) {
    QSignalBlocker blocker(widget);
    widget->setText(QString::fromStdString(txt));
    widget->setEnabled(def->get_sensitive());
  });

  QObject::connect(widget, &QLineEdit::textChanged, [this](const QString &txt) { def->set_value(txt.toStdString()); });

  auto label = def->get_label();
  if (!label.empty())
    {
      container->add_label(label, widget);
    }
  else
    {
      container->add_widget(widget);
    }
}
