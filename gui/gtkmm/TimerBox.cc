// TimerBox.cc --- Timers Widgets
//
// Copyright (C) 2001, 2002, 2003 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

static const char rcsid[] = "$Id$";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <unistd.h>
#include <iostream>

#include "TimerBox.hh"

#include "TimeBar.hh"
#include "GUI.hh"
#include "GUIControl.hh"
#include "Util.hh"
#include "Text.hh"

#include "TimerInterface.hh"
#include "ActivityMonitorInterface.hh"
#include "Configurator.hh"
#include "Menus.hh"

#ifdef HAVE_DISTRIBUTION
#include "DistributionManager.hh"
#endif

const string TimerBox::CFG_KEY_TIMERBOX = "gui/";
const string TimerBox::CFG_KEY_TIMERBOX_CYCLE_TIME = "/cycle_time";
const string TimerBox::CFG_KEY_TIMERBOX_ENABLED = "/enabled";
const string TimerBox::CFG_KEY_TIMERBOX_POSITION = "/position";
const string TimerBox::CFG_KEY_TIMERBOX_FLAGS = "/flags";
const string TimerBox::CFG_KEY_TIMERBOX_IMMINENT = "/imminent";


//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
TimerBox::TimerBox(string n) :
  labels(NULL),
  bars(NULL),
  sheep(NULL),
  cycle_time(10),
  vertical(false),
  size(0),
  table_rows(-1),
  table_columns(-1),
  visible_count(-1),
  name(n)
{
  init();
}
  


//! Destructor.
TimerBox::~TimerBox()
{
  TRACE_ENTER("TimerBox::~TimerBox");

  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      if (labels[i] != NULL)
        labels[i]->unreference();
      if (bars[i] != NULL)
        bars[i]->unreference();
    }

  delete [] bars;
  delete [] labels;
  
  if (sheep != NULL)
    sheep->unreference();

  Configurator *config = GUIControl::get_instance()->get_configurator();
  config->remove_listener(this);
  
  TRACE_EXIT();
}


//! Updates the main window.
void
TimerBox::update()
{
  if (reconfigure)
    {
      // Configuration was changed. reinit.
      init_table();
      reconfigure = false;
    }
  else
    {
      time_t t = time(NULL);
      if (t % cycle_time == 0)
        {
          init_table();
          cycle_slots();
        }
    }
  
  // Update the timer widgets.
  update_widgets();
}


void
TimerBox::set_geometry(bool vertical, int size)
{
  this->vertical = vertical;
  this->size = size;
  init_table();
}


//! Initializes the main window.
void
TimerBox::init()
{
  TRACE_ENTER("TimerBox::init");

  // Listen for configugration changes.
  Configurator *config = GUIControl::get_instance()->get_configurator();
  config->add_listener(TimerBox::CFG_KEY_TIMERBOX + name, this);

  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      GUIControl::TimerData *timer = &GUIControl::get_instance()->timers[i];
      config->add_listener(GUIControl::CFG_KEY_BREAK
                           + timer->break_name
                           + GUIControl::CFG_KEY_BREAK_ENABLED, this);

      break_position[i] = i;
      break_flags[i] = 0;
      break_imminent_time[i] = 0;
      current_content[i] = -1;
      
      for (int j = 0; j < GUIControl::BREAK_ID_SIZEOF; j++)
        {
          break_slots[i][j] = -1;
        }
      break_slot_cycle[i] = 0;
    }

  // Load the configuration
  read_configuration();

  string sheep_file = Util::complete_directory("workrave-icon-medium.png", Util::SEARCH_PATH_IMAGES);
  sheep = manage(new Gtk::Image(sheep_file));
  sheep->reference();

  init_widgets();

  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      labels[i]->reference();
      bars[i]->reference();
    }

  reconfigure = true;
  
  TRACE_EXIT();
}


void
TimerBox::init_widgets()
{
  labels = new Gtk::Widget*[GUIControl::BREAK_ID_SIZEOF];
  bars = new TimeBar*[GUIControl::BREAK_ID_SIZEOF];

  Glib::RefPtr<Gtk::SizeGroup> size_group
    = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);
  
  for (int count = 0; count < GUIControl::BREAK_ID_SIZEOF; count++)
    {
      GUIControl::TimerData *timer = &GUIControl::get_instance()->timers[count];
      Gtk::Image *img = manage(new Gtk::Image(timer->icon)); // FIXME: LEAK
      Gtk::Widget *w;
      if (count == GUIControl::BREAK_ID_REST_BREAK)
        {
          Gtk::Button *b = manage(new Gtk::Button());
          b->set_relief(Gtk::RELIEF_NONE);
          b->set_border_width(0);
          b->add(*img);
          
          Menus *menus = Menus::get_instance();
          b->signal_clicked().connect(SigC::slot(*menus, &Menus::on_menu_restbreak_now));
          w = b;
	}
      else
        {
	 w = img;
        }
      
      size_group->add_widget(*w);
      labels[count] = w;

      bars[count] = manage(new TimeBar); // FIXME: LEAK
      bars[count]->set_text_alignment(1);
      bars[count]->set_progress(0, 60);
      bars[count]->set_text(_("Wait"));
    }
}


