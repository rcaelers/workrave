// Workrave.hh
//
// Copyright (C) 2001, 2002, 2003, 2005, 2006, 2007, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_HH
#define WORKRAVE_HH

#include "IApp.hh"
#include "ICore.hh"
#include "Networking.hh"

using namespace workrave;

class Workrave : public IApp
{
public:
  typedef boost::shared_ptr<Workrave> Ptr;

public:
  static Ptr create();

  Workrave();
  virtual ~Workrave();

  void init();
  void heartbeat();
  void connect(const std::string host, int port);
  
  void create_prelude_window(BreakId break_id)  { }
  void create_break_window(BreakId break_id, BreakHint break_hint)  { }
  void hide_break_window()  { }
  void show_break_window()  { }
  void refresh_break_window()  { }
  void set_break_progress(int value, int max_value)  { }
  void set_prelude_stage(PreludeStage stage)  { }
  void set_prelude_progress_text(PreludeProgressText text)  { }
  void terminate()  { }
  
private:
  ICore::Ptr core;

  //! Current state
  Networking::Ptr networking;
};

#endif // WORKRAVE_HH
