// LinkedHistoryManager.cc --- Linked (networked) HistoryManager
//
// Copyright (C) 2007, 2008, 2009 Rob Caelers <robc@krandor.nl>
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

static const char rcsid[] = "$Id: HistoryManager.cc 1087 2006-10-01 18:40:08Z dotsphinx $";

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fstream>

#include <glib/gstdio.h>

#include "debug.hh"

#include "LinkedHistoryManager.hh"

#include "ICore.hh"
#include "IConfigurator.hh"
#include "INetwork.hh"
#include "Timer.hh"
#include "Core.hh"
#include "CoreConfig.hh"

#include "TimePredFactory.hh"
#include "TimePred.hh"
#include "Break.hh"
#include "CoreFactory.hh"
#include "LinkRouter.hh"
#include "LinkEvent.hh"
#include "LinkException.hh"
#include "LinkStateLinkEvent.hh"
#include "HistoryLinkEvent.hh"
#include "ByteStream.hh"
#include "Util.hh"
#include "StringUtil.hh"

using namespace std;

#define HISTORY_MAXSIZE     (5000)
#define HISTORY_MAXAGE      (24 * 60 * 60)
#define HISTORY_INTERVAL    (60)
#define HISTORY_FILENAME    "usage_history.log"

//! Construct a new Linked History manager
LinkedHistoryManager::LinkedHistoryManager(LinkRouter *router)
  : router(router)
{
  core = CoreFactory::get_core();

  activity_log = new Activity;
  settings_log = new Settings;
}


//! Destruct the manager
LinkedHistoryManager::~LinkedHistoryManager()
{
  delete activity_log;
  delete settings_log;
}


//! Initialize the linked history manager
void
LinkedHistoryManager::init()
{
  router->subscribe("linkstatelinkevent", this);
  router->subscribe("historylinkevent", this);

  IConfigurator *config = CoreFactory::get_configurator();
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      IBreak *b = core->get_break((BreakId)i);

      config->add_listener(CoreConfig::CFG_KEY_TIMER_AUTO_RESET % (BreakId)i, this);

      auto_reset[i] = (int) b->get_auto_reset();

      config->add_listener(CoreConfig::CFG_KEY_TIMER_LIMIT % (BreakId)i, this);

      limit[i] = (int) b->get_limit();

      report_setting(SETTING_RESET, (BreakId)i, auto_reset[i]);
      report_setting(SETTING_LIMIT, (BreakId)i, limit[i]);
    }

  load();
}


void
LinkedHistoryManager::terminate()
{
  save();
}


void
LinkedHistoryManager::heartbeat()
{
  time_t now = core->get_time();

  if (now % HISTORY_INTERVAL == 0)
    {
      expire();
      save();
    }

}

void
LinkedHistoryManager::report_user_activity(bool active)
{
  report_activity(0, active);
}


void
LinkedHistoryManager::report_timer_activity(int id, bool running)
{
  report_activity(id + 1, running);
}



void
LinkedHistoryManager::report_activity(int num, bool running)
{
  TRACE_ENTER_MSG("LinkedHistoryManager::add_activity", num << " " << running);

  bool add = true;
  time_t now = core->get_time();

  if (activity_log->size() > 0)
    {
      ActivityEntry &ae = activity_log->back();
      bool active = (ae.activity_mask & (1 << num)) != 0;

      TRACE_MSG((int)ae.activity_mask << " " << ae.timestamp);

      if (active == running)
        {
          add = false;
        }
      else if (ae.timestamp == now)
        {
          if (running)
            {
              ae.activity_mask |= (1 << num);
            }
          else
            {
              ae.activity_mask &= ~(1 << num);
            }

          add = false;
        }
    }

  if (add)
    {
      ActivityEntry ae;
      ae.timestamp = now;
      ae.activity_mask = running ? (1 << num) : 0;

      activity_log->push_back(ae);
    }

  TRACE_EXIT();
}


