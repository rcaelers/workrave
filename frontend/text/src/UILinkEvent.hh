// UILinkEvent.hh --- An event of the Workrave core
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

#ifndef UILINKEVENT_HH
#define UILINKEVENT_HH

#include <string>

#include "LinkEvent.hh"
#include "ICore.hh"
#include "ISerializable.hh"

using namespace workrave;

//! Link Event
class UILinkEvent : public LinkEvent
{
public:
  enum UIEvent
    {
      UI_EVENT_NONE,
      UI_EVENT_TEST,
    };

public:
  UILinkEvent();
  UILinkEvent(UIEvent event);
  virtual ~UILinkEvent();

  virtual std::string str() const;
  virtual std::string class_name() const;
  virtual void serialize(workrave::serialization::Target *s);

  UIEvent get_ui_event() const
  {
    return ui_event;
  }

private:
  /*! Ui Event */
  UIEvent ui_event;
};

#endif // UILINKEVENT_HH
