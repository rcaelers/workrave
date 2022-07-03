// Copyright (C) 2020 Rob Caelers <robc@krandor.nl>
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

#include "IconListNotebook.hh"

#include <QtGui>
#include <QtWidgets>
#include <QtGlobal>

#include <iostream>

IconListNotebook::IconListNotebook(QWidget *parent)
  : QWidget(parent)
{
  content = new QStackedWidget;
  content->setFrameShape(QFrame::StyledPanel);

  button_group = std::make_unique<QButtonGroup>();

  connect(button_group.get(), &QButtonGroup::buttonClicked, [=](QAbstractButton *btn) { on_button_selected(btn); });

  button_layout = new QVBoxLayout;
  button_layout->setSpacing(0);

  auto *button_stretch_layout = new QVBoxLayout();
  button_stretch_layout->setContentsMargins(2, 2, 2, 2);
  button_stretch_layout->setSpacing(2);
  button_stretch_layout->addLayout(button_layout);
  button_stretch_layout->addStretch();

  auto *button_box = new QFrame;
  button_box->setFrameShape(QFrame::StyledPanel);
  button_box->setLayout(button_stretch_layout);

  layout = new QHBoxLayout;
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(button_box);
  layout->addWidget(content, 1);

  setLayout(layout);
}

auto
IconListNotebook::sizeHint() const -> QSize
{
  int max_x{0};
  int max_y{0};
  for (QAbstractButton *button: button_group->buttons())
    {
      max_x = qMax(max_x, button->sizeHint().width());
      max_y = qMax(max_y, button->sizeHint().height());
    }
  return {max_x, max_y};
}

void
IconListNotebook::add_page(QWidget *page, const QIcon &icon, const QString &title)
{
  int index = content->count();

  page->setParent(content);
  content->addWidget(page);

  auto *button = new QToolButton();
  button->setCheckable(true);
  if (index == 0)
    {
      button->setChecked(true);
    }
  button->setIconSize(icon.availableSizes().first());
  button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  button->setIcon(icon);
  button->setText(title);
  button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  button_group->addButton(button, index);
  button_layout->addWidget(button);
}

void
IconListNotebook::on_button_selected(QAbstractButton *button)
{
  auto index = button_group->id(button);
  if (index < 0 || index >= content->count())
    {
      index = 0;
    }
  if (index != content->currentIndex())
    {
      content->setCurrentIndex(index);
      button_group->button(index)->setChecked(true);
    }
}
