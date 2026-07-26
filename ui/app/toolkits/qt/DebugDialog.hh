// Copyright (C) 2015 Rob Caelers <robc@krandor.org>
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

#ifndef DEBUGDIALOG_HH
#define DEBUGDIALOG_HH

#include <QtGui>
#include <QtWidgets>

#include "ui/IApplicationContext.hh"
#include "ui/IBreakWindow.hh"
#include "ui/IPreludeWindow.hh"

class QPushButton;
class QTextBrowser;
class QTimer;

class DebugDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DebugDialog(std::shared_ptr<IApplicationContext> app);
  ~DebugDialog() override;

private:
  void toggle_break(workrave::BreakId break_id, IBreakWindow::Ptr &window, QPushButton *button);
  void toggle_prelude(workrave::BreakId break_id, IPreludeWindow::Ptr &window, QPushButton *button);
  void stop_all();
  void refresh_state();
  [[nodiscard]] QString active_core_debug_state_html() const;

  std::shared_ptr<IApplicationContext> app;

  IBreakWindow::Ptr micro_window;
  IBreakWindow::Ptr rest_window;
  IBreakWindow::Ptr daily_window;
  IPreludeWindow::Ptr prelude_window;

  QPushButton *btn_micro{nullptr};
  QPushButton *btn_rest{nullptr};
  QPushButton *btn_daily{nullptr};
  QPushButton *btn_prelude{nullptr};
  QTextBrowser *state_view{nullptr};
  QTimer *refresh_timer{nullptr};
};

#endif // DEBUGDIALOG_HH
