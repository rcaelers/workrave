// TimerBoxPreferencePage.cc --- Preferences widgets for a timer
//
// Copyright (C) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtkmm.h>

#include "commonui/nls.h"
#include "debug.hh"

#include "TimerBoxPreferencePage.hh"

#include "core/IBreak.hh"
#include "config/IConfigurator.hh"
#include "commonui/Backend.hh"
#include "core/ICore.hh"
#include "core/CoreConfig.hh"
#include "GtkUtil.hh"
#include "Hig.hh"
#include "commonui/GUIConfig.hh"
#include "commonui/TimerBoxControl.hh"
#include "utils/Platform.hh"

//! Constructs the Applet Preference Notebook page.
TimerBoxPreferencePage::TimerBoxPreferencePage(std::string n)
  : Gtk::HBox(false, 6),
    name(n),
    ontop_cb(nullptr)
{
  TRACE_ENTER("TimerBoxPreferencePage::TimerBoxPreferencePage");

  create_page();
  init_page_values();
  enable_buttons();
  init_page_callbacks();

  connections.add(GUIConfig::key_timerbox(name).connect(std::bind(&TimerBoxPreferencePage::enable_buttons, this)));

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      ICore::Ptr core = Backend::get_core();
      connections.add(CoreConfig::break_enabled(BreakId(i)).connect(std::bind(&TimerBoxPreferencePage::enable_buttons, this)));
    }

  TRACE_EXIT();
}


//! Destructs the Applet Preference Notebook page.
TimerBoxPreferencePage::~TimerBoxPreferencePage()
{
  TRACE_ENTER("TimerBoxPreferencePage::~TimerBoxPreferencePage");
  TRACE_EXIT();
}


//! Initializes all widgets.
void
TimerBoxPreferencePage::create_page()
{
  // Placement
  place_button  = Gtk::manage(new Gtk::ComboBoxText());
#if GTKMM_CHECK_VERSION(2, 24, 0)
  place_button->append(_("Place timers next to each other"));
  place_button->append(_("Place micro-break and rest break in one spot"));
  place_button->append(_("Place rest break and daily limit in one spot"));
  place_button->append(_("Place all timers in one spot"));
#else
  place_button->append_text(_("Place timers next to each other"));
  place_button->append_text(_("Place micro-break and rest break in one spot"));
  place_button->append_text(_("Place rest break and daily limit in one spot"));
  place_button->append_text(_("Place all timers in one spot"));
#endif

  // Cycle time spin button.
  cycle_entry = Gtk::manage(new Gtk::SpinButton());
  cycle_entry->set_range(1, 999);
  cycle_entry->set_increments(1, 10);
  cycle_entry->set_numeric(true);
  cycle_entry->set_width_chars(3);
  cycle_entry->signal_changed().connect
    (sigc::mem_fun(*this, &TimerBoxPreferencePage::on_cycle_time_changed));

  // Timer display
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      Gtk::ComboBoxText *display_button  = Gtk::manage(new Gtk::ComboBoxText());
      timer_display_button[i] = display_button;

#if GTKMM_CHECK_VERSION(2, 24, 0)
      display_button->append(_("Hide"));
      display_button->append(_("Show"));
      display_button->append(_("Show only when this timer is first due"));
#else
      display_button->append_text(_("Hide"));
      display_button->append_text(_("Show"));
      display_button->append_text(_("Show only when this timer is first due"));
#endif
    }

  // Enabled/Disabled checkbox
  Gtk::Label *enabled_lab = nullptr;

  if (name == "main_window")
    {
      enabled_lab = Gtk::manage(GtkUtil::create_label(_("Show status window"), false));

      // Always-on-top
#ifdef PLATFORM_OS_UNIX
      if (! workrave::utils::Platform::running_on_wayland())
#endif
        {
          ontop_cb = Gtk::manage
            (new Gtk::CheckButton
            (_("The status window stays always on top of other windows")));
          ontop_cb->signal_toggled().connect(sigc::mem_fun(*this, &TimerBoxPreferencePage::on_always_on_top_toggled));
          ontop_cb->set_active(GUIConfig::main_window_always_on_top()());
        }
    }
  else if (name == "applet")
    {
      enabled_lab = Gtk::manage(GtkUtil::create_label(_("Applet enabled"), false));
    }

  enabled_cb = Gtk::manage(new Gtk::CheckButton());
  enabled_cb->add(*enabled_lab);

  HigCategoryPanel *hig = Gtk::manage(new HigCategoryPanel(_("Display")));

  hig->add_widget(*enabled_cb);

  if (ontop_cb != nullptr)
    {
      hig->add_widget(*ontop_cb);
    }

  hig->add_label(_("Placement:"), *place_button);
  hig->add_label(_("Cycle time:"), *cycle_entry);

  hig->add_caption(_("Timers"));

  // Layout
  hig->add_label(_("Micro-break:"), *timer_display_button[0]);
  hig->add_label(_("Rest break:"), *timer_display_button[1]);
  hig->add_label(_("Daily limit:"), *timer_display_button[2]);

  pack_end(*hig, true, true, 0);

  set_border_width(12);
}


