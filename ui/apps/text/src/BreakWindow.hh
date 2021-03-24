// BreakWindow.hh --- base class for the break windows
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2007 Rob Caelers & Raymond Penners
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

#ifndef BREAKWINDOW_HH
#define BREAKWINDOW_HH

#include <stdio.h>

#include "preinclude.h"

#include "core/ICore.hh"
#include "IBreakWindow.hh"
#include "GUI.hh"

namespace workrave
{
  class IBreakResponse;
}

using namespace workrave;

class BreakWindow : public IBreakWindow
{
public:
  BreakWindow(BreakId break_id, bool ignorable, GUI::BlockMode block_mode);
  virtual ~BreakWindow();

  void set_response(IBreakResponse *bri);

  virtual void start();
  virtual void stop();
  virtual void destroy();
  virtual void refresh();
  virtual void set_progress(int value, int max_value);

protected:
  //! Insist
  GUI::BlockMode block_mode;

  //! Ignorable
  bool ignorable_break;

private:
  //! Send response to this interface.
  IBreakResponse *break_response;

  //! Break ID
  BreakId break_id;

  //! Progress
  int progress_value;

  //! Progress
  int progress_max_value;
};

#endif // BREAKWINDOW_HH
