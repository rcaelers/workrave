// Copyright (C) 2001 - 2014 Rob Caelers & Raymond Penners
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

#include <memory>

#include "dbus/IDBus.hh"
#include "utils/Signals.hh"

#include "BreakStateModel.hh"

class BreakDBus : public workrave::utils::Trackable
{
public:
  using Ptr = std::shared_ptr<BreakDBus>;

public:
  BreakDBus(workrave::BreakId break_id, BreakStateModel::Ptr break_state_model, workrave::dbus::IDBus::Ptr dbus);
  virtual ~BreakDBus() = default;

private:
  void on_break_stage_changed(BreakStage stage);
  void on_break_event(workrave::BreakEvent event);

private:
  workrave::BreakId break_id;
  BreakStateModel::Ptr break_state_model;
  workrave::dbus::IDBus::Ptr dbus;
};

#endif // BREAKDBUS_HH
