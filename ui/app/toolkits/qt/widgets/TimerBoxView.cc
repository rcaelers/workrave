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
#  include "config.h"
#endif

#include <QPushButton>
#include <QLabel>

#include "debug.hh"

#include "Ui.hh"
#include "UiUtil.hh"
#include "utils/AssetPath.hh"

#include "SizeGroup.hh"
#include "TimerBoxView.hh"

using namespace workrave;
using namespace workrave::utils;

TimerBoxView::TimerBoxView()
{
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      new_content[i] = BREAK_ID_NONE;
      current_content[i] = BREAK_ID_NONE;
      labels[i] = nullptr;
      bars[i] = nullptr;
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

  auto *size_group = new SizeGroup(Qt::Horizontal | Qt::Vertical);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      QPixmap pixmap(Ui::get_break_icon_filename(BreakId(i)));

      if (false) // TODO: i == BREAK_ID_REST_BREAK)
        {
          auto *button = new QPushButton("");
          button->setIcon(pixmap);
          labels[i] = button;
        }
      else
        {
          auto *label = new QLabel("");
          label->setPixmap(pixmap);
          labels[i] = label;
        }

      size_group->add_widget(labels[i]);

      bars[i] = new TimeBar();
      bars[i]->set_text_alignment(1);
      bars[i]->set_progress(0, 60);
      bars[i]->set_text(tr("Wait"));
    }

  // TODO: move to UiUtil
  std::string sheep_file = AssetPath::complete_directory("workrave-icon-medium.png", SearchPathId::Images);
  QPixmap pixmap(QString::fromStdString(sheep_file));
  sheep = new QLabel("");
  sheep->setPixmap(pixmap);
  // sheep->set_tooltip_text("Workrave");
}

auto
TimerBoxView::get_number_of_timers() const -> int
{
  int number_of_timers = 0;
  if (!sheep_only)
    {
      for (int timer: new_content)
        {
          if (timer != BREAK_ID_NONE)
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
  TRACE_ENTRY();
  int number_of_timers = get_number_of_timers();
  TRACE_MSG("number_of_timers = {}", number_of_timers);

  // Compute table dimensions.
  int columns = 2;
  int reverse = 0;

  bool remove_all = number_of_timers != visible_count;

  // Remove old
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      int id = current_content[i];
      if (id != -1 && (id != new_content[i] || remove_all))
        {
          TRACE_MSG("remove {} {}", i, id);
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

          if (reverse != 0)
            {
              item = number_of_timers - i + 1;
            }

          current_content[i] = id;

          int cur_row = (2 * item) / columns;
          int cur_col = (2 * item) % columns;

          layout->addWidget(labels[id], cur_row, cur_col);
          labels[id]->show();

          int bias = 1;
          if (reverse != 0)
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
                           int value,
                           TimerColorId primary_color,
                           int primary_val,
                           int primary_max,
                           TimerColorId secondary_color,
                           int secondary_val,
                           int secondary_max)
{
  TRACE_ENTRY_PAR(id);

  TRACE_VAR(value);
  TRACE_VAR(primary_val, primary_max, int(primary_color));
  TRACE_VAR(secondary_val, secondary_max, int(secondary_color));

  TimeBar *bar = bars[id];
  bar->set_text(UiUtil::time_to_string(value));
  bar->set_bar_color(primary_color);
  bar->set_progress(primary_val, primary_max);
  bar->set_secondary_bar_color(secondary_color);
  bar->set_secondary_progress(secondary_val, secondary_max);
}

void
TimerBoxView::set_icon(OperationModeIcon icon)
{
  QString file = Ui::get_status_icon_filename(icon);
  // TODO:
}

void
TimerBoxView::set_geometry(Orientation orientation, int size)
{
}

void
TimerBoxView::update_view()
{
  if (reconfigure)
    {
      init_table();
      reconfigure = false;
    }
  for (auto &b: bars)
    {
      b->update();
    }
}

void
TimerBoxView::set_sheep_only(bool sheep_only)
{
  TRACE_ENTRY_PAR(sheep_only);
  if (this->sheep_only != sheep_only)
    {
      this->sheep_only = sheep_only;
      update_view();
    }
}

auto
TimerBoxView::is_sheep_only() const -> bool
{
  return sheep_only || get_number_of_timers() == 0;
}
