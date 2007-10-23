// IApp.hh
//
// Copyright (C) 2001 - 2007 Rob Caelers <robc@krandor.nl>
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
// $Id$
//

#ifndef IAPP_HH
#define IAPP_HH

#include "ICore.hh"

namespace workrave
{
  // Forward declarion of external interfaces.
  class IBreakResponse;

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
        STAGE_ALERT,
      };

    //! Text that the GUI show must in the prelude window.
    enum PreludeProgressText
      {
        PROGRESS_TEXT_BREAK_IN,
        PROGRESS_TEXT_DISAPPEARS_IN,
        PROGRESS_TEXT_SILENT_IN,
      };

    virtual ~IApp() {}

    //! Set the response interface that must the used by the GUI to respond.
    virtual void set_break_response(IBreakResponse *rep) = 0;

    //! Show a prelude window for specified break type.
    virtual void start_prelude_window(BreakId break_id) = 0;

    //! Show a break window for specified break type.
    virtual void start_break_window(BreakId break_id, bool ignorable) = 0;

    //! Hide the break or prelude window.
    virtual void hide_break_window() = 0;

    //! Refreshe the content of the break or prelude window.
    virtual void refresh_break_window() = 0;

    //! Set the break progress to the specified value and maximum value.
    virtual void set_break_progress(int value, int max_value) = 0;

    //! Set the alert stage of the prelude window.
    virtual void set_prelude_stage(PreludeStage stage) = 0;

    //! Set the progress text of the prelude window.
    virtual void set_prelude_progress_text(PreludeProgressText text) = 0;

    //! Terminate the application.
    virtual void terminate() = 0;
  };
}

#endif // IAPP_HH
