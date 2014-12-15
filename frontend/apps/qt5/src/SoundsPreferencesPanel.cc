// SoundsPreferencesPanel.cc --- base class for the break windows
//
// Copyright (C) 2001 - 2014 Rob Caelers & Raymond Penners
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

#include <boost/filesystem.hpp>

#include "SoundsPreferencesPanel.hh"

#include <QtGui>
#include <QStyle>

#include "debug.hh"
#include "nls.h"

#include "TimerPreferencesPanel.hh"

#include "utils/Locale.hh"

#include "ICore.hh"
#include "UiUtil.hh"
#include "CoreFactory.hh"
#include "DataConnector.hh"

using namespace workrave;
using namespace workrave::utils;

SoundsPreferencesPanel::SoundsPreferencesPanel(SoundTheme::Ptr sound_theme)
  : sound_theme(sound_theme),
    connector(NULL),
    enabled_cb(NULL)
{
  TRACE_ENTER("SoundsPreferencesPanel::SoundsPreferencesPanel");

  connector = DataConnector::create();

  QVBoxLayout *layout = new QVBoxLayout;
  setLayout(layout);

  QGroupBox *sound_options_box = new QGroupBox(_("Sound Options"));
  layout->addWidget(sound_options_box);
  QVBoxLayout *sound_options_layout = new QVBoxLayout;
  sound_options_box->setLayout(sound_options_layout);

  if (sound_theme->capability(workrave::audio::SOUND_CAP_VOLUME))
    {
      QSlider *sound_volume_scale = new QSlider(Qt::Horizontal);
      sound_volume_scale->setMinimum(0);
      sound_volume_scale->setMaximum(100);
      sound_volume_scale->setSingleStep(1);
      sound_volume_scale->setPageStep(5);

      connector->connect(SoundTheme::sound_volume(), dc::wrap(sound_volume_scale));

      UiUtil::add_widget(sound_options_layout, _("Volume:"), sound_volume_scale);
    }

  enabled_cb = new QCheckBox;
  enabled_cb->setText(_("Enable sounds"));
  connector->connect(SoundTheme::sound_enabled(), dc::wrap(enabled_cb));
  sound_options_layout->addWidget(enabled_cb);

  if (sound_theme->capability(workrave::audio::SOUND_CAP_MUTE))
    {
      // Volume
      QCheckBox *mute_cb = new QCheckBox;
      mute_cb->setText(_("Mute sounds during rest break and daily limit"));
      connector->connect(SoundTheme::sound_mute(), dc::wrap(mute_cb));
      sound_options_layout->addWidget(mute_cb);
    }

  QGroupBox *sound_events_box = new QGroupBox(_("Sound Events"));
  layout->addWidget(sound_events_box);
  QVBoxLayout *sound_events_layout = new QVBoxLayout;
  sound_events_box->setLayout(sound_events_layout);

  sound_theme_button = new QComboBox;
  UiUtil::add_widget(sound_events_layout, _("Sound Theme:"), sound_theme_button);
  
  sound_theme_model = new QStandardItemModel();
  sound_theme_button->setModel(sound_theme_model);

  update_theme_selection();

  void (QComboBox:: *signal)(int) = &QComboBox::currentIndexChanged;
  QObject::connect(sound_theme_button, signal, this, &SoundsPreferencesPanel::on_sound_theme_changed);

  sounds_model = new QStandardItemModel(); // TODO: SOUND_MAX, 4);
  sounds_view = new QTreeView;
  sound_events_layout->addWidget(sounds_view);

  sounds_view->setModel(sounds_model);
  sounds_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  sounds_view->setAllColumnsShowFocus(true);
  sounds_view->setRootIsDecorated(false);

  connect(sounds_view, &QTreeView::activated, this, &SoundsPreferencesPanel::on_sound_item_activated);

  // sounds_view->setColumnWidth(0, 100);
  // sounds_view->setColumnWidth(1, 200);

  sounds_model->setHeaderData(0, Qt::Horizontal, _("Enabled"));
  sounds_model->setHeaderData(1, Qt::Horizontal, _("Sound"));

  SoundTheme::ThemeInfo::Ptr active_theme = sound_theme->get_active_theme();

  int item_count = 0;
  for (SoundTheme::SoundInfo snd : active_theme->sounds)
    {
      bool sound_enabled = SoundTheme::sound_event_enabled(snd.event)();

      QStandardItem *item = new QStandardItem();
      sounds_model->setItem(item_count, 0, item);
      item->setCheckable(true);
      item->setCheckState(sound_enabled ? Qt::Checked : Qt::Unchecked);
      
      sounds_model->setItem(item_count, 1, new QStandardItem(QString::fromStdString(SoundTheme::sound_event_to_friendly_name(snd.event))));
      sounds_model->setItem(item_count, 2, new QStandardItem(QString::fromStdString(SoundTheme::sound_event_to_id(snd.event))));
      sounds_model->setItem(item_count, 3, new QStandardItem(true));
      
      item_count++;
    }

  sounds_view->setColumnHidden(2, true);
  sounds_view->setColumnHidden(3, true);
  connect(sounds_model, &QStandardItemModel::itemChanged, this, &SoundsPreferencesPanel::on_sound_item_changed);
  
  QHBoxLayout *sound_buttons_layout = new QHBoxLayout;
  sound_events_layout->addLayout(sound_buttons_layout);

  QPushButton *sound_play_button = new QPushButton(_("Play"));
  sound_buttons_layout->addWidget(sound_play_button);
  connect(sound_play_button, &QPushButton::clicked, this, &SoundsPreferencesPanel::on_play_sound);

  QPushButton *sound_select_button = new QPushButton("Choose");
  sound_buttons_layout->addWidget(sound_select_button);
  connect(sound_select_button, &QPushButton::clicked, this, &SoundsPreferencesPanel::on_select_sound);

  sound_buttons_layout->addStretch();

  sounds_view->setCurrentIndex(sounds_model->index(0, 0));

  TRACE_EXIT();
}


