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

#ifndef BREAKWINDOW_HH
#define BREAKWINDOW_HH

#include <QtGui>
#include <QtWidgets>

#include <memory>

#include "ui/GUIConfig.hh"
#include "ui/UiTypes.hh"
#include "core/CoreTypes.hh"
#include "session/System.hh"

#include "ui/IBreakWindow.hh"
#include "Frame.hh"
#include "SizeGroup.hh"
#include "ui/IApplicationContext.hh"

class BreakWindow
  : public QWidget
  , public IBreakWindow
{
  Q_OBJECT

public:
  BreakWindow(std::shared_ptr<IApplicationContext> app, QScreen *screen, workrave::BreakId break_id, BreakFlags break_flags);
  ~BreakWindow() override;

  void init() override;
  void start() override;
  void stop() override;
  void refresh() override;

protected:
  BreakFlags get_break_flags() const
  {
    return break_flags;
  }
  QScreen *get_screen()
  {
    return screen;
  }
  void center();

  void add_skip_button(QGridLayout *box, bool locked);
  void add_postpone_button(QGridLayout *box, bool locked);
  void add_lock_button(QGridLayout *box) const;
  void add_sysoper_combobox(QGridLayout *box);

private:
  virtual QWidget *create_gui() = 0;
  virtual void update_break_window();

  void update_skip_postpone_lock();
  void update_flashing_border();

  QLayout *create_break_buttons(bool lockable, bool shutdownable);

  void check_skip_postpone_lock(bool &skip_locked, bool &postpone_locked, workrave::BreakId &overdue_break_id);

  void on_lock_button_clicked();
  void on_skip_button_clicked();
  void on_postpone_button_clicked();
  void on_sysoper_combobox_changed(int index);

  std::vector<System::SystemOperation> supported_system_operations;
  void append_row_to_sysoper_model(System::SystemOperation::SystemOperationType type);
  void get_operation_name_and_icon(System::SystemOperation::SystemOperationType type, QString &name, QString &icon_name);

private:
  IApplicationContext::Ptr app;
  workrave::BreakId break_id;
  BreakFlags break_flags;
  BlockMode block_mode;
  bool is_flashing = false;
  QScreen *screen{nullptr};
  Frame *frame{nullptr};
  QWidget *gui{nullptr};
  QWidget *block_window{nullptr};
  QProgressBar *progress_bar{nullptr};
  QPushButton *postpone_button{nullptr};
  QPushButton *skip_button{nullptr};
  QComboBox *sysoper_combo{nullptr};
  std::shared_ptr<SizeGroup> size_group;
};

#endif // BREAKWINDOW_HH
