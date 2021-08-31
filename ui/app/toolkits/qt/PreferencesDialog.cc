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

#include "core/ICore.hh"
#include "utils/AssetPath.hh"

#include "IconListNotebook.hh"
#include "GeneralUiPreferencesPanel.hh"
#include "SoundsPreferencesPanel.hh"
#include "TimerBoxPreferencesPanel.hh"
#include "TimerPreferencesPanel.hh"
#include "Ui.hh"
#include "UiUtil.hh"

using namespace workrave;
using namespace workrave::utils;

PreferencesDialog::PreferencesDialog(std::shared_ptr<IApplication> app)
  : app(app)
{
  connector = std::make_shared<DataConnector>(app);

  auto *layout = new QVBoxLayout();
  setLayout(layout);

  auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
  QPushButton *close_button = buttonBox->button(QDialogButtonBox::Close);
  close_button->setAutoDefault(true);
  close_button->setDefault(true);

  notebook = new IconListNotebook();
  layout->addWidget(notebook);

  connect(buttonBox, SIGNAL(rejected()), this, SLOT(accept()));
  layout->addWidget(buttonBox);

  hsize_group = std::make_shared<SizeGroup>(Qt::Horizontal);
  vsize_group = std::make_shared<SizeGroup>(Qt::Horizontal);

  QWidget *timer_page = create_timer_page();
  add_page(tr("Timers"), "time.png", timer_page);

  QWidget *gui_page = create_ui_page();
  add_page(tr("User interface"), "display.png", gui_page);
}

auto
PreferencesDialog::create_timer_page() -> QWidget *
{
  auto *timer_tab = new QTabWidget;
  timer_tab->setTabPosition(QTabWidget::North);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      auto *panel = new TimerPreferencesPanel(app, BreakId(i), hsize_group, vsize_group);
      QPixmap pixmap(Ui::get_break_icon_filename(BreakId(i)));
      QIcon icon(pixmap);

      timer_tab->addTab(panel, icon, Ui::get_break_name(BreakId(i)));
    }

  return timer_tab;
}

auto
PreferencesDialog::create_ui_page() -> QWidget *
{
  auto *timer_tab = new QTabWidget;
  timer_tab->setTabPosition(QTabWidget::North);

  timer_tab->addTab(new GeneralUiPreferencesPanel(app), tr("General"));
  timer_tab->addTab(new SoundsPreferencesPanel(app), tr("Sounds"));
  timer_tab->addTab(new TimerBoxPreferencesPanel(app, "main_window"), tr("Status Window"));
  timer_tab->addTab(new TimerBoxPreferencesPanel(app, "applet"), tr("Applet"));

  return timer_tab;
}

void
PreferencesDialog::add_page(const QString &label, const char *image, QWidget *page)
{
  std::string file = AssetPath::complete_directory(image, AssetPath::SEARCH_PATH_IMAGES);
  QPixmap pixmap(file.c_str());
  QIcon icon(pixmap);

  notebook->add_page(page, icon, label);
}
