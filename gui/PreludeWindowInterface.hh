// PreludeWindowInterface.hh --- base class for the break windows
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
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

#ifndef PRELUDEWINDOWINTERFACE_HH
#define PRELUDEWINDOWINTERFACE_HH

#include <string>

class PreludeResponseInterface;

class PreludeWindowInterface
{
public:
  PreludeWindowInterface()
  {
    prelude_response = NULL;
  }

  enum Stage
  {
    STAGE_INITIAL = 0,
    STAGE_MOVE_OUT,
    STAGE_WARN,
    STAGE_ALERT,
  };

  //! Starts (i.e. shows) the prelude window.
  virtual void start() = 0;
  
  //! Stops (i.e. hides) the prelude window.
  virtual void stop() = 0;

  //! Sets the prelude window on hold, returns true if supported.
  virtual bool delayed_stop() = 0;

  //! Destroys the prelude window.
  /*! \warn this will 'delete' the window.
   */
  virtual void destroy() = 0;

  //! Refreshes the content of the prelude window.
  virtual void refresh() = 0;
  
  //! Sets the progress to the specified value and maximum value.
  virtual void set_progress(int value, int max_value) = 0;

  //! Sets the additional text displayed by the prelude window.
  virtual void set_text(string text) = 0;

  //! Sets the alert stage of the prelude window.
  virtual void set_stage(Stage stage) = 0;

  //! Sets the progress text of the prelude window.
  virtual void set_progress_text(string text) = 0;

  //! Sets the response callback.
  virtual void set_prelude_response(PreludeResponseInterface *pri)
  {
    // FIXME: this does not belong in an interface.
    prelude_response = pri;
  }

protected:
  //! Send response to this interface.
  PreludeResponseInterface *prelude_response;
};

#endif // RESTPRELUDEWINDOWINTERFACE_HH
