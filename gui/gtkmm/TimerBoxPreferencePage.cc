// TimerBoxPreferencePage.cc --- Preferences widgets for a timer
//
// Copyright (C) 2002, 2003 Rob Caelers & Raymond Penners
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
// $Id$
//

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "TimerBoxPreferencePage.hh"

#include "Configurator.hh"
#include "TimerBox.hh"
#ifdef HAVE_X
#include "AppletWindow.hh"
#endif
#include "MainWindow.hh"
#include "GtkUtil.hh"
#include "GUI.hh"
#include "Hig.hh"

//! Constructs the Applet Preference Notebook page.
TimerBoxPreferencePage::TimerBoxPreferencePage(string n)
  : Gtk::HBox(false, 6),
    name(n),
    ontop_cb(NULL)
{
  TRACE_ENTER("TimerBoxPreferencePage::TimerBoxPreferencePage");

  create_page();
  init_page_values();
  
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
  place_button  = manage(new Gtk::OptionMenu());
  Gtk::Menu *place_menu = manage(new Gtk::Menu());
  Gtk::Menu::MenuList &place_items = place_menu->items();
  place_button->set_menu(*place_menu);
  place_items.push_back(Gtk::Menu_Helpers::MenuElem
                        (_("Place timers next to each other")));
  place_items.push_back(Gtk::Menu_Helpers::MenuElem
                        (_("Place micro-pause and rest break in one spot")));
  place_items.push_back(Gtk::Menu_Helpers::MenuElem
                        (_("Place rest break and daily limit in one spot")));
  place_items.push_back(Gtk::Menu_Helpers::MenuElem
                        (_("Place all timers in one spot")));
  place_button->signal_changed().connect
    (SigC::slot(*this, &TimerBoxPreferencePage::on_place_changed));
  
  // Cycle time spin button.
  cycle_entry = manage(new Gtk::SpinButton());
  cycle_entry->set_range(1, 999);
  cycle_entry->set_increments(1, 10);
  cycle_entry->set_numeric(true);
  cycle_entry->set_width_chars(3);
  cycle_entry->signal_changed().connect
    (SigC::slot(*this, &TimerBoxPreferencePage::on_cycle_time_changed));

  // Timer display
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      Gtk::OptionMenu *display_button  = manage(new Gtk::OptionMenu());
      timer_display_button[i] = display_button;
      
      Gtk::Menu *menu = manage(new Gtk::Menu());
      Gtk::Menu::MenuList &menu_list = menu->items();
      display_button->set_menu(*menu);
      menu_list.push_back(Gtk::Menu_Helpers::MenuElem
                          (_("Hide")));
      menu_list.push_back(Gtk::Menu_Helpers::MenuElem
                          (_("Show")));
      menu_list.push_back(Gtk::Menu_Helpers::MenuElem
                          (_("Show only when this timer is first due")));
      display_button->signal_changed().connect
        (bind(SigC::slot(*this, &TimerBoxPreferencePage::on_display_changed), i));
    }

  // Enabled/Disabled checkbox
  Gtk::Label *enabled_lab = NULL;

  if (name == "main_window")
    {
      enabled_lab = manage(GtkUtil::create_label(_("Show status window"), false));

      // Always-on-top
      ontop_cb = manage
        (new Gtk::CheckButton
         (_("The status window stays always on top of other windows")));
      ontop_cb->signal_toggled().connect(SigC::slot(*this, &TimerBoxPreferencePage::on_always_on_top_toggled));
      ontop_cb->set_active(MainWindow::get_always_on_top());
    }
#ifdef HAVE_X  
  else if (name == "applet")
    {
      enabled_lab = manage(GtkUtil::create_label(_("Applet enabled"), false));
    }
#endif
  
  enabled_cb = manage(new  Gtk::CheckButton());
  enabled_cb->add(*enabled_lab);
  enabled_cb->signal_toggled().connect(SigC::slot(*this, &TimerBoxPreferencePage::on_enabled_toggled));

  HigCategoryPanel *hig = manage(new HigCategoryPanel(_("Display")));

  hig->add(*enabled_cb);

  if (ontop_cb != NULL)
    {
      hig->add(*ontop_cb);
    }
  
  hig->add(_("Placement:"), *place_button);
  hig->add(_("Cycle time:"), *cycle_entry);

  hig->add_caption(_("Timers"));
  
  // Layout
  hig->add(_("Micro-pause:"), *timer_display_button[0]);
  hig->add(_("Rest break:"), *timer_display_button[1]);
  hig->add(_("Daily limit:"), *timer_display_button[2]);

  pack_end(*hig, true, true, 0);
  
  set_border_width(12);
}