void
LinkedHistoryManager::packetize(ByteStream *bs)
{
  TRACE_ENTER("LinkedHistoryManager::packetize");

  time_t now = core->get_time();

  bs->put_u8('W');     // Magic.
  bs->put_u8('R');
  bs->put_u8('H');
  bs->put_u8('L');
  bs->put_u8(1);       // Version
  bs->put_u8(0);       // Reserved
  bs->put_u64(0);      // timebase, not used, yet.
  bs->put_u64(now);    // now

  bs->put_u32(activity_log->size());   // Number of activity changes
  bs->put_u8(1);       // Version
  bs->put_u8(0);       // Reserved

  for (ActivityIter i = activity_log->begin(); i != activity_log->end(); i++)
    {
      ActivityEntry &ae = *i;

      bs->put_u32((guint32)ae.timestamp);
      bs->put_u8(ae.activity_mask);
      bs->put_u8(0);   // Reserved
      TRACE_MSG("act: " << ae.timestamp << " " << (int) ae.activity_mask);
    }

  bs->put_u32(settings_log->size());   // Number of settings changes
  bs->put_u8(1);       // Version
  bs->put_u8(0);       // Reserved

  for (SettingsIter i = settings_log->begin(); i != settings_log->end(); i++)
    {
      SettingsEntry &se = *i;

      bs->put_u32((guint32)se.timestamp);
      bs->put_u8(se.setting);
      bs->put_u8(0);
      bs->put_u8(se.break_id);
      bs->put_u8(0);
      bs->put_u32(se.value);
      bs->put_u32(0);

      TRACE_MSG("set: " << se.timestamp << " " << (int)se.break_id << " " << (int)se.setting << " " << se.value);
    }
  TRACE_EXIT();
}


void
LinkedHistoryManager::depacketize(ByteStream *bs, bool instant,
                                  History &newhistory)
{
  TRACE_ENTER("LinkedHistoryManager::depacketize");

  guint8 magic1 = bs->get_u8();
  guint8 magic2 = bs->get_u8();
  guint8 magic3 = bs->get_u8();
  guint8 magic4 = bs->get_u8();

  if (magic1 != 'W' || magic2 != 'R' ||
      magic3 != 'H' || magic4 != 'L')
    {
      throw LinkException("Incorrect history magic");
    }

  guint8 version = bs->get_u8();
  guint8 reserved = bs->get_u8();

  if (version != 1)
    {
      throw LinkException("Unsupported history version");
    }

  time_t now = core->get_time();

  guint64 timebase = bs->get_u64();
  guint64 remote_now = bs->get_u64();

  guint32 logsize = bs->get_u32();
  version = bs->get_u8();
  reserved = bs->get_u8();

  TRACE_MSG("ref: " << timebase << " " << remote_now << " " << now);

  bool skipped = false;
  ActivityMask skipped_activity_mask = 0;
  time_t threshold = 0;

  if (now > HISTORY_MAXAGE)
    {
      // Normally, now is always more than HISTORY_MAXAGE.
      // Except during testing, where time is set to 0.
      threshold = now - HISTORY_MAXAGE;
    }

  
  for (int i = 0; i < (int)logsize; i++)
    {
      ActivityEntry ae;

      guint32 timestamp = bs->get_u32();
      ae.activity_mask = bs->get_u8();
      reserved = bs->get_u8();

      ae.timestamp = timestamp + timebase;

      if (instant)
        {
          ae.timestamp += (now - remote_now);
        }

      if (timestamp > (guint32)threshold)
        {
          TRACE_MSG("act: " << ae.timestamp << " " << (int)ae.activity_mask);
          newhistory.activity_log->push_back(ae);
        }
      else
        {
          skipped_activity_mask = ae.activity_mask;
          skipped = true;
        }
    }

  if (skipped)
    {
      ActivityEntry ae;

      ae.activity_mask = skipped_activity_mask;
      ae.timestamp = threshold;
      newhistory.activity_log->push_front(ae);
    }

  logsize = bs->get_u32();
  version = bs->get_u8();
  reserved = bs->get_u8();
  for (int i = 0; i < (int)logsize; i++)
    {
      SettingsEntry se;

      guint32 timestamp = bs->get_u32();
      se.setting = bs->get_u8();
      reserved = bs->get_u8();
      se.break_id = bs->get_u8();
      reserved = bs->get_u8();
      se.value = bs->get_u32();
      reserved = bs->get_u32();

      se.timestamp = timestamp + timebase + (now - remote_now);

      newhistory.settings_log->push_back(se);
      TRACE_MSG("set: " << se.timestamp << " " << (int)se.break_id << " " << (int)se.setting << " " << se.value);
    }

  if (!instant)
    {
      ActivityEntry ae;

      ae.activity_mask = 0;
      ae.timestamp = remote_now;
      newhistory.activity_log->push_back(ae);
    }

  TRACE_EXIT();
}