//! Updates the main window.
void
TimerBox::update_widgets()
{
  bool node_master = true;
  int num_peers = 0;
  
#ifdef HAVE_DISTRIBUTION
  DistributionManager *dist_manager = DistributionManager::get_instance();
  if (dist_manager != NULL)
    {
      node_master = dist_manager->is_master();
      num_peers = dist_manager->get_number_of_peers();
    }
#endif
  
  for (unsigned int count = 0; count < GUIControl::BREAK_ID_SIZEOF; count++)
    {
      TimerInterface *timer = GUIControl::get_instance()->timers[count].timer;
      TimeBar *bar = bars[count];

      if (!node_master && num_peers > 0)
        {
#ifndef NEW_DISTR          
          bar->set_text(_("Inactive"));
          bar->set_bar_color(TimeBar::COLOR_ID_INACTIVE);
          bar->set_progress(0, 60);
          bar->set_secondary_progress(0, 0);
          bar->update();
          continue;
#else
          bar->set_text_color(Gdk::Color("white"));
#endif          
        }
      else
        {
          bar->set_text_color(Gdk::Color("black"));
        }
      
      if (timer == NULL)
        {
          continue;
        }
      
      TimerInterface::TimerState timerState = timer->get_state();

      // Collect some data.
      time_t maxActiveTime = timer->get_limit();
      time_t activeTime = timer->get_elapsed_time();
      time_t breakDuration = timer->get_auto_reset();
      time_t idleTime = timer->get_elapsed_idle_time();
      bool overdue = (maxActiveTime < activeTime);
          
      // Set the text
      if (timer->is_limit_enabled() && maxActiveTime != 0)
        {
          bar->set_text(Text::time_to_string(maxActiveTime - activeTime));
        }
      else
        {
          bar->set_text(Text::time_to_string(activeTime));
        }

      // And set the bar.
      bar->set_secondary_progress(0, 0);

      if (timerState == TimerInterface::STATE_INVALID)
        {
          bar->set_bar_color(TimeBar::COLOR_ID_INACTIVE);
          bar->set_progress(0, 60);
          bar->set_text(_("Wait"));
        }
      else
        {
          // Timer is running, show elapsed time.
          bar->set_progress(activeTime, maxActiveTime);
          
          if (overdue)
            {
              bar->set_bar_color(TimeBar::COLOR_ID_OVERDUE);
            }
          else
            {
              bar->set_bar_color(TimeBar::COLOR_ID_ACTIVE);
            }

          if (//timerState == TimerInterface::STATE_STOPPED &&
              timer->is_auto_reset_enabled() && breakDuration != 0)
            {
              // resting.
              bar->set_secondary_bar_color(TimeBar::COLOR_ID_INACTIVE);
              bar->set_secondary_progress(idleTime, breakDuration);
            }
        }
      bar->update();
    }
}