//! Retrieves the applet configuration and sets the widgets.
void
TimerBoxPreferencePage::init_page_values()
{
  enabled_cb->set_active(TimerBox::is_enabled(name));

  int mp_slot = TimerBox::get_timer_slot(name, GUIControl::BREAK_ID_MICRO_PAUSE);
  int rb_slot = TimerBox::get_timer_slot(name, GUIControl::BREAK_ID_REST_BREAK);
  int dl_slot = TimerBox::get_timer_slot(name, GUIControl::BREAK_ID_DAILY_LIMIT);
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
  place_button->set_history(place);


  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      int flags = TimerBox::get_timer_flags(name, (GUIControl::BreakId) i);
      int showhide;
      if (flags & TimerBox::BREAK_HIDE)
        {
          showhide = 0;
        }
      else if (flags & TimerBox::BREAK_WHEN_FIRST)
        {
          showhide = 2;
        }
      else
        {
          showhide = 1;
        }
      timer_display_button[i]->set_history(showhide);
    }
  cycle_entry->set_value(TimerBox::get_cycle_time(name));
  enable_buttons();
}


//! The applet on/off checkbox has been toggled.
void
TimerBoxPreferencePage::on_enabled_toggled()
{
  bool on = enabled_cb->get_active();

  TimerBox::set_enabled(name, on);

  enable_buttons();
}


//! The placement is changed.
void
TimerBoxPreferencePage::on_place_changed()
{
  int slots[GUIControl::BREAK_ID_SIZEOF];
  int idx = place_button->get_history();
  switch (idx)
    {
    case 0:
      slots[GUIControl::BREAK_ID_MICRO_PAUSE] = 0;
      slots[GUIControl::BREAK_ID_REST_BREAK] = 1;
      slots[GUIControl::BREAK_ID_DAILY_LIMIT] = 2;
      break;
    case 1:
      slots[GUIControl::BREAK_ID_MICRO_PAUSE] = 0;
      slots[GUIControl::BREAK_ID_REST_BREAK] = 0;
      slots[GUIControl::BREAK_ID_DAILY_LIMIT] = 1;
      break;
    case 2:
      slots[GUIControl::BREAK_ID_MICRO_PAUSE] = 0;
      slots[GUIControl::BREAK_ID_REST_BREAK] = 1;
      slots[GUIControl::BREAK_ID_DAILY_LIMIT] = 1;
      break;
    case 3:
      slots[GUIControl::BREAK_ID_MICRO_PAUSE] = 0;
      slots[GUIControl::BREAK_ID_REST_BREAK] = 0;
      slots[GUIControl::BREAK_ID_DAILY_LIMIT] = 0;
      break;
    default:
      slots[GUIControl::BREAK_ID_MICRO_PAUSE] = -1;
      slots[GUIControl::BREAK_ID_REST_BREAK] = -1;
      slots[GUIControl::BREAK_ID_DAILY_LIMIT] = -1;
    }

  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      TimerBox::set_timer_slot(name, (GUIControl::BreakId) i, slots[i]);
    }
  
}


//! The display of the specified break is changed.
void
TimerBoxPreferencePage::on_display_changed(int break_id)
{
  int sel = timer_display_button[break_id]->get_history();
  int flags;
  switch (sel)
    {
    case 0:
      flags |= TimerBox::BREAK_HIDE;
      break;
    case 1:
      flags = 0;
      break;
    default:
      flags = TimerBox::BREAK_WHEN_FIRST;
      break;
    }
  TimerBox::set_timer_flags(name, (GUIControl::BreakId) break_id, flags);

  enable_buttons();
}


//! Enable widgets
void
TimerBoxPreferencePage::enable_buttons(void)
{
  int count = 0;
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      if (timer_display_button[i]->get_history() == 0)
        {
          count++;
        }
    }

  if (name == "applet")
    {
      bool on = enabled_cb->get_active();

      place_button->set_sensitive(on && count != 3);
      for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
        {
          timer_display_button[i]->set_sensitive(on);
        }
      cycle_entry->set_sensitive(on && count != 3);

    }
  else if (name == "main_window")
    {
      if (count == 3)
        {
          TimerBox::set_enabled(name, false);
          enabled_cb->set_active(false);
        }
      enabled_cb->set_sensitive(count != 3);
      place_button->set_sensitive(count != 3);
      cycle_entry->set_sensitive(count != 3);
      ontop_cb->set_sensitive(count != 3);
    }
}


//! The applet cycle time has been changed.
void
TimerBoxPreferencePage::on_cycle_time_changed()
{
  int value = (int) cycle_entry->get_value();
  TimerBox::set_cycle_time(name, value);
}

void
TimerBoxPreferencePage::on_always_on_top_toggled()
{
  MainWindow::set_always_on_top(ontop_cb->get_active());
}

