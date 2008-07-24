// ResolveConfigurationLinkEvent.hh --- Request to resolve config conflict
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

#ifndef RESOLVECONFIGURATIONLINKEVENT_HH
#define RESOLVECONFIGURATIONLINKEVENT_HH

#include <string>
#include <map>

#include "LinkEvent.hh"

using namespace workrave;

namespace workrave
{
  //! Link Event indicating a configuration conflict.
  class ResolveConfigurationLinkEvent : public LinkEvent
  {
  public:
    typedef std::map<std::string, std::string> ConfigChanges;
    typedef ConfigChanges::const_iterator ConfigChangeCIter;

  public:
    ResolveConfigurationLinkEvent();
    virtual ~ResolveConfigurationLinkEvent();

    virtual std::string str() const;
    virtual std::string class_name() const;
    virtual void serialize(workrave::serialization::Target *s);

    void add(const std::string &key, const std::string &value);
    ConfigChanges get_changes() const;

  private:
    ConfigChanges changes;
  };
}

#endif // RESOLVECONFIGURATIONLINKEVENT_HH
