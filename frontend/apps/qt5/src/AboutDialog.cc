// AboutDialog.cc --- base class for the break windows
//
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
#include "config.h"
#endif

#include "AboutDialog.hh"

#include <QtGui>
#include <QStyle>

#include <boost/format.hpp>

#include "nls.h"
#include "debug.hh"

#include "utils/AssetPath.hh"
#include "ICore.hh"
#include "UiUtil.hh"
#include "Ui.hh"
#include "credits.h"

using namespace workrave;
using namespace workrave::utils;
using namespace workrave::ui;

AboutDialog::AboutDialog()
  : QDialog()
{
  TRACE_ENTER("AboutDialog::AboutDialog");

  setWindowTitle(_("About Workrave"));
  // setWindowIcon(...);

  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);


  QGridLayout *layout = new QGridLayout(this);
  layout->setSizeConstraint(QLayout::SetFixedSize);

  std::string description = boost::str(boost::format(_("<h3>Workrave %s %s</h3>"
                                                       "<br/>"
                                                       "%s<br/>"
                                                       "<br/>"
                                                       "%s<br/>"
                                                       "<br/>"
                                                       "%s<br/>")) %
#ifdef GIT_VERSION
                                       (PACKAGE_VERSION  "\n(" GIT_VERSION ")") %
#else
                                       (PACKAGE_VERSION "") %
#endif
                                        _("This program assists in the prevention and recovery"
                                          " of Repetitive Strain Injury (RSI).") %
                                        workrave_copyright %
                                        workrave_authors);
                                       
                                                       
  QLabel *copyRightLabel = new QLabel(QString::fromStdString(description));
  copyRightLabel->setWordWrap(true);
  copyRightLabel->setOpenExternalLinks(true);
  copyRightLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
  
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
  QPushButton *closeButton = buttonBox->button(QDialogButtonBox::Close);

  buttonBox->addButton(closeButton, QDialogButtonBox::ButtonRole(QDialogButtonBox::RejectRole | QDialogButtonBox::AcceptRole));
  connect(buttonBox , SIGNAL(rejected()), this, SLOT(reject()));

  std::string logo_file = AssetPath::complete_directory("workrave.png", AssetPath::SEARCH_PATH_IMAGES);
  QPixmap pixmap(logo_file.c_str());
  QLabel *logoLabel = new QLabel;
  logoLabel->setPixmap(pixmap);

  layout->addWidget(logoLabel , 0, 0, 1, 1);
  layout->addWidget(copyRightLabel, 0, 1, 4, 4);
  layout->addWidget(buttonBox, 4, 0, 1, 5);

  TRACE_EXIT();
}


//! Destructor.
AboutDialog::~AboutDialog()
{
  TRACE_ENTER("AboutDialog::~AboutDialog");
  TRACE_EXIT();
}
