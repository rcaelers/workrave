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

#include "DebugDialog.hh"

#include <QStyle>
#include <QtGui>

#include "core/ICore.hh"
#include "utils/AssetPath.hh"

#include "Ui.hh"
#include "UiUtil.hh"
#include "qformat.hh"

using namespace workrave;
using namespace workrave::utils;

DebugDialog::DebugDialog()
  : QDialog()
{
  setWindowTitle(tr("Debug Workrave"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  // QGridLayout *layout = new QGridLayout(this);
  // layout->setSizeConstraint(QLayout::SetFixedSize);
}