void
LinkedHistoryManager::load()
{
  TRACE_ENTER("LinkedHistoryManager::load");

  try
    {
      string filename = Util::get_home_directory() + HISTORY_FILENAME;

      // Open file
      ifstream file(filename.c_str());

      if (file.good())
        {
          // get file size using buffer's members
          filebuf *pbuf=file.rdbuf();
          int size = pbuf->pubseekoff (0,ios::end, ios::in);
          pbuf->pubseekpos (0,ios::in);

          if (size > 0 )
            {
              // Read it
              ByteStream bs(size, 1024);
              file.read((char *)bs.get_data_ptr(), size);
              bs.advance(file.gcount());
              file.close();

              // Parse
              History newhistory;
              newhistory.activity_log = new Activity;
              newhistory.settings_log = new Settings;

              bs.rewind();
              depacketize(&bs, false, newhistory);

              delete activity_log;
              delete settings_log;

              // Use them
              activity_log = newhistory.activity_log;
              settings_log = newhistory.settings_log;
            }
        }
    }
  catch(ByteStreamException &e)
    {
      // Error depacketizing incoming data.
      TRACE_MSG("Exception " << e.details());

      // FIXME: resolve memory leak
    }

  for (ActivityIter i = activity_log->begin(); i != activity_log->end(); i++)
    {
      ActivityEntry &ae = *i;
      TRACE_MSG("act: " << ae.timestamp << " " << (int)ae.activity_mask);
    }

  TRACE_EXIT();
}


void
LinkedHistoryManager::save()
{
  TRACE_ENTER("LinkedHistoryManager::save");

  ByteStream bs(10240, 1024);
  packetize(&bs);

  string filename = Util::get_home_directory() + HISTORY_FILENAME;
  string new_filename = filename + ".new";
  
  ofstream file(new_filename.c_str());
  file.write((char *)bs.get_data(), bs.get_offset());
  file.close();

  for (ActivityIter i = activity_log->begin(); i != activity_log->end(); i++)
    {
      ActivityEntry &ae = *i;
      TRACE_MSG("act: " << ae.timestamp << " " << (int)ae.activity_mask);
    }

  g_rename(new_filename.c_str(), filename.c_str());
  TRACE_EXIT();
}


void
LinkedHistoryManager::expire()
{
  TRACE_ENTER("LinkedHistoryManager::expire");
  expire_activity();
  expire_settings();
  TRACE_EXIT();
}

void
LinkedHistoryManager::expire_activity()
{
  TRACE_ENTER("LinkedHistoryManager::expire");
  time_t now = core->get_time();
  time_t time_threshold = now - HISTORY_MAXAGE;
  int num_remove = 0;

  TRACE_MSG("size " << activity_log->size());
  if (activity_log->size() > HISTORY_MAXSIZE)
    {
      num_remove = activity_log->size() - HISTORY_MAXSIZE;
    }

  ActivityIter i = activity_log->begin();
  while (i != activity_log->end())
    {
      ActivityEntry &ae = *i;

      if (num_remove > 0)
        {
          num_remove--;
        }
      else if (ae.timestamp >= time_threshold)
        {
          break;
        }

      i++;
    }

  activity_log->erase(activity_log->begin(), i);

  TRACE_MSG("size " << activity_log->size());
  for (ActivityIter i = activity_log->begin(); i != activity_log->end(); i++)
    {
      ActivityEntry &ae = *i;
      TRACE_MSG("act: " << ae.timestamp << " " << (int)ae.activity_mask);
    }

  TRACE_EXIT();
}

