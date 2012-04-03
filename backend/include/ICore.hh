// ICore.hh --- The main controller interface
//
// Copyright (C) 2001 - 2009, 2011 Rob Caelers <robc@krandor.nl>
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

#ifndef ICORE_HH
#define ICORE_HH

#include <string>

#include "enum.h"

namespace workrave {

  // Forward declaratons
  class IBreak;
  class IApp;
  class IStatistics;
  class ICoreEventListener;
  class INetwork;
  class IDistributionManager;

  //! ID of a break.
  enum BreakId
    {
      BREAK_ID_NONE = -1,
      BREAK_ID_MICRO_BREAK = 0,
      BREAK_ID_REST_BREAK,
      BREAK_ID_DAILY_LIMIT,
      BREAK_ID_SIZEOF
    };

    enum BreakHint
      {
        BREAK_HINT_NONE = 0,

        // Break was started on user request
        BREAK_HINT_USER_INITIATED = 1,

        // Natural break.
        BREAK_HINT_NATURAL_BREAK = 2,

      };


  //! Main interface of the backend.
  class ICore
  {
  public:
    virtual ~ICore() {}

    //! The way a break is insisted.
    enum InsistPolicy
      {
        //! Uninitialized policy
        INSIST_POLICY_INVALID,

        //! Halts the timer on activity.
        INSIST_POLICY_HALT,

        //! Resets the timer on activity.
        INSIST_POLICY_RESET,

        //! Ignores all activity.
        INSIST_POLICY_IGNORE,

        //! Number of policies.
        INSIST_POLICY_SIZEOF
      };

    //! Initialize the Core. Must be called first.
    virtual void init(int argc, char **argv, IApp *app, const std::string &display) = 0;

    //! Periodic heartbeat. The GUI *MUST* call this method every second.
    virtual void heartbeat() = 0;

    //! Force a break of the specified type.
    virtual void force_break(BreakId id, BreakHint break_hint) = 0;

    //! Return the break interface of the specified type.
    virtual IBreak *get_break(BreakId id) = 0;

    //! Return the break interface of the specified type.
    virtual IBreak *get_break(std::string name) = 0;

    //! Return the statistics interface.
    virtual IStatistics *get_statistics() const = 0;

#ifdef HAVE_DISTRIBUTION
    //! Returns the distribution manager (if available).
    virtual IDistributionManager *get_distribution_manager() const = 0;
#endif

    //! Is the user currently active?
    virtual bool is_user_active() const = 0;

    //! Retrieves the operation mode.
    virtual OperationMode get_operation_mode() = 0;

    //! Retrieves the regular operation mode.
    virtual OperationMode get_operation_mode_regular() = 0;

    //! Checks if operation_mode is an override.
    virtual bool is_operation_mode_an_override() = 0;

    //! Sets the operation mode.
    virtual void set_operation_mode(OperationMode mode) = 0;

    //! Temporarily overrides the operation mode.
    virtual void set_operation_mode_override( OperationMode mode, const std::string &id ) = 0;

    //! Removes the overriden operation mode.
    virtual void remove_operation_mode_override( const std::string &id ) = 0;

    //! Return the current usage mode.
    virtual UsageMode get_usage_mode() = 0;

    //! Set the usage mode.
    virtual void set_usage_mode(UsageMode mode) = 0;

    //! Set the callback for activity monitor events.
    virtual void set_core_events_listener(ICoreEventListener *l) = 0;

    //! Notify the core that the computer will enter or leave powersave (suspend/hibernate)
    virtual void set_powersave(bool down) = 0;

    //! Notify the core that the computer time has changed
    virtual void time_changed() = 0;

    //! Set the break insist policy.
    virtual void set_insist_policy(InsistPolicy p) = 0;

    //! Return the current time
    virtual time_t get_time() const = 0;

    //! Return the current time
    virtual void force_idle() = 0;
  };

  std::string operator%(const std::string &key, BreakId id);
};

#endif // ICORE_HH
