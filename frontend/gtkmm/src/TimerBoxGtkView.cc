// TimerBoxGtkView.cc --- Timers Widgets
//
// Copyright (C) 2001, 2002, 2003, 2004 Rob Caelers & Raymond Penners
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"

#include <unistd.h>
#include <iostream>

#include <gtkmm/image.h>
#include <gtkmm/sizegroup.h>
#include <gtkmm/button.h>

#include "nls.h"
#include "debug.hh"


#include "TimerBoxGtkView.hh"
#include "TimeBar.hh"
#include "Util.hh"
#include "Text.hh"
#include "Menus.hh"
#include "GUI.hh"
#include "GtkUtil.hh"

#include "CoreFactory.hh"
#include "BreakInterface.hh"


//! Constructor.
TimerBoxGtkView::TimerBoxGtkView() :
  reconfigure(true),
  labels(NULL),
  bars(NULL),
  sheep(NULL),
  vertical(false),
  size(0),
  table_rows(-1),
  table_columns(-1),
  visible_count(-1)
{
  init();
}
  


//! Destructor.
TimerBoxGtkView::~TimerBoxGtkView()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
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
}



//! Sets the geometry of the timerbox.
void
TimerBoxGtkView::set_geometry(bool vertical, int size)
{
  this->vertical = vertical;
  this->size = size;
  init_table();
}


//! Initializes the timerbox.
void
TimerBoxGtkView::init()
{
  TRACE_ENTER("TimerBoxGtkView::init");

  string sheep_file = Util::complete_directory("workrave-icon-medium.png", Util::SEARCH_PATH_IMAGES);
  sheep = GtkUtil::create_image_with_tooltip(sheep_file, "Workrave");
  sheep->reference();

  init_widgets();

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      current_content[i] = new_content[i] = BREAK_ID_NONE;
      labels[i]->reference();
      bars[i]->reference();
    }

  reconfigure = true;
  
  TRACE_EXIT();
}


//! Initializes the widgets.
void
TimerBoxGtkView::init_widgets()
{
  labels = new Gtk::Widget*[BREAK_ID_SIZEOF];
  bars = new TimeBar*[BREAK_ID_SIZEOF];

  Glib::RefPtr<Gtk::SizeGroup> size_group
    = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_HORIZONTAL);

  char *icons[] = { "timer-micro-break.png", "timer-rest-break.png", "timer-daily.png" };
  for (int count = 0; count < BREAK_ID_SIZEOF; count++)
    {
      string icon = Util::complete_directory(string(icons[count]), Util::SEARCH_PATH_IMAGES);
      Gtk::Image *img = manage(new Gtk::Image(icon));
      Gtk::Widget *w;
      if (count == BREAK_ID_REST_BREAK)
        {
          Gtk::Button *b = manage(new Gtk::Button());
          b->set_relief(Gtk::RELIEF_NONE);
          b->set_border_width(0);
          b->add(*img);
          
          Menus *menus = Menus::get_instance();
          b->signal_clicked().connect(MEMBER_SLOT(*menus, &Menus::on_menu_restbreak_now));
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




//! Initializes the applet.
void
TimerBoxGtkView::init_table()
{
  TRACE_ENTER("TimerBoxGtkView::init_table");

  // Compute number of visible breaks.
  int number_of_timers = 0;
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (new_content[i] != BREAK_ID_NONE)
        {
          number_of_timers++;
        }
    }
  TRACE_MSG("number_of_timers = " << number_of_timers);
  
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
  TRACE_MSG("c/r " << columns << " " << rows);


  bool remove_all = rows != table_rows || columns != table_columns || number_of_timers != visible_count;
  
  // Remove old
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      int id = current_content[i];
      if (id != -1 && (id != new_content[i] || remove_all))
        {
          TRACE_MSG("remove " << i << " " << id);
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
      TRACE_MSG("remove sheep");
      remove(*sheep);
      visible_count = -1;
    }

  TRACE_MSG(rows <<" " << table_rows << " " << columns << " " << table_columns);
  //  if (rows != table_rows || columns != table_columns || number_of_timers != visible_count)
    {
      TRACE_MSG("resize");
      resize(rows, 2 * columns);
      //set_spacings(1);
      //show_all();

      table_columns = columns;
      table_rows = rows;
    }
  
  // Add sheep.
  if (number_of_timers == 0 && visible_count != 0)
    {
      attach(*sheep, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
    }
  
  // Fill table.
  for (int i = 0; i < number_of_timers; i++)
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
#ifdef HAVE_GTKMM24
              Gtk::Requisition widget_size;
              size_request(widget_size);
              
              TRACE_MSG("size = " << widget_size.width << " " << widget_size.height);
              //bars[id]->set_size_request(-1, size / rows - (rows + 1) - 2);
#else
              GtkRequisition widget_size;
              size_request(&widget_size);
              
              TRACE_MSG("size = " << widget_size.width << " " << widget_size.height);
              //bars[id]->set_size_request(-1, size / rows - (rows + 1) - 2);
#endif
              
            }

          TRACE_MSG("attach " << cur_col << " " << cur_row);
          
          attach(*labels[id], 2 * cur_col, 2 * cur_col + 1, cur_row, cur_row + 1,
                 Gtk::FILL, Gtk::SHRINK);
          attach(*bars[id], 2 * cur_col + 1, 2 * cur_col + 2, cur_row, cur_row + 1,
                 Gtk::EXPAND | Gtk::FILL, Gtk::SHRINK);
        }
    }

  for (int i = number_of_timers; i < BREAK_ID_SIZEOF; i++)
    {
      current_content[i] = -1;
    }

  visible_count = number_of_timers;
  
  show_all();
  TRACE_EXIT();
}







void
TimerBoxGtkView::set_slot(BreakId id, int slot)
{
  if (current_content[slot] != id)
    {
      new_content[slot] = id;
      reconfigure = true;
    }
}

void
TimerBoxGtkView::set_time_bar(BreakId id,
                              std::string text, TimeBar::ColorId primary_color,
                              int primary_val, int primary_max,
                              TimeBar::ColorId secondary_color,
                              int secondary_val, int secondary_max)
{
  TimeBar *bar = bars[id];
  bar->set_text(text);
  bar->set_bar_color(primary_color);
  bar->set_progress(primary_val, primary_max);
  bar->set_secondary_bar_color(secondary_color);
  bar->set_secondary_progress(secondary_val, secondary_max);
}


void
TimerBoxGtkView::set_tip(string tip)
{
  Gtk::Tooltips *tt = GUI::get_instance()->get_tooltips();
  tt->set_tip(*sheep, tip.c_str());
}

void
TimerBoxGtkView::update()
{
  if (reconfigure)
    {
      init_table();
      reconfigure = false;
    }
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      bars[i]->update();
    }
}
  
void
TimerBoxGtkView::set_enabled(bool enabled)
{
  (void) enabled;
  // Status window disappears, no need to do anything here.
}