SoundsPreferencesPanel::~SoundsPreferencesPanel()
{
}

void
SoundsPreferencesPanel::on_sound_item_activated(const QModelIndex & index)
{
}

void
SoundsPreferencesPanel::on_select_sound()
{
  SoundEvent event = currentEvent();
  std::string filename = SoundTheme::sound_event(event)();
  if (filename != "")
    {
      boost::filesystem::path path(filename);
      boost::filesystem::path dirname = path.parent_path();
      boost::filesystem::path basename = path.filename();
      
      QFileDialog *fd = new QFileDialog(this);
      fd->setAttribute(Qt::WA_DeleteOnClose, true);
      fd->setFileMode(QFileDialog::ExistingFile);
      fd->setDirectory(QString::fromStdString(dirname.string()));
      fd->setLabelText(QFileDialog::Accept, _("Select"));
      fd->show();

      connect(fd, &QFileDialog::fileSelected, this, &SoundsPreferencesPanel::on_sound_selected);
    }
}

void
SoundsPreferencesPanel::on_play_sound()
{
  SoundEvent event = currentEvent();
  std::string filename = SoundTheme::sound_event(event)();
  if (filename != "")
    {
      sound_theme->play_sound(filename);
    }
}

void
SoundsPreferencesPanel::on_sound_selected(const QString &filename)
{
  SoundEvent event = currentEvent();
  SoundTheme::sound_event(event).set(filename.toStdString());
  update_theme_selection();
}

void 
SoundsPreferencesPanel::on_sound_item_changed(QStandardItem *item)
{
  QStandardItem *iditem = sounds_model->item(item->index().row(), 2);
  std::string id = iditem->text().toStdString();
  SoundEvent event = SoundTheme::sound_id_to_event(id);

  bool enabled = item->checkState() == Qt::Checked;
  SoundTheme::sound_event_enabled(event).set(enabled);
}

void
SoundsPreferencesPanel::on_sound_theme_changed(int index)
{
  TRACE_ENTER_MSG("SoundsPreferencesPanel::on_sound_theme_changed", index);
  QStandardItem *item = sound_theme_model->item(index);
  std::string theme_id = item->data().toString().toStdString();
  TRACE_MSG(theme_id);
  sound_theme->activate_theme(theme_id);
  TRACE_EXIT();
}

void
SoundsPreferencesPanel::update_theme_selection()
{
  TRACE_ENTER("SoundsPreferencesPanel::update_theme_selection");
  sound_theme_model->clear();

  SoundTheme::ThemeInfo::Ptr active_theme = sound_theme->get_active_theme();
  
  int active_index = -1;
  for (SoundTheme::ThemeInfo::Ptr theme : sound_theme->get_themes())
    {
      QStandardItem *item = new QStandardItem(QString::fromStdString(theme->description));
      item->setData(QVariant(QString::fromStdString(theme->theme_id)));
      sound_theme_model->appendRow(item);
      
      if (theme == active_theme)
        {
          active_index = sound_theme_model->indexFromItem(item).row();
        }
    }

  if (active_index == -1)
    {
      QStandardItem *item = new QStandardItem(QString::fromStdString(_("Custom")));
      sound_theme_model->appendRow(item);
      active_index = sound_theme_model->indexFromItem(item).row();
    }

  sound_theme_button->setCurrentIndex(active_index);
  TRACE_EXIT();
}

SoundEvent 
SoundsPreferencesPanel::currentEvent() const
{
  const QModelIndex &index = sounds_view->currentIndex();
  QStandardItem *item = sounds_model->item(index.row(), 2);
  std::string id = item->text().toStdString();
  return SoundTheme::sound_id_to_event(id);
}
