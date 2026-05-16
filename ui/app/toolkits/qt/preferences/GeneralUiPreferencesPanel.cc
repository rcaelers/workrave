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

#include <filesystem>
#include <set>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include <QtGui>
#include <QStyle>

#include "utils/Platform.hh"
#include "utils/Paths.hh"
#include "utils/AssetPath.hh"

#include "UiUtil.hh"
#include "ui/GUIConfig.hh"

using namespace workrave;
using namespace workrave::utils;

#if defined(PLATFORM_OS_WINDOWS)
#  include <windows.h>
constexpr auto RUNKEY = R"(Software\Microsoft\Windows\CurrentVersion\Run)";
#endif

GeneralUiPreferencesPanel::GeneralUiPreferencesPanel(std::shared_ptr<IApplicationContext> app)
{
  connector = std::make_shared<DataConnector>(app);
  size_group = std::make_shared<SizeGroup>(Qt::Horizontal);

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

  UiUtil::add_widget(layout, tr("Block mode:"), block_button, size_group);

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
      QString country = l.nativeTerritoryName();

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
  UiUtil::add_widget(layout, tr("Language:"), languages_combo, size_group);
#endif

  update_icon_theme_combo();
  if (icon_theme_combo != nullptr)
    {
      UiUtil::add_widget(layout, tr("Icon Theme:"), icon_theme_combo, size_group);
    }

#if defined(PLATFORM_OS_WINDOWS)
  autostart_cb = new QCheckBox;
  autostart_cb->setText(tr("Start Workrave on Windows startup"));

  layout->addWidget(autostart_cb);

  auto value = Platform::registry_get_value(RUNKEY, "Workrave");
  autostart_cb->setCheckState(value.has_value() ? Qt::Checked : Qt::Unchecked);
  connect(autostart_cb, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState) { on_autostart_toggled(); });

  light_dark_combo = new QComboBox;
  light_dark_combo->addItem(tr("Light"));
  light_dark_combo->addItem(tr("Dark"));
  light_dark_combo->addItem(tr("Auto"));
  connector->connect(GUIConfig::light_dark_mode(), dc::wrap(light_dark_combo));
  UiUtil::add_widget(layout, tr("Dark mode:"), light_dark_combo, size_group);
#endif

  auto *trayicon_cb = new QCheckBox;
  trayicon_cb->setText(tr("Show system tray icon"));
  trayicon_cb->setToolTip(tr("Note that not all desktop environments show system tray icons, or have disabled system tray icons by default."));
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
  bool on = autostart_cb->checkState() == Qt::Checked;

  if (on)
    {
      auto exe = Paths::get_application_directory() / "bin" / "workrave.exe";
      Platform::registry_set_value(RUNKEY, "Workrave", exe.string().c_str());
    }
  else
    {
      Platform::registry_set_value(RUNKEY, "Workrave", nullptr);
    }
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

void
GeneralUiPreferencesPanel::on_icon_theme_changed()
{
  if (icon_theme_combo == nullptr)
    {
      return;
    }

  int idx = icon_theme_combo->currentIndex();
  GUIConfig::icon_theme().set(idx <= 0 ? std::string() : icon_theme_combo->currentText().toStdString());
}

void
GeneralUiPreferencesPanel::update_icon_theme_combo()
{
  std::set<std::string> themes;

  for (const auto &dirname: AssetPath::get_search_path(SearchPathId::Images))
    {
      if (dirname.filename() != "images")
        {
          continue;
        }

      std::error_code ec;
      if (!std::filesystem::is_directory(dirname, ec))
        {
          continue;
        }

      for (const auto &entry: std::filesystem::directory_iterator(dirname, ec))
        {
          std::error_code entry_ec;
          if (entry.is_directory(entry_ec))
            {
              themes.insert(entry.path().filename().string());
            }
        }
    }

  if (themes.empty())
    {
      return;
    }

  icon_theme_combo = new QComboBox;
  icon_theme_combo->addItem(tr("Default"));

  std::string current_icon_theme = GUIConfig::icon_theme()();
  int selected = 0;
  int idx = 1;
  for (const auto &theme: themes)
    {
      icon_theme_combo->addItem(QString::fromStdString(theme));
      if (current_icon_theme == theme)
        {
          selected = idx;
        }
      idx++;
    }

  icon_theme_combo->setCurrentIndex(selected);

  void (QComboBox::*signal)(int) = &QComboBox::currentIndexChanged;
  QObject::connect(icon_theme_combo, signal, this, &GeneralUiPreferencesPanel::on_icon_theme_changed);
}
