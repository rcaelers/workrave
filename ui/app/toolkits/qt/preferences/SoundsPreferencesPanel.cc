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
#  include "config.h"
#endif

#include "SoundsPreferencesPanel.hh"

#include <filesystem>
#include <memory>

#include <QtGui>
#include <QDir>
#include <QGridLayout>
#include <QStyle>

#include "DataConnector.hh"
#include "Ui.hh"
#include "UiUtil.hh"

using namespace workrave;
using namespace workrave::utils;

SoundsPreferencesPanel::SoundsPreferencesPanel(std::shared_ptr<IApplicationContext> app)
  : sound_theme(app->get_sound_theme())
{
  connector = std::make_shared<DataConnector>(app);

  auto *layout = new QVBoxLayout;
  setLayout(layout);

  auto *sound_options_box = new QGroupBox(tr("Sound Options"));
  layout->addWidget(sound_options_box);
  auto *sound_options_layout = new QVBoxLayout;
  sound_options_box->setLayout(sound_options_layout);

  if (sound_theme->capability(workrave::audio::SoundCapability::VOLUME))
    {
      sound_volume_scale = new QSlider(Qt::Horizontal);
      sound_volume_scale->setMinimum(0);
      sound_volume_scale->setMaximum(100);
      sound_volume_scale->setSingleStep(1);
      sound_volume_scale->setPageStep(5);

      connector->connect(sound_theme->sound_volume(), dc::wrap(sound_volume_scale));

      UiUtil::add_widget(sound_options_layout, tr("Volume:"), sound_volume_scale);
    }

  enabled_cb = new QCheckBox;
  enabled_cb->setText(tr("Enable sounds"));
  connector->connect(sound_theme->sound_enabled(), dc::wrap(enabled_cb));
  sound_options_layout->addWidget(enabled_cb);
  connect(enabled_cb, &QCheckBox::toggled, this, [this]() { update_sensitives(); });

  if (sound_theme->capability(workrave::audio::SoundCapability::MUTE))
    {
      mute_cb = new QCheckBox;
      mute_cb->setText(tr("Mute sounds during rest break and daily limit"));
      connector->connect(sound_theme->sound_mute(), dc::wrap(mute_cb));
      sound_options_layout->addWidget(mute_cb);
    }

  sound_events_box = new QGroupBox(tr("Sound Events"));
  layout->addWidget(sound_events_box);
  auto *sound_events_layout = new QVBoxLayout;
  sound_events_box->setLayout(sound_events_layout);

  sound_theme_button = new QComboBox;
  UiUtil::add_widget(sound_events_layout, tr("Sound Theme:"), sound_theme_button);

  sound_theme_model = new QStandardItemModel();
  sound_theme_button->setModel(sound_theme_model);

  update_theme_selection();

  void (QComboBox::*signal)(int) = &QComboBox::currentIndexChanged;
  QObject::connect(sound_theme_button, signal, this, &SoundsPreferencesPanel::on_sound_theme_changed);

  sounds_model = new QStandardItemModel();
  sounds_view = new QTreeView;
  sound_events_layout->addWidget(sounds_view);

  sounds_view->setModel(sounds_model);
  sounds_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  sounds_view->setAllColumnsShowFocus(true);
  sounds_view->setRootIsDecorated(false);

  connect(sounds_view, &QTreeView::activated, this, &SoundsPreferencesPanel::on_sound_item_activated);
  connect(sounds_view->selectionModel(), &QItemSelectionModel::currentRowChanged, this, [this]() { update_selected_sound_file(); });

  // sounds_view->setColumnWidth(0, 100);
  // sounds_view->setColumnWidth(1, 200);

  sounds_model->setColumnCount(4);
  sounds_model->setHeaderData(0, Qt::Horizontal, tr("Enabled"));
  sounds_model->setHeaderData(1, Qt::Horizontal, tr("Sound"));

  SoundTheme::ThemeInfo::Ptr active_theme = sound_theme->get_active_theme();

  int item_count = 0;
  for (SoundTheme::SoundInfo snd: active_theme->sounds)
    {
      bool sound_enabled = sound_theme->sound_event_enabled(snd.event)();

      auto *item = new QStandardItem();
      sounds_model->setItem(item_count, 0, item);
      item->setCheckable(true);
      item->setCheckState(sound_enabled ? Qt::Checked : Qt::Unchecked);

      sounds_model->setItem(item_count, 1, new QStandardItem(Ui::get_sound_event_name(snd.event)));
      sounds_model->setItem(item_count, 2, new QStandardItem(QString::fromStdString(sound_theme->sound_event_to_id(snd.event))));
      sounds_model->setItem(item_count, 3, new QStandardItem(1));

      item_count++;
    }

  sounds_view->setColumnHidden(2, true);
  sounds_view->setColumnHidden(3, true);
  connect(sounds_model, &QStandardItemModel::itemChanged, this, &SoundsPreferencesPanel::on_sound_item_changed);

  selected_sound_label = new QLabel;
  selected_sound_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  selected_sound_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  UiUtil::add_widget(sound_events_layout, tr("File:"), selected_sound_label);

  auto *sound_buttons_layout = new QHBoxLayout;
  sound_events_layout->addLayout(sound_buttons_layout);

  sound_play_button = new QPushButton(tr("Play"));
  sound_buttons_layout->addWidget(sound_play_button);
  connect(sound_play_button, &QPushButton::clicked, this, &SoundsPreferencesPanel::on_play_sound);

  sound_select_button = new QPushButton(tr("Choose"));
  sound_buttons_layout->addWidget(sound_select_button);
  connect(sound_select_button, &QPushButton::clicked, this, &SoundsPreferencesPanel::on_select_sound);

  sound_buttons_layout->addStretch();

  sounds_view->setCurrentIndex(sounds_model->index(0, 0));
  update_selected_sound_file();
  update_sensitives();
}

