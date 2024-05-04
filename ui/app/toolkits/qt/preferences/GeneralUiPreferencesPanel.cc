// GeneralUiPreferencesPanel.cc --- base class for the break windows
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
#  include "config.h"
#endif

#define HAVE_LANGUAGE_SELECTION
#include "GeneralUiPreferencesPanel.hh"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include <QtGui>
#include <QStyle>

#include "debug.hh"

#include "core/ICore.hh"
#include "utils/Platform.hh"
#include "utils/Paths.hh"

#include "TimerPreferencesPanel.hh"
#include "UiUtil.hh"

using namespace workrave;
using namespace workrave::utils;

#if defined(PLATFORM_OS_WINDOWS)
#  include <windows.h>
#  define RUNKEY "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#endif

GeneralUiPreferencesPanel::GeneralUiPreferencesPanel(std::shared_ptr<IApplicationContext> app)
{
  connector = std::make_shared<DataConnector>(app);

  // Block types
  block_button = new QComboBox;
  block_button->addItem(tr("No blocking"));
  block_button->addItem(tr("Block input"));
  block_button->addItem(tr("Block input and screen"));

  int block_idx = 0;
  switch (GUIConfig::block_mode()())
    {
    case BlockMode::Off:
      block_idx = 0;
      break;
    case BlockMode::Input:
      block_idx = 1;
      break;
    default:
      block_idx = 2;
    }
  block_button->setCurrentIndex(block_idx);

  void (QComboBox::*signal)(int) = &QComboBox::currentIndexChanged;
  QObject::connect(block_button, signal, this, &GeneralUiPreferencesPanel::on_block_changed);

  // Options
  auto *layout = new QVBoxLayout;
  setLayout(layout);

  UiUtil::add_widget(layout, tr("Block mode:"), block_button);

#if defined(HAVE_LANGUAGE_SELECTION)
  std::string current_locale_name = GUIConfig::locale()();

  std::vector<std::string> all_linguas;
  std::string str(ALL_LINGUAS);
  boost::split(all_linguas, str, boost::is_any_of(" "));

  all_linguas.emplace_back("en");

  languages_combo = new QComboBox();
  model = new QStandardItemModel(static_cast<int>(all_linguas.size()), 2);

  auto *languages_view = new QTreeView;
  languages_combo->setView(languages_view);
  languages_view->setHeaderHidden(true);
  languages_view->setColumnWidth(0, 300);
  languages_view->setColumnWidth(1, 100);
  languages_view->setModel(model);

  languages_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  languages_view->setAllColumnsShowFocus(true);
  languages_view->setRootIsDecorated(false);

  languages_combo->setEditable(false);
  languages_combo->setModel(model);
  languages_combo->setModelColumn(0);

  model->setItem(0, 0, new QStandardItem(tr("System default")));
  model->setItem(0, 1, new QStandardItem(""));

  QLocale current_locale;

  int row = 1;
  int selected = 0;
  for (auto code: all_linguas)
    {
      if (current_locale_name == code)
        {
          selected = row;
        }

      QLocale l(QString::fromStdString(code));

      QString language = l.nativeLanguageName();
      QString country = l.nativeCountryName();

      if (language == "")
        {
          language = QString::fromStdString(code);
        }

      if (country != "")
        {
          language += " (" + country + ")";
        }

      model->setItem(row, 0, new QStandardItem(language));

      auto *item = new QStandardItem("");
      item->setTextAlignment(Qt::AlignRight);
      model->setItem(row, 1, item);

      item = new QStandardItem(code.c_str());
      model->setItem(row, 2, item);

      row++;
    }

  languages_view->setColumnHidden(2, true);
  languages_combo->setCurrentIndex(selected);
  UiUtil::add_widget(layout, tr("Language:"), languages_combo);
#endif

#if defined(PLATFORM_OS_WINDOWS)
  QCheckBox *autostart_cb = new QCheckBox;
  autostart_cb->setText(tr("Start Workrave on Windows startup"));
  connect(autostart_cb, &QCheckBox::stateChanged, this, &GeneralUiPreferencesPanel::on_autostart_toggled);

  layout->addWidget(autostart_cb);

  char value[MAX_PATH];
  bool rc = Platform::registry_get_value(RUNKEY, "Workrave", value);
  autostart_cb->setCheckState(rc ? Qt::Checked : Qt::Unchecked);
#endif

  auto *trayicon_cb = new QCheckBox;
  trayicon_cb->setText(tr("Show system tray icon"));
  connector->connect(GUIConfig::trayicon_enabled(), dc::wrap(trayicon_cb));

  layout->addWidget(trayicon_cb);

  auto *plugin_box = new QVBoxLayout;
  layout->addLayout(plugin_box);

  layout->addStretch();
}

GeneralUiPreferencesPanel::~GeneralUiPreferencesPanel()
{
  QStandardItem *item = model->item(languages_combo->currentIndex(), 2);
  if (item != nullptr)
    {
      GUIConfig::locale().set(item->text().toStdString());
    }
}

#if defined(PLATFORM_OS_WINDOWS)
void
GeneralUiPreferencesPanel::on_autostart_toggled()
{
  // bool on = autostart_cb->checkState() == Qt::Checked;
  // gchar *value = NULL;

  // if (on)
  //   {
  // string appdir = Paths::get_application_directory();

  // value = g_strdup_printf("%s" G_DIR_SEPARATOR_S "bin" G_DIR_SEPARATOR_S "workrave.exe", appdir.c_str());
  //}

  // Util::registry_set_value(RUNKEY, "Workrave", value);
}
#endif

void
GeneralUiPreferencesPanel::on_block_changed()
{
  int idx = block_button->currentIndex();
  BlockMode m{};
  switch (idx)
    {
    case 0:
      m = BlockMode::Off;
      break;
    case 1:
      m = BlockMode::Input;
      break;
    default:
      m = BlockMode::All;
    }
  GUIConfig::block_mode().set(m);
}
