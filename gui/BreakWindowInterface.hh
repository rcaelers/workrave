// BreakWindowInterface.hh --- base class for the break windows
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers <robc@krandor.org>
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

#ifndef BREAKWINDOWINTERFACE_HH
#define BREAKWINDOWINTERFACE_HH

#include <stdio.h>

class BreakResponseInterface;

class BreakWindowInterface
{
public:
  BreakWindowInterface()
  {
    break_response = NULL;
  }

  //! Starts (i.e. shows) the break window.
  virtual void start() = 0;

  //! Stops (i.e. hides) the break window.
  virtual void stop() = 0;

  //! Refreshes the content of the break window.
  virtual void refresh() = 0;

  //! Destroys the break window.
  /*! \warn this will 'delete' the window, so all pointers to the
   *        BreakWindowInterface will become invalid.
   */
  virtual void destroy() = 0;

  //! Sets the progress to the specified value and maximum value.
  virtual void set_progress(int value, int max_value) = 0;

  //! Sets the insist-break flags
  /*!
   *  A break that has 'insist' set, locks the keyboard during the break.
   */
  virtual void set_insist_break(bool insist) = 0;

  //! Sets the response callback.
  virtual void set_break_response(BreakResponseInterface *bri)
  {
    // FIXME: this does not belong in an interface.
    break_response = bri;
  }

protected:
  //! Send response to this interface.
  BreakResponseInterface *break_response;
};

#endif // RESTBREAKWINDOWINTERFACE_HH
