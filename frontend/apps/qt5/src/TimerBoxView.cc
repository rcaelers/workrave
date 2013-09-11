// TimerBoxGtkView.cc --- Timers Widgets
//
// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#include "debug.hh"
#include "Util.hh"
#include "nls.h"

#include "TimerBoxView.hh"

using namespace workrave;

//! Constructor.
TimerBoxView::TimerBoxView() :
  sheep_only(false),
  reconfigure(false)
{
  init();
}



//! Destructor.
TimerBoxView::~TimerBoxView()
{
  TRACE_ENTER("TimerBoxView::~TimerBoxView");

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      delete labels[i];
      delete bars[i];
    }

  TRACE_EXIT();
}



//! Sets the geometry of the timerbox.
void
TimerBoxView::set_geometry(Orientation orientation, int size)
{
  TRACE_ENTER_MSG("TimerBoxView::set_geometry", orientation << " " << size);
  TRACE_EXIT();
}


//! Initializes the timerbox.
void
TimerBoxView::init()
{
  TRACE_ENTER("TimerBoxView::init");

  layout = new QGridLayout();
  setLayout(layout);
  
  init_widgets();

  for (auto &elem : content)
    {
      elem = BREAK_ID_NONE;
    }

  TRACE_EXIT();
}


//! Initializes the widgets.
void
TimerBoxView::init_widgets()
{
  for (size_t i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      labels[i] = new QLabel("foo");
      bars[i] = new TimeBar();

      bars[i]->set_text_alignment(1);
      bars[i]->set_progress(0, 60);
      bars[i]->set_text(_("Wait"));
      
    }

  //const char *icons[] = { "timer-micro-break.png", "timer-rest-break.png", "timer-daily.png" };
  //for (int count = 0; count < BREAK_ID_SIZEOF; count++)
  //  {
  //    string icon = Util::complete_directory(string(icons[count]), Util::SEARCH_PATH_IMAGES);
  //    Image *img = new Image(icon);
  //    QWidget *w;
  //    if (count == BREAK_ID_REST_BREAK)
  //      {
  //        img->set_padding(0,0);
  //
  //        EventButton *b = new EventButton();
  //        b->set_relief(::RELIEF_NONE);
  //        b->set_border_width(0);
  //        b->add(*::manage(img));
  //
  //        b->set_tooltip_text(_("Take rest break now"));
  //
  //        w = b;
  //      }
  //    else
  //      {
  //        w = img;
  //        img->set_padding(0,2);
  //      }
  //
  //    size_group->add_widget(*w);
  //    labels[count] = w;
  //
  //    bars[count] = new TimeBar;
  //    bars[count]->set_text_alignment(1);
  //    bars[count]->set_progress(0, 60);
  //    bars[count]->set_text(_("Wait"));
  //  }
}


int
TimerBoxView::get_number_of_timers() const
{
  int number_of_timers = 0;
  if (!sheep_only)
    {
      for (auto &elem : content)
        {
          if (elem != BREAK_ID_NONE)
            {
              number_of_timers++;
            }
        }
    }
  return number_of_timers;
}


//! Initializes the applet.
void
TimerBoxView::init_table()
{
  TRACE_ENTER("TimerBoxView::init_table");

  int number_of_timers = get_number_of_timers();
  TRACE_MSG("number_of_timers = " << number_of_timers);

  // Fill table.
  for (int i = 0; i < number_of_timers; i++)
    {
      layout->addWidget(labels[i], i, 0);
      layout->addWidget(bars[i], i, 1);
    }

  TRACE_EXIT();
}


void
TimerBoxView::set_slot(BreakId id, int slot)
{
  if (content[slot] != id)
    {
      content[slot] = id;
      reconfigure = true;
    }
}

void
TimerBoxView::set_time_bar(BreakId id,
                           std::string text, ITimeBar::ColorId primary_color,
                           int primary_val, int primary_max,
                           ITimeBar::ColorId secondary_color,
                           int secondary_val, int secondary_max)
{
  TRACE_ENTER_MSG("TimerBoxView::set_time_bar", id);

  TRACE_MSG(text);
  TRACE_MSG(primary_val << " " << primary_max << " " << int(primary_color));
  TRACE_MSG(secondary_val << " " << secondary_max <<" " << int(secondary_color));

  TimeBar *bar = bars[id];
  bar->set_text(text);
  bar->set_bar_color(primary_color);
  bar->set_progress(primary_val, primary_max);
  bar->set_secondary_bar_color(secondary_color);
  bar->set_secondary_progress(secondary_val, secondary_max);
  TRACE_EXIT();
}


void
TimerBoxView::set_tip(std::string tip)
{
}


void
TimerBoxView::set_icon(IconType icon)
{
  std::string file;
  switch (icon)
    {
    case ICON_NORMAL:
      file = Util::complete_directory("workrave-icon-medium.png",
                                      Util::SEARCH_PATH_IMAGES);
      break;

    case ICON_QUIET:
      file = Util::complete_directory("workrave-quiet-icon-medium.png",
                                             Util::SEARCH_PATH_IMAGES);
      break;

    case ICON_SUSPENDED:
      file = Util::complete_directory("workrave-suspended-icon-medium.png",
                                      Util::SEARCH_PATH_IMAGES);
      break;
    }

  // if (file != "")
  //   {
  //     sheep->set(file);
  //   }
}

void
TimerBoxView::update_view()
{
  if (reconfigure)
    {
      init_table();
      reconfigure = false;
    }
  for (auto &b : bars)
    {
      b->update();
    }
}

void
TimerBoxView::set_enabled(bool enabled)
{
  (void) enabled;
  // Status window disappears, no need to do anything here.
}

void
TimerBoxView::set_sheep_only(bool sheep_only)
{
  TRACE_ENTER_MSG("TimerBoxView::set_sheep_only", sheep_only);
  if (this->sheep_only != sheep_only)
    {
      this->sheep_only = sheep_only;
      update_view();
    }
  TRACE_EXIT();
}

bool
TimerBoxView::is_sheep_only() const
{
  return sheep_only || get_number_of_timers() == 0;
}
