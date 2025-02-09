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

#include "PreferencesDialog.hh"

#include <QtGui>
#include <QStyle>

#include "debug.hh"

#include "IconListNotebook.hh"
#include "GeneralUiPreferencesPanel.hh"
#include "MonitoringPreferencesPanel.hh"
#include "SoundsPreferencesPanel.hh"
#include "TimerBoxPreferencesPanel.hh"
#include "TimerPreferencesPanel.hh"

#include "Ui.hh"
#include "Icon.hh"

#include "core/CoreTypes.hh"
#include "ui/GUIConfig.hh"
#include "ui/prefwidgets/qt/Builder.hh"
#include "utils/AssetPath.hh"
#include "utils/Enum.hh"
#include "utils/EnumIterator.hh"

using namespace workrave;
using namespace workrave::utils;

PreferencesDialog::PreferencesDialog(std::shared_ptr<IApplicationContext> app, QWidget *parent)
  : QDialog(parent)
  , app(app)
{
  TRACE_ENTRY();

  init_ui();

  create_timers_page();
  create_ui_page();
  create_monitoring_page();
  create_sounds_page();

  create_plugin_pages();
  create_plugin_panels();
}

PreferencesDialog::~PreferencesDialog()
{
  TRACE_ENTRY();
  auto core = app->get_core();
  core->remove_operation_mode_override("preferences");
}

void
PreferencesDialog::init_ui()
{
  setMinimumSize(960, 640);

  auto *layout = new QVBoxLayout();
  setLayout(layout);

  auto *button_box = new QDialogButtonBox(QDialogButtonBox::Close);
  QPushButton *close_button = button_box->button(QDialogButtonBox::Close);
  close_button->setAutoDefault(true);
  close_button->setDefault(true);

  notebook = new IconListNotebook();
  layout->addWidget(notebook);

  connect(button_box, SIGNAL(rejected()), this, SLOT(accept()));
  layout->addWidget(button_box);
}

void
PreferencesDialog::create_timers_page()
{
  auto page = add_page("timer", tr("Timers"), "timer.svg");

  auto hsize_group = std::make_shared<SizeGroup>(Qt::Horizontal);
  auto vsize_group = std::make_shared<SizeGroup>(Qt::Horizontal);

  for (auto id: workrave::utils::enum_range<BreakId>())
    {
      auto *panel = new TimerPreferencesPanel(app, id, hsize_group, vsize_group);
      QPixmap pixmap(Ui::get_break_icon_filename(id));
      QIcon icon(pixmap);
      page->add_panel(std::string(workrave::utils::enum_to_string(id)), panel, Ui::get_break_name(id), icon);
    }
}

void
PreferencesDialog::create_ui_page()
{
  auto page = add_page("ui", tr("User interface"), "display.svg");

  QWidget *gui_general_page = new GeneralUiPreferencesPanel(app);
  page->add_panel("general", gui_general_page, tr("General"));

  QWidget *gui_mainwindow_page = new TimerBoxPreferencesPanel(app, "main_window");
  page->add_panel("mainwindow", gui_mainwindow_page, tr("Status Window"));

  QWidget *gui_applet_page = new TimerBoxPreferencesPanel(app, "applet");
  page->add_panel("applet", gui_applet_page, tr("Applet"));
}

void
PreferencesDialog::create_monitoring_page()
{
  auto page = add_page("monitoring", tr("Monitoring"), "mouse.svg");

  QWidget *monitoring_panel = new MonitoringPreferencesPanel(app);
  page->add_panel("monitoring", monitoring_panel, tr("Monitoring"));
}

void
PreferencesDialog::create_sounds_page()
{
  auto page = add_page("sounds", tr("Sounds"), "sound.svg");

  QWidget *gui_sounds_page = new SoundsPreferencesPanel(app);
  page->add_panel("sounds", gui_sounds_page, tr("Sounds"));
}

void
PreferencesDialog::create_plugin_pages()
{
  auto preferences_registry = app->get_internal_preferences_registry();
  for (auto &[id, def]: preferences_registry->get_pages())
    {
      auto &[label, image] = def;
      spdlog::info("Adding page {} with label {} and image {}", id, label, image);
      auto page = add_page(id, QString::fromStdString(label), image);
    }
}

void
PreferencesDialog::create_panel(std::shared_ptr<ui::prefwidgets::Def> &def)
{
  auto pageid = def->get_page();
  auto panelid = def->get_panel();
  auto widget = def->get_widget();

  auto pageit = pages.find(pageid);
  if (pageit == pages.end())
    {
      spdlog::error("Cannot find page {} when adding panel {}", pageid, panelid);
      return;
    }

  auto page = pageit->second;
  if (auto paneldef = std::dynamic_pointer_cast<ui::prefwidgets::PanelDef>(def); paneldef)
    {
      ui::prefwidgets::qt::Builder builder;

      auto *box = new QVBoxLayout;
      auto *boxwidget = new QWidget;
      boxwidget->setLayout(box);

      auto frame = std::make_shared<ui::prefwidgets::qt::BoxWidget>(box);
      builder.build(widget, frame);
      page->add_panel(panelid, boxwidget, QString::fromStdString(paneldef->get_label()));
      frames.push_back(frame);
    }
}

void
PreferencesDialog::create_plugin_panels()
{
  auto preferences_registry = app->get_internal_preferences_registry();

  for (auto &[id, deflist]: preferences_registry->get_widgets())
    {
      for (auto def: deflist)
        {
          create_panel(def);
        }
    }
}

std::shared_ptr<PreferencesPage>
PreferencesDialog::add_page(const std::string &id, const QString &label, const std::string &image)
{
  auto *tabs = new QTabWidget;
  tabs->setTabPosition(QTabWidget::North);
  tabs->setTabBarAutoHide(true);

  std::string file = AssetPath::complete_directory(image, SearchPathId::Images);
  Icon icon(file);

  auto page_info = std::make_shared<PreferencesPage>(id, tabs);
  pages[id] = page_info;

  notebook->add_page(tabs, icon, label);

  return page_info;
}

bool
PreferencesDialog::eventFilter(QObject *watched, QEvent *event)
{
  if (event->type() == QEvent::FocusIn)
    {
      TRACE_ENTRY();
      BlockMode block_mode = GUIConfig::block_mode()();
      if (block_mode != BlockMode::Off)
        {
          auto core = app->get_core();
          OperationMode mode = core->get_active_operation_mode();
          if (mode == OperationMode::Normal)
            {
              core->set_operation_mode_override(OperationMode::Quiet, "preferences");
            }
        }
    }
  else if (event->type() == QEvent::FocusOut)
    {
      TRACE_ENTRY();
      auto core = app->get_core();
      core->remove_operation_mode_override("preferences");
    }
  return QDialog::eventFilter(watched, event);
}

PreferencesPage::PreferencesPage(const std::string &id, QTabWidget *notebook)
  : id(id)
  , notebook(notebook)
{
}

void
PreferencesPage::add_panel(const std::string &id, QWidget *widget, const QString &label)
{
  panels.emplace(id, widget);
  panel_order.push_back(id);

  auto *layout = new QVBoxLayout;
  layout->addWidget(widget);
  layout->addStretch();
  auto *layoutwidget = new QWidget;
  layoutwidget->setLayout(layout);

  notebook->addTab(layoutwidget, label);
}

void
PreferencesPage::add_panel(const std::string &id, QWidget *widget, const QString &label, QIcon icon)
{
  panels.emplace(id, widget);
  panel_order.push_back(id);
  notebook->addTab(widget, icon, label);
}
