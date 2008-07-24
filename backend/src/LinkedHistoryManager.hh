// LinkedHistoryManager.hh
//
// Copyright (C) 2007, 2008 Rob Caelers <robc@krandor.nl>
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
// $Id: LinkedHistoryManager.hh 1090 2006-10-01 20:49:47Z dotsphinx $
//

#ifndef LINKEDHISTORYMANAGER_HH
#define LINKEDHISTORYMANAGER_HH

#include <glib.h>
#include <list>
#include <deque>

#include "ICore.hh"

#include "IConfiguratorListener.hh"
#include "ILinkEventListener.hh"

class org_workrave_DebugInterface_Stub;
class LinkRouter;
class ByteStream;
class TimePred;

using namespace workrave;

class LinkedHistoryManager
  : public ILinkEventListener,
    public IConfiguratorListener
{
public:
  LinkedHistoryManager(LinkRouter *router);
  virtual ~LinkedHistoryManager();

  //! Initializes the manager
  void init();

  //! Terminates the manager
  void terminate();

  //! Period heartbeat
  void heartbeat();

  //! Report change in user activity state
  void report_user_activity(bool active);

  //! Report timer running state
  void report_timer_activity(int id, bool running);

private:
  //! Bitmask indicating activity
  /*! bit 0     : activity monitor state
   *  bit 1 - 3 : micro, rest, daily timers
   */
  typedef guint8 ActivityMask;
  const static int ACTIVITY_MASK_SIZE = 4;
  
  //! Activity state at certain point in time.
  struct ActivityEntry
  {
    bool operator<(const ActivityEntry &rhs)
    {
      return timestamp <  rhs.timestamp;
    }

    //! Timestamp of entry
    time_t timestamp;

    //! Bitmask indicating activity
    ActivityMask activity_mask;
  };

  enum Setting
    {
      SETTING_RESET = 0,
      SETTING_LIMIT,
      SETTING_SIZEOF
    };

  //! Timer settings at certain point in time.
  struct SettingsEntry
  {
    SettingsEntry()
      : timestamp(0), setting(0), break_id(0), value(0)
    {
    }

    bool operator<(const SettingsEntry &rhs)
    {
      return timestamp <  rhs.timestamp;
    }

    //! Timestamp of entry
    time_t timestamp;

    //! What setting has changed
    guint8 setting;

    //! What timer setting is reported in this entry
    guint8 break_id;

    //! New settings value.
    int value;
  };

  typedef std::deque<ActivityEntry> Activity;
  typedef Activity::iterator ActivityIter;

  typedef std::deque<SettingsEntry> Settings;
  typedef Settings::iterator SettingsIter;
  typedef Settings::reverse_iterator SettingsRIter;

  //! Complete history needed to reconstruct timers.
  struct History
  {
    //! History of activity changes.
    Activity *activity_log;

    //! History of timer settings changes
    Settings *settings_log;
  };

  //! Merges activity history of two workraves.
  class ActivityMerger
  {
  public:
    ActivityMerger(History &local, History &remote);

    void merge();

    Activity *get_new_activity_log();
    Settings *get_new_settings_log();

    int get_timer_idle(BreakId id);
    int get_timer_elapsed(BreakId id);
    int get_timer_overdue(BreakId id);

  private:

    /*! Timer state for merging. */
    struct TimerState
    {
      int idle_time;
      int active_time;
      int overdue_time;
      time_t last_update_time;
      bool running;

      //! Current auto reset interval
      int auto_reset;

      //! Current break limit
      int limit;

      //! Settings over time.
      Settings settings;
    };

    /*! Originator of activity change */
    enum From
      {
        /*! Invalid activity */
        FROM_NONE = -1,

        /*! Activity from local workrave*/
        FROM_LOCAL = 0,

        /*! Activity from remote workrave*/
        FROM_REMOTE,

        /*! Enum size */
        FROM_SIZE
      };

    template<typename Type>
    From get_first(std::deque<Type> *local, std::deque<Type> *remote, Type &out);
    
    void merge_settings();
    void merge_history();

    void process_change(int num, time_t ts, bool running);
    void process_history(int num, time_t ts, bool running);

    void process_timer_and_setting(int num, time_t ts, bool running);
    void process_timer(int num, time_t ts, bool running);

    //! New activity log
    Activity *activity_log;

    //! New activity log
    Settings *settings_log;

    //! History from local Workrave.
    History &local;

    //! History from remte Workrave.
    History &remote;

    //! State of all timers
    TimerState timers[BREAK_ID_SIZEOF];

    //!
    ActivityMask last_activity_mask;

    //!
    time_t last_time;

    //! Auto reset time predicate. (or NULL if not used)
    TimePred *autoreset_interval_predicate;
  };

private:
  void handle_linkstate_event(LinkEvent *event);
  void handle_history_event(LinkEvent *event);

  void packetize(ByteStream *bs);
  void depacketize(ByteStream *bs, bool instant, History &newhistory);

  void load();
  void save();

  void expire();
  void expire_activity();
  void expire_settings();

  void merge(History &new_log);

  void config_changed_notify(const std::string &key);

  void report_activity(int num, bool running);
  void report_setting(Setting setting, BreakId id, int value);

  // ILinkEventListener
  void event_received(LinkEvent *event);
  
private:
  //! The main core.
  ICore *core;

  //! The distributed event router
  LinkRouter *router;

  //! Activity log
  Activity *activity_log;

  //! Settings log
  Settings *settings_log;

  //! Auto reset time of all breaks
  int auto_reset[workrave::BREAK_ID_SIZEOF];

  //! Limit time of all breaks
  int limit[workrave::BREAK_ID_SIZEOF];

#ifdef HAVE_TESTS
  friend class Test;
  friend class org_workrave_DebugInterface_Stub;
#endif
};

#endif // LINKEDHISTORYMANAGER_HH