void
LinkedHistoryManager::expire_settings()
{
  TRACE_ENTER("LinkedHistoryManager::expire_settings");
  time_t now = core->get_time();
  time_t time_threshold = now - HISTORY_MAXAGE;
  int num_remove = 0;

  TRACE_MSG("size " << settings_log->size());
  if (settings_log->size() > HISTORY_MAXSIZE)
    {
      num_remove = settings_log->size() - HISTORY_MAXSIZE;
    }

  SettingsIter i = settings_log->begin();
  while (i != settings_log->end())
    {
      SettingsEntry &ae = *i;

      if (num_remove > 0)
        {
          num_remove--;
        }
      else if (ae.timestamp >= time_threshold)
        {
          break;
        }

      i++;
    }

  settings_log->erase(settings_log->begin(), i);

  TRACE_MSG("size " << settings_log->size());
  for (SettingsIter i = settings_log->begin(); i != settings_log->end(); i++)
    {
      SettingsEntry &se = *i;
      TRACE_MSG("set: " << se.timestamp << " ");
    }

  TRACE_EXIT();
  TRACE_EXIT();
}

void
LinkedHistoryManager::event_received(LinkEvent *event)
{
  TRACE_ENTER_MSG("LinkedHistoryManager::event_received", event->str());
  TRACE_MSG("current_time" << core->get_time());

  string eventid = event->get_eventid();

  if (eventid == "linkstatelinkevent")
    {
      handle_linkstate_event(event);
    }
  else if (eventid == "historylinkevent")
    {
      handle_history_event(event);
    }
  else
    {
    }

  TRACE_EXIT();
}

void
LinkedHistoryManager::handle_linkstate_event(LinkEvent *event)
{
  TRACE_ENTER_MSG("LinkedHistoryManager::handle_linkstate_event", event->str());

  LinkStateLinkEvent *lse = dynamic_cast<LinkStateLinkEvent *>(event);
  if (lse == NULL)
    {
      return;
    }

  try
    {
      LinkStateLinkEvent::LinkState ls = lse->get_link_state();
      if (ls == LinkStateLinkEvent::LINKSTATE_UP)
        {
          const WRID &link_id = lse->get_link_id();
          ByteStream bs(10240, 1024);
          packetize(&bs);

          HistoryLinkEvent alevent(bs.get_offset(),
                                   bs.get_data());
          router->send_event_to_link(link_id, &alevent);
        }
    }
  catch(ByteStreamException &e)
    {
      TRACE_MSG("Exception " << e.details());
      // Error creating packet.
    }

  TRACE_EXIT();
}


void
LinkedHistoryManager::handle_history_event(LinkEvent *event)
{
  TRACE_ENTER_MSG("LinkedHistoryManager::handle_history_event", event->str());

  HistoryLinkEvent *ale = dynamic_cast<HistoryLinkEvent *>(event);
  if (ale == NULL)
    {
      return;
    }

  try
    {
      History newhistory;

      newhistory.activity_log = new Activity;
      newhistory.settings_log = new Settings;

      ByteStream bs(ale->get_log_size(), ale->get_log_data());
      depacketize(&bs, true, newhistory);
      merge(newhistory);
    }
  catch(ByteStreamException &e)
    {
      // Error depacketizing incoming data.
      TRACE_MSG("Exception " << e.details());
    }

  TRACE_EXIT();
}


void
LinkedHistoryManager::merge(History &remote_history)
{
  History local_history;

  local_history.activity_log = activity_log;
  local_history.settings_log = settings_log;

  ActivityMerger merger(local_history, remote_history);
  merger.merge();

  delete local_history.activity_log;
  delete local_history.settings_log;
  delete remote_history.activity_log;
  delete remote_history.settings_log;

  activity_log = merger.get_new_activity_log();
  settings_log = merger.get_new_settings_log();

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Core *icore = Core::get_instance();
      Timer *timer = icore->get_timer((BreakId)i);

      timer->set_state(merger.get_timer_elapsed((BreakId)i),
                       merger.get_timer_idle((BreakId)i),
                       merger.get_timer_overdue((BreakId)i));
    }

  save();
}

