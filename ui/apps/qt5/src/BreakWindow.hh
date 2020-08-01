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

#include "commonui/GUIConfig.hh"
#include "commonui/UiTypes.hh"
#include "core/ICore.hh"

#include "Frame.hh"
#include "IBreakWindow.hh"

class BreakWindow :
  public QWidget,
  public IBreakWindow
{
  Q_OBJECT

public:
  BreakWindow(int screen,
              workrave::BreakId break_id,
              BreakFlags break_flags,
              GUIConfig::BlockMode block_mode);
  ~BreakWindow() override;

  void init() override;
  void start() override;
  void stop() override;
  void refresh() override;

protected:
  BreakFlags get_break_flags() const { return break_flags; }
  int get_screen() { return screen; }
  void center();

  void add_skip_button(QLayout *box);
  void add_postpone_button(QLayout *box);
  void add_lock_button(QLayout *box);
  void add_shutdown_button(QLayout *box);
  
private:
  virtual QWidget *create_gui() = 0;
  virtual void update_break_window();

  QHBoxLayout *create_break_buttons(bool lockable, bool shutdownable);

private:
  void resume_non_ignorable_break();
  void on_lock_button_clicked();
  void on_shutdown_button_clicked();
  void on_skip_button_clicked();
  //  bool on_delete_event(GdkEventAny *);
  void on_postpone_button_clicked();

private:
  workrave::BreakId break_id;
  int screen;
  BreakFlags break_flags;
  GUIConfig::BlockMode block_mode;
  bool is_flashing = false;
  Frame *frame { nullptr };
  QWidget *gui { nullptr };
  QWidget *block_window { nullptr };

#ifdef PLATFORM_OS_OSX
  class Private;
  std::shared_ptr<Private> priv;
#endif
};

#endif // BREAKWINDOW_HH
