// UILinkEvent.cc
//
// Copyright (C) 2007 Rob Caelers
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

static const char rcsid[] = "$Id";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string>
#include <sstream>

#include "Factory.hh"
#include "UILinkEvent.hh"
#include "Serializer.hh"

REGISTER_TYPE(LinkEvent, UILinkEvent, "uilinkevent");

using namespace std;

UILinkEvent::UILinkEvent()
  : ui_event(UI_EVENT_NONE)
{
}


UILinkEvent::UILinkEvent(UIEvent event)
  : ui_event(event)
{
}


UILinkEvent::~UILinkEvent()
{
}


string
UILinkEvent::str() const
{
  stringstream ss;

  ss << "[UILinkEvent "
     << LinkEvent::str() << " "
     << " event: " << ui_event << " ]";

  return ss.str();
}

void
UILinkEvent::serialize(workrave::serialization::Target *s)
{
  LinkEvent::serialize(s);

  (*s) & workrave::serialization::attr("ui_event", (int &)ui_event);
}
