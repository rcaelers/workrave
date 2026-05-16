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

#include "ui/IApplicationContext.hh"

#include "DataConnector.hh"

class SoundsPreferencesPanel : public QWidget
{
  Q_OBJECT

public:
  explicit SoundsPreferencesPanel(std::shared_ptr<IApplicationContext> app);

private:
  void on_sound_theme_changed(int index);
  void on_sound_item_activated(const QModelIndex &index);
  void on_select_sound();
  void on_play_sound();
  void on_sound_selected(const QString &filename);
  void on_sound_item_changed(QStandardItem *item);

  void update_theme_selection();
  void update_selected_sound_file();
  void update_sensitives();

  auto currentEvent() const -> SoundEvent;
  auto currentSoundFilename() const -> std::string;

private:
  SoundTheme::Ptr sound_theme;
  DataConnector::Ptr connector;

  QCheckBox *enabled_cb{nullptr};
  QSlider *sound_volume_scale{nullptr};
  QCheckBox *mute_cb{nullptr};
  QGroupBox *sound_events_box{nullptr};
  QComboBox *sound_theme_button{nullptr};
  QStandardItemModel *sound_theme_model{nullptr};
  QTreeView *sounds_view{nullptr};
  QStandardItemModel *sounds_model{nullptr};
  QPushButton *sound_play_button{nullptr};
  QPushButton *sound_select_button{nullptr};
  QLabel *selected_sound_label{nullptr};
};

#endif // SOUNDSPREFERENCESPANEL_HH