void
SoundsPreferencesPanel::on_sound_item_activated(const QModelIndex &index)
{
  // TODO:
}

void
SoundsPreferencesPanel::on_select_sound()
{
  std::string filename = currentSoundFilename();

  auto *fd = new QFileDialog(this, tr("Choose a sound"));
  fd->setAttribute(Qt::WA_DeleteOnClose, true);
  fd->setFileMode(QFileDialog::ExistingFile);
  fd->setNameFilter(tr("Wave files (*.wav)"));
  fd->setOption(QFileDialog::DontUseNativeDialog, true);
  fd->setLabelText(QFileDialog::Accept, tr("Select"));

  if (!filename.empty())
    {
      std::filesystem::path path(filename);
      std::filesystem::path dirname = path.parent_path();
      fd->setDirectory(QString::fromStdString(dirname.string()));
      fd->selectFile(QString::fromStdString(path.filename().string()));
    }

  auto preview_filename = std::make_shared<QString>(QString::fromStdString(filename));
  auto *preview_button = new QPushButton(tr("Play"), fd);
  if (auto *dialog_layout = qobject_cast<QGridLayout *>(fd->layout()))
    {
      dialog_layout->addWidget(preview_button, dialog_layout->rowCount(), 0, 1, dialog_layout->columnCount());
    }

  connect(fd, &QFileDialog::currentChanged, this, [preview_filename](const QString &path) { *preview_filename = path; });
  connect(preview_button, &QPushButton::clicked, this, [this, preview_filename]() {
    if (!preview_filename->isEmpty())
      {
        sound_theme->play_sound(preview_filename->toStdString());
      }
  });
  connect(fd, &QFileDialog::fileSelected, this, &SoundsPreferencesPanel::on_sound_selected);

  fd->show();
}

void
SoundsPreferencesPanel::on_play_sound()
{
  std::string filename = currentSoundFilename();
  if (!filename.empty())
    {
      sound_theme->play_sound(filename);
    }
}

void
SoundsPreferencesPanel::on_sound_selected(const QString &filename)
{
  SoundEvent event = currentEvent();
  sound_theme->sound_event(event).set(filename.toStdString());
  update_theme_selection();
  update_selected_sound_file();
}

void
SoundsPreferencesPanel::on_sound_item_changed(QStandardItem *item)
{
  QStandardItem *iditem = sounds_model->item(item->index().row(), 2);
  std::string id = iditem->text().toStdString();
  SoundEvent event = sound_theme->sound_id_to_event(id);

  bool enabled = item->checkState() == Qt::Checked;
  sound_theme->sound_event_enabled(event).set(enabled);
}

void
SoundsPreferencesPanel::on_sound_theme_changed(int index)
{
  QStandardItem *item = sound_theme_model->item(index);
  if (item == nullptr)
    {
      return;
    }

  std::string theme_id = item->data().toString().toStdString();
  if (theme_id.empty())
    {
      return;
    }

  sound_theme->activate_theme(theme_id);
  update_selected_sound_file();
}

void
SoundsPreferencesPanel::update_theme_selection()
{
  sound_theme_model->clear();

  SoundTheme::ThemeInfo::Ptr active_theme = sound_theme->get_active_theme();

  int active_index = -1;
  for (SoundTheme::ThemeInfo::Ptr theme: sound_theme->get_themes())
    {
      auto *item = new QStandardItem(QString::fromStdString(theme->description));
      item->setData(QVariant(QString::fromStdString(theme->theme_id)));
      sound_theme_model->appendRow(item);

      if (theme == active_theme)
        {
          active_index = sound_theme_model->indexFromItem(item).row();
        }
    }

  if (active_index == -1)
    {
      auto *item = new QStandardItem(tr("Custom"));
      sound_theme_model->appendRow(item);
      active_index = sound_theme_model->indexFromItem(item).row();
    }

  sound_theme_button->setCurrentIndex(active_index);
}

void
SoundsPreferencesPanel::update_selected_sound_file()
{
  if (selected_sound_label == nullptr)
    {
      return;
    }

  std::string filename = currentSoundFilename();
  if (filename.empty())
    {
      selected_sound_label->setText(tr("(No sound file)"));
    }
  else
    {
      selected_sound_label->setText(QDir::toNativeSeparators(QString::fromStdString(filename)));
    }
}

void
SoundsPreferencesPanel::update_sensitives()
{
  bool enabled = enabled_cb == nullptr || enabled_cb->isChecked();

  if (sound_volume_scale != nullptr)
    {
      sound_volume_scale->setEnabled(enabled);
    }
  if (mute_cb != nullptr)
    {
      mute_cb->setEnabled(enabled);
    }
  if (sound_events_box != nullptr)
    {
      sound_events_box->setEnabled(enabled);
    }
}

auto
SoundsPreferencesPanel::currentEvent() const -> SoundEvent
{
  const QModelIndex &index = sounds_view->currentIndex();
  QStandardItem *item = sounds_model->item(index.row(), 2);
  std::string id = item->text().toStdString();
  return sound_theme->sound_id_to_event(id);
}

auto
SoundsPreferencesPanel::currentSoundFilename() const -> std::string
{
  if (sounds_view == nullptr || sounds_model == nullptr || !sounds_view->currentIndex().isValid())
    {
      return {};
    }

  SoundEvent event = currentEvent();
  return sound_theme->sound_event(event)();
}
