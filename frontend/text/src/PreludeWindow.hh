// PreludeWindow.hh --- window for the microbreak
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

#ifndef PRELUDEWINDOW_HH
#define PRELUDEWINDOW_HH

#include "IPreludeWindow.hh"

class Dispatcher;

using namespace workrave;

class PreludeWindow : public IPreludeWindow
{
public:
  PreludeWindow(BreakId break_id);
  virtual ~PreludeWindow();

  void start();
  void stop();
  void destroy();
  void refresh();
  void set_progress(int value, int max_value);
  void set_stage(IApp::PreludeStage stage);
  void set_progress_text(IApp::PreludeProgressText text);
  void set_response(IBreakResponse *pri);

private:
  //!
  BreakId break_id;

  //! Final prelude
  std::string progress_text;

  //! Progress values
  int progress_value;
  int progress_max_value;

  //! Send response to this interface.
  IBreakResponse *prelude_response;
};

inline void
PreludeWindow::set_response(IBreakResponse *pri)
{
  prelude_response = pri;
}

#endif // PRELUDEWINDOW_HH
