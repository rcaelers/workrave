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

#include <QPushButton>
#include <QLabel>

#include "debug.hh"
#include "commonui/nls.h"

#include "Text.hh"
#include "Ui.hh"
#include "utils/AssetPath.hh"

#include "SizeGroup.hh"
#include "TimerBoxView.hh"

using namespace workrave;
using namespace workrave::utils;

TimerBoxView::TimerBoxView() :
  sheep(NULL),
  reconfigure(true),
  size(0),
  visible_count(-1),
  sheep_only(false)
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      new_content[i] = BREAK_ID_NONE;
      current_content[i] = BREAK_ID_NONE;
      labels[i] = NULL;
      bars[i] = NULL;
    }
  
  init();
}

TimerBoxView::~TimerBoxView()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      delete labels[i];
      delete bars[i];
    }
}

void
TimerBoxView::init()
{
  layout = new QGridLayout();
  layout->setSpacing(2);
  layout->setContentsMargins(2, 2, 2, 2);

  setLayout(layout);

  SizeGroup *size_group = new SizeGroup(Qt::Horizontal | Qt::Vertical);

  for (size_t i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      QPixmap pixmap(Ui::get_break_icon_filename(i));

      if (false) // TODO: i == BREAK_ID_REST_BREAK)
        {
          QPushButton *button = new QPushButton("");
          button->setIcon(pixmap);
          labels[i] = button;
        }
      else
        {
          QLabel *label = new QLabel("");
          label->setPixmap(pixmap);
          labels[i] = label;
        }

      size_group->addWidget(labels[i]);

      bars[i] = new TimeBar();
      bars[i]->set_text_alignment(1);
      bars[i]->set_progress(0, 60);
      bars[i]->set_text(tr("Wait"));
    }

  // TODO: move to UiUtil
  std::string sheep_file = AssetPath::complete_directory("workrave-icon-medium.png", AssetPath::SEARCH_PATH_IMAGES);
  QPixmap pixmap(QString::fromStdString(sheep_file));
  sheep = new QLabel("");
  sheep->setPixmap(pixmap);
  // sheep->set_tooltip_text("Workrave");
}

int
TimerBoxView::get_number_of_timers() const
{
  int number_of_timers = 0;
  if (!sheep_only)
    {
      for (int i = 0; i < BREAK_ID_SIZEOF; i++)
        {
          if (new_content[i] != BREAK_ID_NONE)
            {
              number_of_timers++;
            }
        }
    }
  return number_of_timers;
}

void
TimerBoxView::init_table()
{
  TRACE_ENTER("TimerBoxView::init_table");

  int number_of_timers = get_number_of_timers();
  TRACE_MSG("number_of_timers = " << number_of_timers);

  // Compute table dimensions.
  int columns = 2;
  int reverse = false;

  bool remove_all = number_of_timers != visible_count;

  // Remove old
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      int id = current_content[i];
      if (id != -1 && (id != new_content[i] || remove_all))
        {
          TRACE_MSG("remove " << i << " " << id);
          layout->removeWidget(labels[id]);
          labels[id]->hide();
          layout->removeWidget(bars[id]);
          bars[id]->hide();
          current_content[i] = -1;
        }
    }

  // Remove sheep
  if ((number_of_timers > 0 || remove_all) && visible_count == 0)
    {
      TRACE_MSG("remove sheep");
      layout->removeWidget(sheep);
      sheep->hide();
      visible_count = -1;
    }

  // Add sheep.
  if (number_of_timers == 0 && visible_count != 0)
    {
      TRACE_MSG("add sheep");
      layout->addWidget(sheep, 0, 0);
      sheep->show();
    }


  // Fill table.
  for (int i = 0; i < number_of_timers; i++)
    {
      int id = new_content[i];
      int cid = current_content[i];

      if (id != cid)
        {
          int item = i;

          if (reverse)
            {
              item = number_of_timers - i + 1;
            }

          current_content[i] = id;

          int cur_row = (2 * item) / columns;
          int cur_col = (2 * item) % columns;

          layout->addWidget(labels[id], cur_row, cur_col);
          labels[id]->show();

          int bias = 1;
          if (reverse)
            {
              bias = -1;
            }

          cur_row = (2 * item + bias) / columns;
          cur_col = (2 * item + bias) % columns;

          layout->addWidget(bars[id], cur_row, cur_col);
          bars[id]->show();
        }
    }

  for (int i = number_of_timers; i < BREAK_ID_SIZEOF; i++)
    {
      current_content[i] = -1;
    }

  visible_count = number_of_timers;

  adjustSize();
  TRACE_EXIT();
}

void
TimerBoxView::set_slot(BreakId id, int slot)
{
  if (new_content[slot] != id)
    {
      new_content[slot] = id;
      reconfigure = true;
    }
}

void
TimerBoxView::set_time_bar(BreakId id,
                           int value, TimerColorId primary_color,
                           int primary_val, int primary_max,
                           TimerColorId secondary_color,
                           int secondary_val, int secondary_max)
{
  TRACE_ENTER_MSG("TimerBoxView::set_time_bar", id);

  TRACE_MSG(value);
  TRACE_MSG(primary_val << " " << primary_max << " " << int(primary_color));
  TRACE_MSG(secondary_val << " " << secondary_max <<" " << int(secondary_color));

  TimeBar *bar = bars[id];
  bar->set_text(Text::time_to_string(value));
  bar->set_bar_color(primary_color);
  bar->set_progress(primary_val, primary_max);
  bar->set_secondary_bar_color(secondary_color);
  bar->set_secondary_progress(secondary_val, secondary_max);
  TRACE_EXIT();
}

void
TimerBoxView::set_icon(StatusIconType icon)
{
 QString file = Ui::get_status_icon_filename(icon);
  // TODO:
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