void
LinkedHistoryManager::config_changed_notify(const std::string &key)
{
  TRACE_ENTER_MSG("LinkedHistoryManager::config_changed_notify", key);
  TRACE_MSG("current_time" << core->get_time());

  BreakId id = BREAK_ID_NONE;
  
  if (CoreConfig::match(key, CoreConfig::CFG_KEY_TIMER_AUTO_RESET, id))
    {
      IBreak *b = core->get_break(id);
      report_setting(SETTING_RESET, id, (int) b->get_auto_reset());
    }

  if (CoreConfig::match(key, CoreConfig::CFG_KEY_TIMER_LIMIT, id))
    {
      IBreak *b = core->get_break(id);
      report_setting(SETTING_LIMIT, id, (int) b->get_limit());
    }

  TRACE_EXIT();
}

void
LinkedHistoryManager::report_setting(Setting setting, BreakId id, int value)
{
  TRACE_ENTER_MSG("LinkedHistoryManager::report_setting", setting << " " << id <<" " << value);
  time_t now = core->get_time();
  bool add = true;

  TRACE_MSG(now);
  for (SettingsRIter i = settings_log->rbegin(); i != settings_log->rend(); i++)
    {
      SettingsEntry &se = *i;
      
      if (se.setting == setting && se.break_id == id)
        {
          TRACE_MSG("found " << se.timestamp);
          if (se.timestamp == now)
            {
              TRACE_MSG("same time");
              se.value = value;
              add = false;
            }
          else if (se.value == value)
            {
              TRACE_MSG("same value");
              add = false;
            }
          break;
        }
    }
  
  if (add)
    {
      SettingsEntry se;

      se.timestamp = now;
      se.break_id = id;
      se.setting = setting;
      se.value = value;

      settings_log->push_back(se);
    }

  TRACE_EXIT();
}


//! Construct a new activity history merger
LinkedHistoryManager::ActivityMerger::ActivityMerger(History &local, History &remote)
  : local(local), remote(remote)
{
  last_activity_mask = 0;
  last_time = 0;
  activity_log = new Activity;
  settings_log = new Settings;

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      timers[i].idle_time = 0;
      timers[i].active_time = 0;
      timers[i].overdue_time = 0;
      timers[i].last_update_time = 0;
      timers[i].running = false;

      ICore *core = CoreFactory::get_core();
      IBreak *b = core->get_break((BreakId)i);

      timers[i].auto_reset = (int) b->get_auto_reset();
      timers[i].limit = (int) b->get_limit();
    }

  string predicate;
  CoreFactory::get_configurator()->get_value("timers/daily_limit/reset_pred", predicate);
  autoreset_interval_predicate = TimePredFactory::create_time_pred(predicate);
}


//! Return the first unprocessed local or remote activity change
template<typename Type>
LinkedHistoryManager::ActivityMerger::From
LinkedHistoryManager::ActivityMerger::get_first(deque<Type> *local, deque<Type> *remote, Type &out)
{
  if (local->size() == 0)
    {
      if (remote->size() == 0)
        {
          return FROM_NONE;
        }
      else
        {
          out = remote->front();
          remote->pop_front();

          return FROM_REMOTE;
        }
    }
  else
    {
      if (remote->size() == 0)
        {
          out = local->front();
          local->pop_front();

          return FROM_LOCAL;
        }
      else
        {
          Type &le = local->front();
          Type &re = remote->front();

          if (le.timestamp < re.timestamp)
            {
              out = le;
              local->pop_front();

              return FROM_LOCAL;
            }
          else
            {
              out = re;
              remote->pop_front();

              return FROM_REMOTE;
            }
        }
    }
}

//! Merge local and remote activity history.
void
LinkedHistoryManager::ActivityMerger::merge()
{
  TRACE_ENTER("LinkedHistoryManager::ActivityMerger::merge");

  merge_settings();
  merge_history();

  TRACE_EXIT();
}


