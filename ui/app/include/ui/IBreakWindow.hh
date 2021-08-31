// Copyright (C) 2001 - 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_UI_IBREAKWINDOW_HH
#define WORKRAVE_UI_IBREAKWINDOW_HH

#include <memory>

namespace workrave
{
  class IBreakResponse;
}

class IBreakWindow
{
public:
  using Ptr = std::shared_ptr<IBreakWindow>;
  virtual ~IBreakWindow() = default;

  //! Initializes the break window.
  virtual void init() = 0;

  //! Starts (i.e. shows) the break window.
  virtual void start() = 0;

  //! Stops (i.e. hides) the break window.
  virtual void stop() = 0;

  //! Refreshes the content of the break window.
  virtual void refresh() = 0;

  //! Sets the progress to the specified value and maximum value.
  virtual void set_progress(int value, int max_value) = 0;
};

#endif // WORKRAVE_UI_IBREAKWINDOW_HH
