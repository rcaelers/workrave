// PreludeWindow.hh --- window for the microbreak
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005 Rob Caelers & Raymond Penners
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
// $Id$
//

#ifndef PRELUDEWINDOW_HH
#define PRELUDEWINDOW_HH

#include "PreludeWindowInterface.hh"

class Dispatcher;

class PreludeWindow :
  public PreludeWindowInterface
{
public:
  PreludeWindow(BreakId break_id);
  virtual ~PreludeWindow();

  void start();
  void stop();
  void destroy();
  void refresh();
  void set_progress(int value, int max_value);
  void set_stage(AppInterface::PreludeStage stage);
  void set_progress_text(AppInterface::PreludeProgressText text);
  void set_response(BreakResponseInterface *pri);
  
private:
  //!
  BreakId break_id;
  
  //! Final prelude
  string progress_text;

  //! Progress values
  int progress_value;
  int progress_max_value;

  //! Send response to this interface.
  BreakResponseInterface *prelude_response;
};


inline void
PreludeWindow::set_response(BreakResponseInterface *pri)
{
  prelude_response = pri;
}

#endif // PRELUDEWINDOW_HH
