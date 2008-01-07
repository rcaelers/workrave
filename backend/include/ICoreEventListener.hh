// ICoreEventListener.hh
//
// Copyright (C) 2001 - 2007 Rob Caelers & Raymond Penners
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
// $Id: ICoreEventListener.hh 1359 2007-10-22 19:18:06Z rcaelers $
//

#ifndef ICOREEVENTLISTENER_HH
#define ICOREEVENTLISTENER_HH

#include "ICore.hh"

namespace workrave
{
  //! Events send from Core to GUI.
  enum CoreEvent
    {
      CORE_EVENT_NONE = -1,
      CORE_EVENT_SOUND_BREAK_PRELUDE = 0,
      CORE_EVENT_SOUND_BREAK_IGNORED,
      CORE_EVENT_SOUND_REST_BREAK_STARTED,
      CORE_EVENT_SOUND_REST_BREAK_ENDED,
      CORE_EVENT_SOUND_MICRO_BREAK_STARTED,
      CORE_EVENT_SOUND_MICRO_BREAK_ENDED,
      CORE_EVENT_SOUND_DAILY_LIMIT,
    };

  //! Listener for events comming from the Core.
  class ICoreEventListener
  {
  public:
    virtual ~ICoreEventListener() {}

    // Notification of a core event.
    virtual void core_event_notify(const CoreEvent event) = 0;

    // Notification that the operation mode has changed..
    virtual void core_event_operation_mode_changed(const OperationMode m) = 0;
  };
}

#endif // ICOREEVENTLISTENER_HH
