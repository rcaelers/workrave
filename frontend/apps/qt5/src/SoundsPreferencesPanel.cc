// SoundsPreferencesPanel.cc --- base class for the break windows
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

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include "SoundsPreferencesPanel.hh"

#include <QtGui>
#include <QStyle>

#include "debug.hh"
#include "nls.h"

#include "TimerPreferencesPanel.hh"

#include "Locale.hh"

#include "ICore.hh"
#include "UiUtil.hh"
#include "CoreFactory.hh"

using namespace workrave;

SoundsPreferencesPanel::SoundsPreferencesPanel()
  : QGroupBox(_("Sounds"))
{
  TRACE_ENTER("SoundsPreferencesPanel::SoundsPreferencesPanel");

  connector = new DataConnector();

  QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);

  layout->addStretch();
  
  TRACE_EXIT();
}


SoundsPreferencesPanel::~SoundsPreferencesPanel()
{
}

