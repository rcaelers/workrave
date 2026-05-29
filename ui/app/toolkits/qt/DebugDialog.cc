// Copyright (C) 2001 - 2015 Rob Caelers & Raymond Penners
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

#include "DebugDialog.hh"

#include <QStyle>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QtGui>

#include "core/ICore.hh"
#include "core/IApp.hh"
#include "ui/UiTypes.hh"

using namespace workrave;

DebugDialog::DebugDialog(std::shared_ptr<IApplicationContext> app)
  : app(app)
{
  setWindowTitle(tr("Debug Workrave"));
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  auto *vbox = new QVBoxLayout(this);
  vbox->setSpacing(8);
  vbox->setContentsMargins(12, 12, 12, 12);

  auto make_row = [&](const QString &label, QPushButton *&btn, const QString &show_text) {
    auto *row = new QHBoxLayout();
    auto *lbl = new QLabel(label);
    lbl->setMinimumWidth(120);
    btn = new QPushButton(show_text);
    btn->setMinimumWidth(100);
    row->addWidget(lbl);
    row->addWidget(btn);
    row->addStretch();
    vbox->addLayout(row);
  };

  make_row(tr("Micro break:"), btn_micro, tr("Show"));
  make_row(tr("Rest break:"), btn_rest, tr("Show"));
  make_row(tr("Daily limit:"), btn_daily, tr("Show"));
  make_row(tr("Prelude:"), btn_prelude, tr("Show"));

  vbox->addSpacing(8);

  auto *btn_stop_all = new QPushButton(tr("Stop All"));
  vbox->addWidget(btn_stop_all);

  connect(btn_micro, &QPushButton::clicked, this, [this]() {
    toggle_break(BREAK_ID_MICRO_BREAK, micro_window, btn_micro);
  });
  connect(btn_rest, &QPushButton::clicked, this, [this]() {
    toggle_break(BREAK_ID_REST_BREAK, rest_window, btn_rest);
  });
  connect(btn_daily, &QPushButton::clicked, this, [this]() {
    toggle_break(BREAK_ID_DAILY_LIMIT, daily_window, btn_daily);
  });
  connect(btn_prelude, &QPushButton::clicked, this, [this]() {
    toggle_prelude(BREAK_ID_REST_BREAK, prelude_window, btn_prelude);
  });
  connect(btn_stop_all, &QPushButton::clicked, this, [this]() { stop_all(); });

  adjustSize();
}

DebugDialog::~DebugDialog()
{
  stop_all();
}

void
DebugDialog::toggle_break(BreakId break_id, IBreakWindow::Ptr &window, QPushButton *button)
{
  if (window)
    {
      window->stop();
      window.reset();
      button->setText(tr("Show"));
      return;
    }

  window = app->get_toolkit()->create_break_window(0, break_id, BREAK_FLAGS_POSTPONABLE | BREAK_FLAGS_SKIPPABLE);
  window->init();
  window->set_progress(0, 300);
  window->start();
  button->setText(tr("Stop"));
}

void
DebugDialog::toggle_prelude(BreakId break_id, IPreludeWindow::Ptr &window, QPushButton *button)
{
  if (window)
    {
      window->stop();
      window.reset();
      button->setText(tr("Show"));
      return;
    }

  window = app->get_toolkit()->create_prelude_window(0, break_id);
  window->set_stage(IApp::PreludeStage::Initial);
  window->set_progress_text(IApp::PreludeProgressText::BreakIn);
  window->set_progress(0, 20);
  window->start();
  button->setText(tr("Stop"));
}

void
DebugDialog::stop_all()
{
  if (micro_window)
    {
      micro_window->stop();
      micro_window.reset();
      btn_micro->setText(tr("Show"));
    }
  if (rest_window)
    {
      rest_window->stop();
      rest_window.reset();
      btn_rest->setText(tr("Show"));
    }
  if (daily_window)
    {
      daily_window->stop();
      daily_window.reset();
      btn_daily->setText(tr("Show"));
    }
  if (prelude_window)
    {
      prelude_window->stop();
      prelude_window.reset();
      btn_prelude->setText(tr("Show"));
    }
}
