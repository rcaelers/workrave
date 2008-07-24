// ResolveConfigurationLinkEvent.cc
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
#include "ResolveConfigurationLinkEvent.hh"
#include "Serializer.hh"

using namespace std;

REGISTER_TYPE(LinkEvent, ResolveConfigurationLinkEvent, "resolveconfigurationlinkevent");


ResolveConfigurationLinkEvent::ResolveConfigurationLinkEvent()
{
}


ResolveConfigurationLinkEvent::~ResolveConfigurationLinkEvent()
{
}


string
ResolveConfigurationLinkEvent::str() const
{
  stringstream ss;

  ss << "[ResolveConfigurationLinkEvent "
     << LinkEvent::str() << " "
     << "changes: { ";

  for (ConfigChangeCIter i = changes.begin(); i != changes.end(); i++)
    {
      ss << i->first << " : " << i->second << ", ";
    }

  ss << " } ]";

  return ss.str();
}

void
ResolveConfigurationLinkEvent::serialize(workrave::serialization::Target *s)
{
  LinkEvent::serialize(s);

  (*s) & workrave::serialization::attr("changes", changes);
}


void
ResolveConfigurationLinkEvent::add(const std::string &key, const std::string &value)
{
  changes[key] = value;
}

ResolveConfigurationLinkEvent::ConfigChanges
ResolveConfigurationLinkEvent::get_changes() const
{
  return changes;
}