//! Retrieves the applet configuration and sets the widgets.
void
TimerBoxPreferencePage::init_page_values()
{
  int mp_slot = GUIConfig::timerbox_slot(name, BREAK_ID_MICRO_BREAK)();
  int rb_slot = GUIConfig::timerbox_slot(name, BREAK_ID_REST_BREAK)();
  int dl_slot = GUIConfig::timerbox_slot(name, BREAK_ID_DAILY_LIMIT)();
  int place;
  if (mp_slot < rb_slot && rb_slot < dl_slot)
    {
      place = 0;
    }
  else if (mp_slot == rb_slot && rb_slot == dl_slot)
    {
      place = 3;
    }
  else if (mp_slot == rb_slot)
    {
      place = 1;
    }
  else
    {
      place = 2;
    }
  place_button->set_active(place);


  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      int flags = GUIConfig::timerbox_flags(name, (BreakId) i)();
      int showhide;
      if (flags & GUIConfig::BREAK_HIDE)
        {
          showhide = 0;
        }
      else if (flags & GUIConfig::BREAK_WHEN_FIRST)
        {
          showhide = 2;
        }
      else
        {
          showhide = 1;
        }
      timer_display_button[i]->set_active(showhide);
    }
  cycle_entry->set_value(GUIConfig::timerbox_cycle_time(name)());

  enabled_cb->set_active(GUIConfig::timerbox_enabled(name)());
  enable_buttons();
}


void
TimerBoxPreferencePage::init_page_callbacks()
{
  place_button->signal_changed().connect
    (sigc::mem_fun(*this, &TimerBoxPreferencePage::on_place_changed));

  enabled_cb->signal_toggled().connect(sigc::mem_fun(*this, &TimerBoxPreferencePage::on_enabled_toggled));

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      timer_display_button[i]->signal_changed().connect
        (bind(sigc::mem_fun(*this, &TimerBoxPreferencePage::on_display_changed), i));
    }
}

//! The applet on/off checkbox has been toggled.
void
TimerBoxPreferencePage::on_enabled_toggled()
{
  bool on = enabled_cb->get_active();

  GUIConfig::timerbox_enabled(name).set(on);

  enable_buttons();
}


//! The placement is changed.
void
TimerBoxPreferencePage::on_place_changed()
{
  int slots[BREAK_ID_SIZEOF];
  int idx = place_button->get_active_row_number();
  switch (idx)
    {
    case 0:
      slots[BREAK_ID_MICRO_BREAK] = 0;
      slots[BREAK_ID_REST_BREAK] = 1;
      slots[BREAK_ID_DAILY_LIMIT] = 2;
      break;
    case 1:
      slots[BREAK_ID_MICRO_BREAK] = 0;
      slots[BREAK_ID_REST_BREAK] = 0;
      slots[BREAK_ID_DAILY_LIMIT] = 1;
      break;
    case 2:
      slots[BREAK_ID_MICRO_BREAK] = 0;
      slots[BREAK_ID_REST_BREAK] = 1;
      slots[BREAK_ID_DAILY_LIMIT] = 1;
      break;
    case 3:
      slots[BREAK_ID_MICRO_BREAK] = 0;
      slots[BREAK_ID_REST_BREAK] = 0;
      slots[BREAK_ID_DAILY_LIMIT] = 0;
      break;
    default:
      slots[BREAK_ID_MICRO_BREAK] = -1;
      slots[BREAK_ID_REST_BREAK] = -1;
      slots[BREAK_ID_DAILY_LIMIT] = -1;
    }

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      GUIConfig::timerbox_slot(name, (BreakId)i).set(slots[i]);
    }

}


//! The display of the specified break is changed.
void
TimerBoxPreferencePage::on_display_changed(int break_id)
{
  int sel = timer_display_button[break_id]->get_active_row_number();
  int flags = 0;
  switch (sel)
    {
    case 0:
      flags |= GUIConfig::BREAK_HIDE;
      break;
    case 1:
      flags = 0;
      break;
    default:
      flags = GUIConfig::BREAK_WHEN_FIRST;
      break;
    }
  GUIConfig::timerbox_flags(name, (BreakId) break_id).set(flags);

  enable_buttons();
}


//! Enable widgets
void
TimerBoxPreferencePage::enable_buttons()
{
  int count = 0;
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (! timer_display_button[i]->get_active())
        {
          count++;
        }
    }

  if (name == "applet")
    {
      bool on = enabled_cb->get_active();

      place_button->set_sensitive(on && count != 3);
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          ICore::Ptr core = Backend::get_core();
          IBreak::Ptr b = core->get_break(BreakId(i));

          bool timer_on = b->is_enabled();
          timer_display_button[i]->set_sensitive(on && timer_on);
        }
      cycle_entry->set_sensitive(on && count != 3);

    }
  else if (name == "main_window")
    {
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          ICore::Ptr core = Backend::get_core();
          IBreak::Ptr b = core->get_break(BreakId(i));
          timer_display_button[i]->set_sensitive(b->is_enabled());
        }
      if (count == 3)
        {
          if (GUIConfig::timerbox_enabled(name)())
            {
              GUIConfig::timerbox_enabled(name).set(false);
            }
          enabled_cb->set_active(false);
        }
      enabled_cb->set_sensitive(count != 3);
      place_button->set_sensitive(count != 3);
      cycle_entry->set_sensitive(count != 3);
      if (ontop_cb != NULL)
        {
          ontop_cb->set_sensitive(count != 3);
        }
    }
}


//! The applet cycle time has been changed.
void
TimerBoxPreferencePage::on_cycle_time_changed()
{
  int value = (int) cycle_entry->get_value();
  GUIConfig::timerbox_cycle_time(name).set(value);
}


void
TimerBoxPreferencePage::on_always_on_top_toggled()
{
  GUIConfig::main_window_always_on_top().set(ontop_cb->get_active());
}
