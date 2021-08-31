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

#ifndef WORKRAVE_UI_IPRELUDEWINDOW_HH
#define WORKRAVE_UI_IPRELUDEWINDOW_HH

#include <memory>

#include "core/IApp.hh"

class IPreludeWindow
{
public:
  virtual ~IPreludeWindow() = default;

  using Ptr = std::shared_ptr<IPreludeWindow>;

  //! Starts (i.e. shows) the prelude window.
  virtual void start() = 0;

  //! Stops (i.e. hides) the prelude window.
  virtual void stop() = 0;

  //! Refreshes the content of the prelude window.
  virtual void refresh() = 0;

  //! Sets the progress to the specified value and maximum value.
  virtual void set_progress(int value, int max_value) = 0;

  //! Sets the alert stage of the prelude window.
  virtual void set_stage(workrave::IApp::PreludeStage stage) = 0;

  //! Sets the progress text of the prelude window.
  virtual void set_progress_text(workrave::IApp::PreludeProgressText text) = 0;
};

#endif // WORKRAVE_UI_IPRELUDEWINDOW_HH