//! Merge local and remote activity history.
void
LinkedHistoryManager::ActivityMerger::merge_settings()
{
  TRACE_ENTER("LinkedHistoryManager::ActivityMerger::merge_settings");
  SettingsEntry *last_values[SETTING_SIZEOF][BREAK_ID_SIZEOF];

  for (int i = 0; i < SETTING_SIZEOF; i++)
    {
      for (int j = 0; j < BREAK_ID_SIZEOF; j++)
        {
          last_values[i][j] = NULL;
        }
    }

  while (1)
    {
      SettingsEntry se;
      From from = get_first(local.settings_log,
                            remote.settings_log,
                            se);

      TRACE_MSG("set: " << se.timestamp << " " << (int)se.break_id << " " << (int)se.setting << " " << se.value);

      if (from == FROM_NONE)
        break;

      if (last_values[se.setting][se.break_id] != NULL &&
          last_values[se.setting][se.break_id]->timestamp == se.timestamp)
        {
          // This is very unlikely, both local as well as remote Workrave
          // changed a setting at the same time. Both Workraves MUST select the
          // same one. So take the smallest value...
          if (se.value < last_values[se.setting][se.break_id]->value)
            {
              last_values[se.setting][se.break_id]->value = se.value;
            }
        }
      else
        {
          settings_log->push_back(se);
          // FIXME: is this safe?
          last_values[se.setting][se.break_id] = &settings_log->back();
        }
    }
 
  for (SettingsIter i = settings_log->begin(); i != settings_log->end(); i++)
    {
      SettingsEntry &se = *i;
      timers[se.break_id].settings.push_back(se);
    }
  
  TRACE_EXIT();
};


//! Merge local and remote activity history.
void
LinkedHistoryManager::ActivityMerger::merge_history()
{
  TRACE_ENTER("LinkedHistoryManager::ActivityMerger::merge_history");

  // Debugging
  for (ActivityIter i = local.activity_log->begin(); i != local.activity_log->end(); i++)
    {
      ActivityEntry &ae = *i;
      TRACE_MSG("local  act: " << ae.timestamp << " " << (int)ae.activity_mask);
    }
  for (ActivityIter i = remote.activity_log->begin(); i != remote.activity_log->end(); i++)
    {
      ActivityEntry &ae = *i;
      TRACE_MSG("remote act: " << ae.timestamp << " " << (int)ae.activity_mask);
    }

  time_t start_time[ACTIVITY_MASK_SIZE][FROM_SIZE];
  time_t stop_time[ACTIVITY_MASK_SIZE][FROM_SIZE];
  int count[ACTIVITY_MASK_SIZE];
  int start_delay[ACTIVITY_MASK_SIZE];

  // Initalize locals.
  for (int i = 0; i < ACTIVITY_MASK_SIZE; i++)
    {
      for (int j = 0; j < FROM_SIZE; j++)
        {
          start_time[i][j] = 0;
          stop_time[i][j] = 0;
        }
      count[i] = 0;
      start_delay[i] = 0;
    }

  // Loop over all activity changes.
  while (1)
    {
      ActivityEntry ae;
      From from = get_first(local.activity_log, remote.activity_log, ae);

      TRACE_MSG("Activity from: " << from);

      if (from == FROM_NONE)
        break;

      time_t ts = ae.timestamp;

      for (int j = 0; j < ACTIVITY_MASK_SIZE; j++)
        {
          int mask = 1 << j;

          TRACE_MSG(" time:" << ts <<
                    " timer:" << j <<
                    " active:" << (int)ae.activity_mask <<
                    " state:" << (ae.activity_mask && mask) <<
                    " count:" << count[j]);
          TRACE_MSG(" start:" << (start_time[j][from] != 0 ? start_time[j][from] : -1)  <<
                    " stop:" << (stop_time[j][from] != 0 ? stop_time[j][from] : - 1));

          if ((ae.activity_mask & mask) != 0)
            {
              // Activity 'j' from client 'from' is now active.
              if (start_time[j][from] == 0)
                {
                  // The activity wasn't previously active.

                  // Set start and stop time of this activity.
                  start_time[j][from] = ts;
                  stop_time[j][from] = 0;

                  // Increment the number of client that have
                  // activity 'j' active.
                  count[j]++;

                  TRACE_MSG("started " << count[j]);

                  if (count[j] == 1)
                    {
                      // This is the first client that activates
                      // actitivy 'j'. Process it.
                      process_change(j, ts, true);
                    }
                }
            }
          else
            {
              // Activity 'j' from client 'from' is no longer active.

              if (count[j] == 2 &&
                  start_time[j][FROM_LOCAL]  != 0 &&
                  start_time[j][FROM_REMOTE] != 0)
                {
                  // Activity 'j' was active for both clients.
                  // Now client 'from' stopped the activity.
                  // Store the start delay.
                  
                  start_delay[j] = (int)(start_time[j][FROM_REMOTE] - start_time[j][FROM_LOCAL]);
                  TRACE_MSG("start delay " << start_delay[j]);
                }

              if (stop_time[j][from] == 0)
                {
                  // Stopping activity 'j' from client 'from'.
                  
                  if (start_time[j][from] != 0)
                    {
                      // Decrement the number of client that have
                      // activity 'j' active; only if it was previously
                      // started
                      count[j]--;
                    }

                  // Update start/stop times.
                  stop_time[j][from] = ts;
                  start_time[j][from] = 0;

                  TRACE_MSG(" stopped " << count[j]);
                  TRACE_MSG(" count " << j << " " <<  count[j]);

                  // Handles start delayes
                  // If:
                  //  1) start delay between both clients < 5 s
                  //  2) stop delay between both clients < 5 s
                  //  3) difference betwee start and stop delat is < 5s
                  // Then consider the intervals identical
                  //
                  if (abs(start_delay[j]) < 5 &&
                      count[j] == 0 &&
                      stop_time[j][FROM_LOCAL]  != 0 &&
                      stop_time[j][FROM_REMOTE] != 0)
                    {
                      int stop_delay = (int)(stop_time[j][FROM_REMOTE] - stop_time[j][FROM_LOCAL]);
                      TRACE_MSG("stop delay " << stop_delay);
                      TRACE_MSG("start delay " << start_delay[j]);
                      if (abs(stop_delay) < 5 && abs(start_delay[j] - stop_delay) < 5)
                        {
                          ts = stop_time[j][FROM_LOCAL];
                          if (stop_time[j][FROM_REMOTE] < ts)
                            {
                              ts = stop_time[j][FROM_REMOTE];
                            }
                        }
                    }

                  if (count[j] == 0)
                    {
                      // Both clients  deactivated actitivy 'j'. Process it.
                      process_change(j, ts, false);
                    }
                }
            }
        }
    }
  TRACE_MSG("Activity done");

  ICore *core = CoreFactory::get_core();
  time_t now = core->get_time();
  for (int j = 0; j < ACTIVITY_MASK_SIZE; j++)
    {
      int mask = 1 << j;
      bool running = (last_activity_mask & mask) != 0;

      process_change(j, now, running);
    }

  // Debug.
  for (ActivityIter i = activity_log->begin(); i != activity_log->end(); i++)
    {
      ActivityEntry &ae = *i;
      TRACE_MSG("act: " << ae.timestamp << " " << (int)ae.activity_mask);
    }

  TRACE_EXIT();
}


