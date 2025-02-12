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

#include "DataConnector.hh"
#include "SizeGroup.hh"
#include "ui/prefwidgets/qt/BoxWidget.hh"

class GeneralUiPreferencesPanel : public QWidget
{
  Q_OBJECT

public:
  explicit GeneralUiPreferencesPanel(std::shared_ptr<IApplicationContext> app);
  ~GeneralUiPreferencesPanel() override;

private:
  void on_block_changed();

#if defined(PLATFORM_OS_WINDOWS)
  void on_autostart_toggled();
#endif
  void on_icon_theme_changed();

private:
  DataConnector::Ptr connector;

  QComboBox *block_button{nullptr};
  QComboBox *languages_combo{nullptr};
  QStandardItemModel *model{nullptr};

  QComboBox *icon_theme_combo{nullptr}; // TODO
  QCheckBox *trayicon_cb{nullptr};

#if defined(PLATFORM_OS_UNIX)
  QCheckBox *force_x11_cb{nullptr}; // TODO
#endif
#if defined(PLATFORM_OS_WINDOWS)
  QCheckBox *autostart_cb{nullptr};
#endif

  std::shared_ptr<SizeGroup> size_group;
  std::shared_ptr<ui::prefwidgets::qt::BoxWidget> plugin_frame;
};

#endif // GENERALUIPREFERENCESPANEL_HH
