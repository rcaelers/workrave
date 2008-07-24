// ConfigurationLinkEvent.hh --- Interface definition for a Workrave link event
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

#ifndef CONFIGURATIONLINKEVENT_HH
#define CONFIGURATIONLINKEVENT_HH

#include <string>
#include <map>

#include "LinkEvent.hh"

using namespace workrave;

//! Link Event
class ConfigurationLinkEvent : public LinkEvent
{
private:

public:
  enum Reason
    {
      INITIAL,
      USER,
      RESOLVED
    };

  typedef std::map<std::string, std::string> ConfigChanges;
  typedef ConfigChanges::const_iterator ConfigChangeCIter;

public:
  ConfigurationLinkEvent();
  virtual ~ConfigurationLinkEvent();

  virtual std::string str() const;
  virtual std::string class_name() const;
  virtual void serialize(workrave::serialization::Target *s);

  void add_change(std::string key, std::string value);

  void set_reason(Reason reason);
  Reason get_reason() const;

  ConfigChanges get_changes() const;

private:
  ConfigChanges changes;

  Reason reason;
};

#endif // CONFIGURATIONLINKEVENT_HH