//! Processes a change of activity
void
LinkedHistoryManager::ActivityMerger::process_change(int num, time_t ts, bool running)
{
  bool last_running = (last_activity_mask & (1 << num)) != 0;

  if (running != last_running || last_time == 0)
    {
      process_history(num, ts, running);
    }

  int timer_id = num - 1;
  if (timer_id >= 0 && timer_id < BREAK_ID_SIZEOF)
    {
      process_timer_and_setting(timer_id, ts, running);
    }

  last_time = ts;
}


//! Updates history after change of activity
void
LinkedHistoryManager::ActivityMerger::process_history(int num, time_t ts, bool running)
{
  if (last_time == ts)
    {
      ActivityEntry &ae = activity_log->back();

      if (running)
        {
          ae.activity_mask |= (1 << num);
        }
      else
        {
          ae.activity_mask &= ~(1 << num);
        }

      last_activity_mask = ae.activity_mask;
    }
  else
    {
      ActivityEntry ae;
      ae.timestamp = ts;
      ae.activity_mask = running ? (1 << num) : 0;

      last_activity_mask = ae.activity_mask;

      activity_log->push_back(ae);
    }
}


//! Updates timers after change of activity and check settins
void
LinkedHistoryManager::ActivityMerger::process_timer_and_setting(int timer_id, time_t ts, bool running)
{
  while (timers[timer_id].settings.size() > 0)
    {
      SettingsEntry &se = timers[timer_id].settings.front();

      if (se.timestamp <= ts)
        {
          process_timer(timer_id, se.timestamp, timers[timer_id].running);
          timers[timer_id].settings.pop_front();

          if (se.setting == SETTING_LIMIT)
            {
              timers[timer_id].limit = se.value;
            }
          else if (se.setting == SETTING_RESET)
            {
              timers[timer_id].auto_reset = se.value;
            }
        }
      else
        {
          break;
        }
    }

  process_timer(timer_id, ts, running);
}


