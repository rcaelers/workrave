// IBreakWindow.hh --- base class for the break windows
//
// Copyright (C) 2001, 2002, 2003, 2005, 2006 Rob Caelers <robc@krandor.org>
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

#ifndef IBREAKWINDOW_HH
#define IBREAKWINDOW_HH

#include <stdio.h>

class IBreakResponse;

class IBreakWindow
{
public:
  virtual ~IBreakWindow() {}

  //! Starts (i.e. shows) the break window.
  virtual void start() = 0;

  //! Stops (i.e. hides) the break window.
  virtual void stop() = 0;

  //! Refreshes the content of the break window.
  virtual void refresh() = 0;

  //! Destroys the break window.
  /*! \warn this will 'delete' the window, so all pointers to the
   *        IBreakWindow will become invalid.
   */
  virtual void destroy() = 0;

  //! Sets the progress to the specified value and maximum value.
  virtual void set_progress(int value, int max_value) = 0;

  //! Sets the response callback.
  virtual void set_response(IBreakResponse *bri) = 0;

  //
  virtual Glib::RefPtr<Gdk::Window> get_gdk_window() = 0;
};

#endif // RESTBREAKWINDOW_HH
