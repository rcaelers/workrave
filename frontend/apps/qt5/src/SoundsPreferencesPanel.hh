// SoundsPreferencesPanel.hh
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

#ifndef SOUNDSPREFERENCESPANEL_HH
#define SOUNDSPREFERENCESPANEL_HH

#include <QtGui>
#include <QtWidgets>

#include "DataConnector.hh"
#include "SoundTheme.hh"

class SoundsPreferencesPanel : public QWidget
{
  Q_OBJECT

public:
  SoundsPreferencesPanel(SoundTheme::Ptr sound_theme);
  virtual ~SoundsPreferencesPanel();

private:
  void on_sound_theme_changed(int index);
  void on_sound_item_activated(const QModelIndex & index);
  void on_select_sound();
  void on_play_sound();
  void on_sound_selected(const QString &filename);
  void on_sound_item_changed(QStandardItem *item);

  void update_theme_selection();

  SoundEvent currentEvent() const;

private:
  SoundTheme::Ptr sound_theme;
  DataConnector::Ptr connector;

  QCheckBox *enabled_cb;
  QComboBox *sound_theme_button;
  QStandardItemModel *sound_theme_model;
  QTreeView *sounds_view;
  QStandardItemModel *sounds_model;
};

#endif // SOUNDSPREFERENCESPANEL_HH
