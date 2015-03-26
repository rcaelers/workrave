// BreakWindow.hh --- base class for the break windows
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

#ifndef BREAKWINDOW_HH
#define BREAKWINDOW_HH

#include <QtGui>
#include <QtWidgets>

#include <boost/shared_ptr.hpp>

#include "ICore.hh"
#include "IBreakWindow.hh"
#include "UiTypes.hh"
#include "GUIConfig.hh"

#include "Frame.hh"

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
  virtual ~BreakWindow();

  virtual void init();
  virtual void start();
  virtual void stop();
  virtual void refresh();

protected:
  virtual QWidget *create_gui() = 0;
  virtual void update_break_window();

  QHBoxLayout *create_break_buttons(bool lockable, bool shutdownable);
  QAbstractButton *create_skip_button();
  QAbstractButton *create_postpone_button();
  QAbstractButton *create_lock_button();
  QAbstractButton *create_shutdown_button();

  BreakFlags get_break_flags() const { return break_flags; }
  Frame* get_frame() { return frame; }
  int get_screen() { return screen; }

  void center();

private:
  void resume_non_ignorable_break();
  void on_lock_button_clicked();
  void on_shutdown_button_clicked();
  void on_skip_button_clicked();
  //  bool on_delete_event(GdkEventAny *);
  void on_postpone_button_clicked();


private:
  //! Break ID
  workrave::BreakId break_id;

  //! Information about the (multi)head.
  int screen;

  //! Insist
  GUIConfig::BlockMode block_mode;

  //! Ignorable
  BreakFlags break_flags;

  //! Flash frame
  Frame *frame;

  //! Is currently flashing because user is active?
  bool is_flashing;

  //! GUI
  QWidget *gui;

  QAbstractButton *postpone_button;
  QAbstractButton *skip_button;
  QAbstractButton *lock_button;
  QAbstractButton *shutdown_button;

  QWidget *block_window;

#ifdef PLATFORM_OS_OSX  
  class Private;
  boost::shared_ptr<Private> priv;
#endif
};

#endif // BREAKWINDOW_HH
