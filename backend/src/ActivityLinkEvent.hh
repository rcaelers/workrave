// ActivityLinkEvent.hh
//
// Copyright (C) 2007 Rob Caelers <robc@krandor.nl>
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
// $Id$
//

#ifndef ACTIVITYLINKEVENT_HH
#define ACTIVITYLINKEVENT_HH

#include <string>
#include "LinkEvent.hh"
#include "IActivityMonitor.hh"

using namespace workrave;

//! Link Event reporting user activity
class ActivityLinkEvent : public LinkEvent
{
public:
  ActivityLinkEvent();
  ActivityLinkEvent(ActivityState state);
  virtual ~ActivityLinkEvent();

  // ISerializable
  virtual std::string str() const;
  virtual std::string class_name() const;
  virtual void serialize(workrave::serialization::Target *s);

  // Public
  ActivityState get_state() const;

private:
  //! Activity state of the user
  ActivityState state;
};

#endif // ACTIVITYLINKEVENT_HH