//! Initializes the applet.
void
TimerBox::init_table()
{
  TRACE_ENTER("TimerBox::init_table");

  // Determine what breaks to show.
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      init_slot(i);
    }

  
  // Compute number of visible breaks.
  int number_of_timers = 0;
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      if (break_slots[i][0] != -1)
        {
          number_of_timers++;
        }
    }

  // Compute table dimensions.
  int rows = number_of_timers;
  int columns = 1;

  if (rows == 0)
    {
      // Show sheep.
      rows = 1;
    }

  if (!vertical)
    {
      TRACE_MSG("!vertical")
      int width, height;
      bars[0]->get_preferred_size(width, height);
      
      rows = size / (height + 1);

      TRACE_MSG(size << " " << rows);
      if (rows <= 0)
        {
          rows = 1;
        }
    }

  columns = (number_of_timers + rows - 1) / rows;

  // Compute new content.
  int new_content[GUIControl::BREAK_ID_SIZEOF];
  int slot = 0;
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      new_content[i] = -1;
      int cycle = break_slot_cycle[i];
      int id = break_slots[i][cycle]; // break id
      if (id != -1)
        {
          new_content[slot] = id;
          slot++;
        }
    }

  bool remove_all = rows != table_rows || columns != table_columns || number_of_timers != visible_count;
  
  // Remove old
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      int id = current_content[i];
      if (id != -1 && (id != new_content[i] || remove_all))
        {
          Gtk::Widget *child = labels[id];
          remove(*child);
          child = bars[id];
          remove(*child);
          
          current_content[i] = -1;
        }
    }
  
  // Remove sheep
  if ((number_of_timers > 0 || remove_all) && visible_count == 0)
    {
      remove(*sheep);
      visible_count = -1;
    }

  TRACE_MSG(rows <<" " << table_rows << " " << columns << " " << table_columns);
  if (rows != table_rows || columns != table_columns || number_of_timers != visible_count)
    {
      TRACE_MSG("resize");
      resize(rows, 2 * columns);
      set_spacings(1);
      show_all();

      table_columns = columns;
      table_rows = rows;
    }
  
  // Add sheep.
  if (number_of_timers == 0 && visible_count != 0)
    {
      attach(*sheep, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
    }
  
  // Fill table.
  for (int i = 0; i < slot; i++)
    {
      int id = new_content[i];
      int cid = current_content[i];

      if (id != cid)
        {
          current_content[i] = id;
          
          int cur_row = i % rows;
          int cur_col = i / rows;
           
          TRACE_MSG("size = " << size);
          if (!vertical && size > 0)
            {
              GtkRequisition widget_size;
              size_request(&widget_size);
              
              TRACE_MSG("size = " << widget_size.width << " " << widget_size.height);
              //bars[id]->set_size_request(-1, size / rows - (rows + 1) - 2);
            }
          
          attach(*labels[id], 2 * cur_col, 2 * cur_col + 1, cur_row, cur_row + 1,
                 Gtk::FILL, Gtk::SHRINK);
          attach(*bars[id], 2 * cur_col + 1, 2 * cur_col + 2, cur_row, cur_row + 1,
                 Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
        }
    }

  for (int i = slot; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      current_content[i] = -1;
    }

  visible_count = number_of_timers;
  
  show_all();
  TRACE_EXIT();
}


//! Compute what break to show on the specified location.
void
TimerBox::init_slot(int slot)
{
  int count = 0;
  int breaks_id[GUIControl::BREAK_ID_SIZEOF];
  bool stop = false;

  // Collect all timers for this slot.
  for (int i = 0; !stop && i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      GUIControl::TimerData *timer = &GUIControl::get_instance()->timers[i];
      bool on = timer->get_break_enabled();
      
      if (on && break_position[i] == slot && !(break_flags[i] & BREAK_HIDE))
        {
          breaks_id[count] = i;
          break_flags[i] &= ~BREAK_SKIP;
          count++;
        }
    }

  // Compute timer that will elapse first.
  time_t first = 0;
  int first_id = -1;
    
  for (int i = 0; i < count; i++)
    {
      int id = breaks_id[i];
      int flags = break_flags[id];

      TimerInterface *timer = GUIControl::get_instance()->timers[id].timer;
      time_t time_left = timer->get_limit() - timer->get_elapsed_time();
        
      // Exclude break if not imminent.
      if (flags & BREAK_WHEN_IMMINENT && time_left > break_imminent_time[id])
        {
          break_flags[id] |= BREAK_SKIP;
        }

      // update first imminent timer.
      if (!(flags & BREAK_SKIP) && (first_id == -1 || time_left < first))
        {
          first_id = id;
          first = time_left;
        }
    }

  
  // Exclude break if not first.
  for (int i = 0; i < count; i++)
    {
      int id = breaks_id[i];
      int flags = break_flags[id];

      if (!(flags & BREAK_SKIP))
        {
          if (flags & BREAK_WHEN_FIRST && first_id != id)
            {
              break_flags[id] |= BREAK_SKIP;
            }
        }
    }

  
  // Exclude breaks if not exclusive.
  bool have_one = false;
  int breaks_left = 0;
  for (int i = 0; i < count; i++)
    {
      int id = breaks_id[i];
      int flags = break_flags[id];

      if (!(flags & BREAK_SKIP))
        {
          if (flags & BREAK_EXCLUSIVE && have_one)
            {
              break_flags[id] |= BREAK_SKIP;
            }

          have_one = true;
        }
      
      if (!(flags & BREAK_SKIP))
        {
          breaks_left++;
        }
    }

  if (breaks_left == 0)
    {
      for (int i = 0; i < count; i++)
        {
          int id = breaks_id[i];
          int flags = break_flags[id];
          
          if (flags & BREAK_DEFAULT && flags & BREAK_SKIP)
            {
              break_flags[id] &= ~BREAK_SKIP;
              breaks_left = 1;
              break;
            }
        }
    }

  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      break_slots[slot][i] = -1;
    }

  int new_count = 0;
  for (int i = 0; i < count; i++)
    {
      int id = breaks_id[i];
      int flags = break_flags[id];
          
      if (!(flags & BREAK_SKIP))
        {
          break_slots[slot][new_count] = id;
          new_count++;
        }
    }
}


