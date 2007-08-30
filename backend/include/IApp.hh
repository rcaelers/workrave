// IApp.hh
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Rob Caelers <robc@krandor.org>
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

#ifndef IAPP_HH
#define IAPP_HH

#include "ICore.hh"

class IBreakResponse;

class IApp
{
public:
  enum PreludeStage
    {
      STAGE_INITIAL = 0,
      STAGE_MOVE_OUT,
      STAGE_WARN,
      STAGE_ALERT,
    };

  enum PreludeProgressText
    {
      PROGRESS_TEXT_BREAK_IN,
      PROGRESS_TEXT_DISAPPEARS_IN,
      PROGRESS_TEXT_SILENT_IN,
    };

  virtual ~IApp() {}

  //! Sets the response interface.
  virtual void set_break_response(IBreakResponse *rep) = 0;

  //! Shows a prelude window for specified break type.
  virtual void start_prelude_window(BreakId break_id) = 0;

  //! Shows a break window for specified break type.
  virtual void start_break_window(BreakId break_id, bool ignorable) = 0;

  //! Hides the break or prelude window.
  virtual void hide_break_window() = 0;

  //! Refreshes the content of the break or prelude window.
  virtual void refresh_break_window() = 0;

  //! Sets the progress to the specified value and maximum value.
  virtual void set_break_progress(int value, int max_value) = 0;

  //! Sets the alert stage of the prelude window.
  virtual void set_prelude_stage(PreludeStage stage) = 0;

  //! Sets the progress text of the prelude window.
  virtual void set_prelude_progress_text(PreludeProgressText text) = 0;
};

#endif // IAPP_HH