//! Updates timers after change of activity
void
LinkedHistoryManager::ActivityMerger::process_timer(int timer_id, time_t ts, bool running)
{
  TRACE_ENTER_MSG("LinkedHistoryManager::ActivityMerger::process_timer",
                  timer_id << " " << ts << " " << running);

  TRACE_MSG("pre active time: " << timers[timer_id].active_time);
  TRACE_MSG("pre idle time: " << timers[timer_id].idle_time);
  TRACE_MSG("pre overdue time: " << timers[timer_id].overdue_time);
  
  if (timers[timer_id].last_update_time != 0)
    {
      // Elapsed time since last update.
      int elapsed = (int)(ts - timers[timer_id].last_update_time);

      if (timers[timer_id].running)
        {
          // The last 'elapsed' seconds, the timer was running.

          // Time left till next break;
          time_t left = timers[timer_id].limit - timers[timer_id].active_time;
          TRACE_MSG("left: " << left <<
                    " elapsed:" << elapsed <<
                    " overdue: " << timers[timer_id].overdue_time);
          if (left >= 0 && (elapsed - left) > 0)
            {
              // Timer not overdue yet: only part of elapsed time is overdue.
              timers[timer_id].overdue_time += (int)(elapsed - left);
            }
          else if (left < 0)
            {
              // Timer already overdue: entire elapsed time is overdue
              timers[timer_id].overdue_time += elapsed;
            }

          // Update timer idle/active time.
          timers[timer_id].active_time += elapsed;
          timers[timer_id].idle_time = 0;

          TRACE_MSG("now active: " << timers[timer_id].active_time);
        }
      else
        {
          // The last 'elapsed' seconds, the timer was not running.

          // Update timer idle time.
          timers[timer_id].idle_time += elapsed;
          TRACE_MSG("now idle: " << timers[timer_id].idle_time);

          if (timer_id != BREAK_ID_DAILY_LIMIT &&
              timers[timer_id].idle_time >= timers[timer_id].auto_reset)
            {
              // Micro/Rest break timer reset.

              timers[timer_id].active_time = 0;
            }
          else if (timer_id == BREAK_ID_DAILY_LIMIT)
            {
              // Compute next reset time of daily limit.
              autoreset_interval_predicate->set_last(timers[timer_id].last_update_time);
              time_t next = autoreset_interval_predicate->get_next();
              TRACE_MSG("last " << timers[timer_id].last_update_time << " " <<
                        "reset " << next);

              if (ts >= next)
                {
                  // Reset daily limit.
                  timers[timer_id].active_time = 0;

                  // Reset all overdue.
                  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
                    {
                      timers[i].overdue_time = 0;
                    }
                }
            }
        }
    }

  TRACE_MSG("ppst active time: " << timers[timer_id].active_time);
  TRACE_MSG("post idle time: " << timers[timer_id].idle_time);
  TRACE_MSG("post overdue time: " << timers[timer_id].overdue_time);

  timers[timer_id].running = running;
  timers[timer_id].last_update_time = ts;

  TRACE_EXIT();
}


//! Return the current activity log.
LinkedHistoryManager::Activity *
LinkedHistoryManager::ActivityMerger::get_new_activity_log()
{
  return activity_log;
}


//! Return the current settings log.
LinkedHistoryManager::Settings *
LinkedHistoryManager::ActivityMerger::get_new_settings_log()
{
  return settings_log;
}


//! Return the computed idle time of a timer.
int
LinkedHistoryManager::ActivityMerger::get_timer_idle(BreakId id)
{
  return timers[id].idle_time;
}


//! Return the computed elapsed active time of a timer.
int
LinkedHistoryManager::ActivityMerger::get_timer_elapsed(BreakId id)
{
  return timers[id].active_time;
}

//! Return the computed elapsed active time of a timer.
int
LinkedHistoryManager::ActivityMerger::get_timer_overdue(BreakId id)
{
  return timers[id].overdue_time;
}
