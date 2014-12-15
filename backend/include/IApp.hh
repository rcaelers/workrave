// IApp.hh
//
// Copyright (C) 2001 - 2008, 2010, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_BACKEND_IAPP_HH
#define WORKRAVE_BACKEND_IAPP_HH

#include "ICore.hh"

namespace workrave
{
  //! Interface that must be implemented by GUI applications.
  class IApp
  {
  public:

    //! The stage of a break warning (prelude)
    enum PreludeStage
      {
        STAGE_INITIAL = 0,
        STAGE_MOVE_OUT,
        STAGE_WARN,
        STAGE_ALERT
      };

    //! Text that the GUI show must in the prelude window.
    enum PreludeProgressText
      {
        PROGRESS_TEXT_BREAK_IN,
        PROGRESS_TEXT_DISAPPEARS_IN,
        PROGRESS_TEXT_SILENT_IN
      };

    virtual ~IApp() {}

    //! Create a prelude window for specified break type.
    virtual void create_prelude_window(BreakId break_id) = 0;

    //! Create a break window for specified break type.
    virtual void create_break_window(BreakId break_id, BreakHint break_hint) = 0;

    //! Hide the break or prelude window.
    virtual void hide_break_window() = 0;

    //! Show the break or prelude window.
    virtual void show_break_window() = 0;

    //! Refresh the content of the break or prelude window.
    virtual void refresh_break_window() = 0;

    //! Set the break progress to the specified value and maximum value.
    virtual void set_break_progress(int value, int max_value) = 0;

    //! Set the alert stage of the prelude window.
    virtual void set_prelude_stage(PreludeStage stage) = 0;

    //! Set the progress text of the prelude window.
    virtual void set_prelude_progress_text(PreludeProgressText text) = 0;
  };
}

#endif // WORKRAVE_BACKEND_IAPP_HH
