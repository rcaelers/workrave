// LinkedActivityMonitor.hh
//
// Copyright (C) 2007, 2008, 2009 Rob Caelers <robc@krandor.nl>
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
// $Id: LinkedActivityMonitor.hh 1090 2006-10-01 20:49:47Z dotsphinx $
//

#ifndef LINKEDACTIVITYMONITOR_HH
#define LINKEDACTIVITYMONITOR_HH

#include <map>

#include "IActivityMonitor.hh"
#include "ILinkEventListener.hh"

#include "WRID.hh"

using namespace workrave;

// Forward declarion of external interface.
namespace workrave {
  class ICore;
  class LinkEvent;
}

class LinkedActivityMonitor
  : public ILinkEventListener
{
private:

  struct RemoteState
  {
    WRID id;
    ActivityState state;
    time_t lastupdate;
    time_t since;
  };

  // Known states
  typedef std::map<WRID, RemoteState> States;
  typedef States::iterator StateIter;
  typedef States::const_iterator StateCIter;

public:
  LinkedActivityMonitor();
  virtual ~LinkedActivityMonitor();

  //! Initializes the monitor
  void init();

  // ILinkEventListener
  void event_received(LinkEvent *event);

  // Internal
  void report_active(bool active);
  bool get_active();

  bool is_active(const WRID &remote, time_t &since);
  
private:
  void handle_activity_event(LinkEvent *event);
  void handle_presense_event(LinkEvent *event);

private:
  //! The main core
  ICore *core;

  //! Monitor suspended?
  bool suspended;

  //! Current state
  ActivityState state;

  //! Remote states
  States states;
};

#endif // LINKEDACTIVITYMONITOR_HH
