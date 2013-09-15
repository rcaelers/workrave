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

#ifndef BREAKDBUS_HH
#define BREAKDBUS_HH

#include <boost/shared_ptr.hpp>

#include "config/Config.hh"
#include "dbus/IDBus.hh"

#include "BreakStateModel.hh"

using namespace workrave;

class BreakDBus
{
public:
  typedef boost::shared_ptr<BreakDBus> Ptr;

public:
  static Ptr create(BreakId break_id, BreakStateModel::Ptr break_state_model, workrave::dbus::IDBus::Ptr dbus);

  BreakDBus(BreakId break_id, BreakStateModel::Ptr break_state_model, workrave::dbus::IDBus::Ptr dbus);
  virtual ~BreakDBus();

private:
  void on_break_stage_changed(BreakStage stage);
  
private:
  BreakId break_id;
  BreakStateModel::Ptr break_state_model;
  workrave::dbus::IDBus::Ptr dbus;
};

#endif // BREAKDBUS_HH
