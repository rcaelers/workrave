// Copyright (C) 2001 - 2011 Rob Caelers & Raymond Penners
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

#ifndef ICOREEVENTLISTENER_HH
#define ICOREEVENTLISTENER_HH

#include "core/CoreTypes.hh"
#include "utils/Enum.hh"

namespace workrave
{
  //! Events send from Core to GUI.
  enum CoreEvent
  {
    CORE_EVENT_NONE = -1,
    CORE_EVENT_MONITOR_FAILURE = 0,
    CORE_EVENT_SOUND_FIRST = 1,
    CORE_EVENT_SOUND_BREAK_PRELUDE = CORE_EVENT_SOUND_FIRST,
    CORE_EVENT_SOUND_BREAK_IGNORED,
    CORE_EVENT_SOUND_REST_BREAK_STARTED,
    CORE_EVENT_SOUND_REST_BREAK_ENDED,
    CORE_EVENT_SOUND_MICRO_BREAK_STARTED,
    CORE_EVENT_SOUND_MICRO_BREAK_ENDED,
    CORE_EVENT_SOUND_DAILY_LIMIT,
    CORE_EVENT_SOUND_LAST = CORE_EVENT_SOUND_DAILY_LIMIT,
  };

  template<>
  struct workrave::utils::enum_traits<CoreEvent>
  {
    static constexpr std::array<std::pair<std::string_view, CoreEvent>, 9> names = {
      {{"none", CORE_EVENT_NONE},
       {"monitor-failure", CORE_EVENT_MONITOR_FAILURE},
       {"break-prelude", CORE_EVENT_SOUND_BREAK_PRELUDE},
       {"break-ignored", CORE_EVENT_SOUND_BREAK_IGNORED},
       {"rest-break-started", CORE_EVENT_SOUND_REST_BREAK_STARTED},
       {"rest-break-ended", CORE_EVENT_SOUND_REST_BREAK_ENDED},
       {"micro-break-started", CORE_EVENT_SOUND_MICRO_BREAK_STARTED},
       {"micro-break-ended", CORE_EVENT_SOUND_MICRO_BREAK_ENDED},
       {"daily-limit", CORE_EVENT_SOUND_DAILY_LIMIT}}};
  };

  //! Listener for events comming from the Core.
  class ICoreEventListener
  {
  public:
    virtual ~ICoreEventListener() = default;

    // Notification of a core event.
    virtual void core_event_notify(const CoreEvent event) = 0;
  };
} // namespace workrave

#endif // ICOREEVENTLISTENER_HH
