// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#include "ExercisesDialog.hh"

#include <QtGui>
#include <QStyle>

#include "debug.hh"

#include "ExercisesPanel.hh"
#include "UiUtil.hh"

ExercisesDialog::ExercisesDialog(std::shared_ptr<IApplicationContext> app)
{
  TRACE_ENTRY();
  setWindowTitle(tr("Exercises"));

  auto *layout = new QVBoxLayout();
  layout->setContentsMargins(1, 1, 1, 1);
  setLayout(layout);

  auto *button_box = new QDialogButtonBox(QDialogButtonBox::Close);
  QPushButton *close_button = button_box->button(QDialogButtonBox::Close);
  close_button->setIcon(QIcon::fromTheme("window-close", UiUtil::create_icon("window-close-symbolic.svg")));
  close_button->setAutoDefault(true);
  close_button->setDefault(true);

  panel = new ExercisesPanel(app, button_box);
  panel->set_exercise_count(0);
  panel->signal_stop().connect([this] { accept(); });

  layout->addWidget(panel);
  layout->addWidget(button_box);

  connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::accept);
}
