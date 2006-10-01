// GUI.hh
//
// Copyright (C) 2006 Raymond Penners <raymond@dotsphinx.com>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id: IniConfigurator.hh 558 2006-02-23 19:42:12Z rcaelers $
//

#ifndef GUI_HH
#define GUI_HH

#include <QApplication>
#include <QTimer>

#include "ICore.hh"
#include "IApp.hh"
#include "StatusWindow.hh"

class GUI : public QApplication, IApp
{
  Q_OBJECT
  
public:
  GUI(int argc, char *argv[]);
  ~GUI();

public slots:
  void on_timer();

  virtual void set_break_response(IBreakResponse *rep);
  virtual void start_prelude_window(BreakId break_id);
  virtual void start_break_window(BreakId break_id, bool ignorable);
  virtual void hide_break_window();
  virtual void refresh_break_window();
  virtual void set_break_progress(int value, int max_value);
  virtual void set_prelude_stage(PreludeStage stage);
  virtual void set_prelude_progress_text(PreludeProgressText text);
  
private:
  QTimer *heartbeat_timer;
  ICore *core;
  StatusWindow status_window;
};

#endif // GUI_HH
