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

#ifndef GENERALUIPREFERENCESPANEL_HH
#define GENERALUIPREFERENCESPANEL_HH

#include <QtGui>
#include <QtWidgets>

#include "SizeGroup.hh"
#include "DataConnector.hh"

class GeneralUiPreferencesPanel : public QWidget
{
  Q_OBJECT

public:
  GeneralUiPreferencesPanel();
  ~GeneralUiPreferencesPanel();

private:
  void on_block_changed();

#ifdef PLATFORM_OS_WIN32
  void on_autostart_toggled();
#endif

private:
  DataConnector::Ptr connector;

  QComboBox *block_button;
  QComboBox *languages_combo;
  QStandardItemModel *model;

#if defined(PLATFORM_OS_WIN32)
  QCheckBox *autostart_cb;
#endif
};

#endif // GENERALUIPREFERENCESPANEL_HH
