// IPreludeWindow.hh --- base class for the break windows
//
// Copyright (C) 2001 -2013 Rob Caelers <robc@krandor.nl>
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

#ifndef IPRELUDEWINDOW_HH
#define IPRELUDEWINDOW_HH

#include <boost/shared_ptr.hpp>

#include "IApp.hh"

class IPreludeWindow
{
public:
  virtual ~IPreludeWindow() {}
  
  typedef boost::shared_ptr<IPreludeWindow> Ptr;
  
  //! Starts (i.e. shows) the break window.
  virtual void start() = 0;
  
  //! Stops (i.e. hides) the break window.
  virtual void stop() = 0;

  //! Refreshes the content of the break window.
  virtual void refresh() = 0;

  //! Sets the progress to the specified value and maximum value.
  virtual void set_progress(int value, int max_value) = 0;

  //!
  virtual void set_stage(workrave::IApp::PreludeStage stage) = 0;

  //!
  virtual void set_progress_text(workrave::IApp::PreludeProgressText text) = 0;
  
};


#endif // IPRELUDEWINDOW_HH
