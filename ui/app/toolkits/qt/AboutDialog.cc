// Copyright (C) 2001 - 2015 Rob Caelers & Raymond Penners
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

#include "AboutDialog.hh"

#include <QStyle>
#include <QtGui>

#include "commonui/credits.h"
#include "qformat.hh"
#include "UiUtil.hh"

using namespace workrave;
using namespace workrave::utils;

AboutDialog::AboutDialog()
{
  setWindowTitle(tr("About Workrave"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  auto *layout = new QGridLayout(this);
  layout->setSizeConstraint(QLayout::SetFixedSize);

  QString description = qstr(qformat(tr("<h3>Workrave %s</h3>"
                                        "<br/>%s<br/>"))
                             %
#if defined(WORKRAVE_GIT_VERSION)
                             (WORKRAVE_VERSION "(" WORKRAVE_GIT_VERSION ")") %
#else
                             (WORKRAVE_VERSION "") %
#endif
                             tr("This program assists in the prevention and recovery"
                                " of Repetitive Strain Injury (RSI)."));

  auto *description_label = new QLabel(description);
  description_label->setWordWrap(true);
  description_label->setOpenExternalLinks(true);
  description_label->setTextInteractionFlags(Qt::TextBrowserInteraction);

  auto *copyright_label = new QLabel(workrave_copyright);
  copyright_label->setWordWrap(false);
  copyright_label->setOpenExternalLinks(false);
  copyright_label->setTextInteractionFlags(Qt::TextBrowserInteraction);

  auto *vbox = new QVBoxLayout;
  vbox->addWidget(description_label);
  vbox->addWidget(copyright_label);

  auto *button_box = new QDialogButtonBox(QDialogButtonBox::Close);
  QPushButton *closeButton = button_box->button(QDialogButtonBox::Close);

  button_box->addButton(closeButton, QDialogButtonBox::ButtonRole(QDialogButtonBox::RejectRole | QDialogButtonBox::AcceptRole));
  connect(button_box, SIGNAL(rejected()), this, SLOT(reject()));

  QPixmap pixmap = UiUtil::create_pixmap("workrave-sheep.svg", 150);
  auto *logoLabel = new QLabel;
  logoLabel->setPixmap(pixmap);

  layout->addWidget(logoLabel, 0, 0, 5, 1, Qt::AlignCenter);
  layout->addLayout(vbox, 0, 1, 4, 4);
  layout->addWidget(button_box, 4, 0, 1, 5);

  // TODO: add authors/translators.
}
