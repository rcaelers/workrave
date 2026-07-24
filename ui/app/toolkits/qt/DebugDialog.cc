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

#include <sstream>

#include <QStyle>
#include <QPushButton>
#include <QScrollBar>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QtGui>

#ifdef HAVE_CORE_SHADOW
#  include "core-shadow/CoreShadow.hh"
#endif

#include "core/ICore.hh"
#include "core/IApp.hh"
#include "ui/UiTypes.hh"

using namespace workrave;

namespace
{
  auto bool_text(bool value) -> const char *
  {
    return value ? "true" : "false";
  }

  auto break_name(workrave::BreakId break_id) -> const char *
  {
    switch (break_id)
      {
      case BREAK_ID_MICRO_BREAK:
        return "micro-break";
      case BREAK_ID_REST_BREAK:
        return "rest-break";
      case BREAK_ID_DAILY_LIMIT:
        return "daily-limit";
      default:
        return "unknown";
      }
  }
} // namespace

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

  auto *state_group = new QGroupBox(tr("Timer and break state"));
  auto *state_layout = new QVBoxLayout(state_group);
  state_view = new QTextBrowser();
  state_view->setOpenExternalLinks(false);
  state_view->setMinimumSize(1100, 500);
  state_layout->addWidget(state_view);
  vbox->addWidget(state_group);

  refresh_timer = new QTimer(this);
  connect(refresh_timer, &QTimer::timeout, this, &DebugDialog::refresh_state);
  refresh_timer->start(1000);
  refresh_state();

  adjustSize();
}

DebugDialog::~DebugDialog()
{
  stop_all();
}

void
DebugDialog::refresh_state()
{
  QString state_html;

#ifdef HAVE_CORE_SHADOW
  auto core = app->get_core();
  auto *shadow_debug = dynamic_cast<workrave::core_shadow::ICoreShadowDebug *>(core.get());
  if (shadow_debug != nullptr)
    {
      state_html = QString::fromStdString(shadow_debug->get_shadow_debug_state_html());
    }
#endif

  if (state_html.isEmpty())
    {
      state_html = active_core_debug_state_html();
    }

  const auto scroll_position = state_view->verticalScrollBar()->value();
  state_view->setHtml(state_html);
  state_view->verticalScrollBar()->setValue(scroll_position);
}

QString
DebugDialog::active_core_debug_state_html() const
{
  std::ostringstream out;
  auto core = app->get_core();

  out << "<h3>Active core state</h3>";
  out << "<p>user-active=" << bool_text(core->is_user_active()) << "</p>";
  out << "<table cellspacing=\"0\" cellpadding=\"4\" border=\"1\">"
      << "<tr bgcolor=\"#e8e8e8\"><th align=\"left\">break</th><th>elapsed</th><th>idle</th><th>limit</th>"
      << "<th>auto-reset</th><th>enabled</th><th>running</th><th>taking</th><th>active</th></tr>";

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      auto id = static_cast<BreakId>(i);
      auto break_state = core->get_break(id);
      if (!break_state)
        {
          continue;
        }

      out << "<tr><td>" << break_name(id) << "</td><td align=\"right\">" << break_state->get_elapsed_time()
          << "</td><td align=\"right\">" << break_state->get_elapsed_idle_time() << "</td><td align=\"right\">"
          << break_state->get_limit() << "</td><td align=\"right\">" << break_state->get_auto_reset()
          << "</td><td>" << bool_text(break_state->is_enabled()) << "</td><td>"
          << bool_text(break_state->is_running()) << "</td><td>" << bool_text(break_state->is_taking())
          << "</td><td>" << bool_text(break_state->is_active()) << "</td></tr>";
    }

  out << "</table>";
  return QString::fromStdString(out.str());
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