//! Cycles through the breaks.
void
TimerBox::cycle_slots()
{
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      break_slot_cycle[i]++;
      if (break_slot_cycle[i] >= GUIControl::BREAK_ID_SIZEOF
          || break_slots[i][break_slot_cycle[i]] == -1)
        {
          break_slot_cycle[i] = 0;
        }
    }
}


//! Reads the applet configuration.
void
TimerBox::read_configuration()
{
  TRACE_ENTER("TimerBox::read_configuration");
  cycle_time = get_cycle_time(name);
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      GUIControl::BreakId bid = (GUIControl::BreakId) i;
      
      break_position[i] = get_timer_slot(name, bid);;
      break_flags[i] = get_timer_flags(name, bid);
      break_imminent_time[i] = get_timer_imminent_time(name, bid);
    }
  TRACE_EXIT();
}


//! Callback that the configuration has changed.
void
TimerBox::config_changed_notify(string key)
{
  (void) key;

  read_configuration();
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      break_slot_cycle[i] = 0;
    }

  reconfigure = true;
}


int
TimerBox::get_cycle_time(string name)
{
  int ret;
  if (! GUIControl::get_instance()->get_configurator()
      ->get_value(TimerBox::CFG_KEY_TIMERBOX + name + TimerBox::CFG_KEY_TIMERBOX_CYCLE_TIME, &ret))
    {
      ret = 10;
    }
  return ret;
}


void
TimerBox::set_cycle_time(string name, int time)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(TimerBox::CFG_KEY_TIMERBOX + name + TimerBox::CFG_KEY_TIMERBOX_CYCLE_TIME, time);
}


const string
TimerBox::get_timer_config_key(string name, GUIControl::BreakId timer, const string &key)
{
  GUIControl::TimerData *data = GUIControl::get_instance()->get_timer_data(timer);
  return string(CFG_KEY_TIMERBOX) + name + "/" + data->break_name + key;
}


int
TimerBox::get_timer_imminent_time(string name, GUIControl::BreakId timer)
{
  const string key = get_timer_config_key(name, timer, CFG_KEY_TIMERBOX_IMMINENT);
  int ret;
  if (! GUIControl::get_instance()->get_configurator()
      ->get_value(key, &ret))
    {
      ret = 30;
    }
  return ret;
}


void
TimerBox::set_timer_imminent_time(string name, GUIControl::BreakId timer, int time)
{
  const string key = get_timer_config_key(name, timer, CFG_KEY_TIMERBOX_IMMINENT);
  GUIControl::get_instance()->get_configurator()->set_value(key, time);
}


int
TimerBox::get_timer_slot(string name, GUIControl::BreakId timer)
{
  const string key = get_timer_config_key(name, timer, CFG_KEY_TIMERBOX_POSITION);
  int ret;
  if (! GUIControl::get_instance()->get_configurator()
      ->get_value(key, &ret))
    {
      if (name == "applet")
        {
          // All in one slot is probably the best default since we cannot assume
          // any users panel is large enough to hold all timers.
          ret = 0;
        }
      else
        {
          ret = timer;
        }
    }
  return ret;
}


void
TimerBox::set_timer_slot(string name, GUIControl::BreakId timer, int slot)
{
  const string key = get_timer_config_key(name, timer, CFG_KEY_TIMERBOX_POSITION);
  GUIControl::get_instance()->get_configurator()->set_value(key, slot);
}


int
TimerBox::get_timer_flags(string name, GUIControl::BreakId timer)
{
  const string key = get_timer_config_key(name, timer, CFG_KEY_TIMERBOX_FLAGS);
  int ret;
  if (! GUIControl::get_instance()->get_configurator()
      ->get_value(key, &ret))
    {
      ret = 0;
    }
  return ret;
}


void
TimerBox::set_timer_flags(string name, GUIControl::BreakId timer, int flags)
{
  const string key = get_timer_config_key(name, timer, CFG_KEY_TIMERBOX_FLAGS);
  GUIControl::get_instance()->get_configurator()->set_value(key, flags);
}



bool
TimerBox::is_enabled(string name)
{
  bool ret = true;
  if (! GUIControl::get_instance()->get_configurator()
      ->get_value(CFG_KEY_TIMERBOX + name + CFG_KEY_TIMERBOX_ENABLED, &ret))
    {
      ret = true;
    }
  return ret;
}


void
TimerBox::set_enabled(string name, bool enabled)
{
  GUIControl::get_instance()->get_configurator()
    ->set_value(CFG_KEY_TIMERBOX + name + CFG_KEY_TIMERBOX_ENABLED, enabled);
}
