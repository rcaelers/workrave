// Copyright (C) 2001 - 2012 Rob Caelers & Raymond Penners
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

#include <iostream>

#include <gtkmm/image.h>
#include <gtkmm/sizegroup.h>
#include <gtkmm/eventbox.h>

#include "commonui/nls.h"
#include "debug.hh"

#include "EventButton.hh"
#include "TimerBoxGtkView.hh"
#include "TimeBar.hh"
#include "commonui/Text.hh"
#include "GtkUtil.hh"

#include "utils/AssetPath.hh"
#include "ui/GUIConfig.hh"
#include "core/IBreak.hh"

using namespace std;
using namespace workrave;
using namespace workrave::utils;

TimerBoxGtkView::TimerBoxGtkView(std::shared_ptr<workrave::ICore> core, bool transparent)
  : core(core)
  , transparent(transparent)
{
  TRACE_ENTRY();
  init();
}

TimerBoxGtkView::~TimerBoxGtkView()
{
  TRACE_ENTRY();
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (labels[i] != nullptr)
        {
          labels[i]->unreference();
        }

      if (bars[i] != nullptr)
        {
          bars[i]->unreference();
        }
    }
  if (sheep != nullptr)
    {
      sheep->unreference();
    }

  if (sheep_eventbox != nullptr)
    {
      sheep_eventbox->unreference();
    }
}

void
TimerBoxGtkView::set_geometry(Orientation orientation, int size)
{
  TRACE_ENTRY_PAR(orientation, size);
  this->orientation = orientation;
  this->size = size;

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      bars[i]->queue_resize();
    }

  init_table();
}

void
TimerBoxGtkView::init()
{
  TRACE_ENTRY();
  if (sheep != nullptr)
    {
      sheep->unreference();
    }
  if (sheep_eventbox != nullptr)
    {
      sheep_eventbox->unreference();
    }

  sheep_eventbox = new Gtk::EventBox;
  sheep_eventbox->set_events(sheep_eventbox->get_events() | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);

  sheep_eventbox->property_visible_window() = false;

  string sheep_file = AssetPath::complete_directory("workrave-icon-medium.png", AssetPath::SEARCH_PATH_IMAGES);
  sheep = Gtk::manage(new Gtk::Image(sheep_file));
  sheep_eventbox->set_tooltip_text("Workrave");

  sheep_eventbox->add(*sheep);

  sheep->reference();
  sheep_eventbox->reference();

  init_widgets();

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      current_content[i] = new_content[i] = BREAK_ID_NONE;
      labels[i]->reference();
      bars[i]->reference();
    }

  GUIConfig::icon_theme().attach(this, [this](std::string theme) { update_widgets(); });

  reconfigure = true;
}

void
TimerBoxGtkView::init_widgets()
{
  Glib::RefPtr<Gtk::SizeGroup> size_group = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_BOTH);

  const char *icons[] = {"timer-micro-break.png", "timer-rest-break.png", "timer-daily.png"};
  for (int count = 0; count < BREAK_ID_SIZEOF; count++)
    {
      Gtk::Image *img = GtkUtil::create_image(icons[count]);
      Gtk::Widget *w;
      if (count == BREAK_ID_REST_BREAK)
        {
          img->set_padding(0, 0);

          auto *b = new EventButton();
          b->set_relief(Gtk::RELIEF_NONE);
          b->set_border_width(0);
          b->add(*Gtk::manage(img));
          b->set_can_focus(false);

          static const char button_style[] =
            "* {\n"
            "padding-top: 1px;\n"
            "padding-bottom: 1px;\n"
            "}";

          Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
          Glib::RefPtr<Gtk::StyleContext> style_context = b->get_style_context();

          css_provider->load_from_data(button_style);
          style_context->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

          b->set_tooltip_text(_("Take rest break now"));
          b->signal_clicked().connect([this]() { core->force_break(BREAK_ID_REST_BREAK, BreakHint::UserInitiated); });
          w = b;
        }
      else
        {
          w = img;
          img->set_padding(0, 2);
        }

      size_group->add_widget(*w);
      labels[count] = w;
      images[count] = img;

      bars[count] = new TimeBar;
      bars[count]->set_text_alignment(1);
      bars[count]->set_progress(0, 60);
      bars[count]->set_text(_("Wait"));
    }
}

void
TimerBoxGtkView::update_widgets()
{
  const char *icons[] = {"timer-micro-break.png", "timer-rest-break.png", "timer-daily.png"};
  for (int count = 0; count < BREAK_ID_SIZEOF; count++)
    {
      std::string filename = GtkUtil::get_image_filename(icons[count]);
      images[count]->set(filename);
    }
}

int
TimerBoxGtkView::get_number_of_timers() const
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
TimerBoxGtkView::init_table()
{
  TRACE_ENTRY();
  // Compute number of visible breaks.
  int number_of_timers = get_number_of_timers();
  TRACE_MSG("number_of_timers = {}", number_of_timers);

  // Compute table dimensions.
  int rows = number_of_timers;
  int columns = 1;
  bool reverse = false;
  int tsize = size;

  rotation = 0;

  if (rows == 0)
    {
      // Show sheep.
      rows = 1;
    }

  Gtk::Requisition label_size;
  Gtk::Requisition bar_size;
  Gtk::Requisition my_size;

  GtkRequisition natural_size;
  labels[0]->get_preferred_size(label_size, natural_size);
  get_preferred_size(my_size, natural_size);
  TRACE_MSG("my_size = {} {}", my_size.width, my_size.height);
  TRACE_MSG("natural_size = {} {}", natural_size.width, natural_size.height);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      bars[i]->set_rotation(0);
    }

  bars[0]->get_preferred_size(bar_size.width, bar_size.height);
  TRACE_MSG("bar_size = {} {}", bar_size.width, bar_size.height);
  TRACE_MSG("label_size = {} {}", label_size.width, label_size.height);

  if (size == -1 && (orientation == ORIENTATION_LEFT))
    {
      tsize = label_size.width + bar_size.width + 9;
    }

  if (tsize != -1)
    {
      if ((orientation == ORIENTATION_LEFT || orientation == ORIENTATION_RIGHT))
        {
          set_size_request(tsize, -1);
        }
      else
        {
          set_size_request(-1, tsize);
        }
      TRACE_MSG("size request = {}", tsize);
    }

  if (orientation == ORIENTATION_LEFT || orientation == ORIENTATION_RIGHT)
    {
      if (tsize > bar_size.width + label_size.width + 8)
        {
          columns = 2;
          rows = number_of_timers;
        }
      else if (tsize > bar_size.width + 2)
        {
          columns = 1;
          rows = 2 * number_of_timers;
        }
      else
        {
          columns = 1;
          rows = 2 * number_of_timers;

          if (orientation == ORIENTATION_LEFT)
            {
              rotation = 90;
            }
          else
            {
              rotation = 270;
              reverse = true;
            }
        }
      if (rows <= 0)
        {
          TRACE_MSG("too small: rows");
          rows = 1;
        }
    }
  else
    {
      rows = tsize / (bar_size.height);
      if (rows <= 0)
        {
          TRACE_MSG("too small: rows");
          rows = 1;
        }

      columns = 2 * ((number_of_timers + rows - 1) / rows);

      if (columns <= 0)
        {
          columns = 1;
        }
    }

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      bars[i]->set_rotation(rotation);
    }

  TRACE_MSG("c/r {} {} {}", columns, rows, rotation);

  bool remove_all = rows != table_rows || columns != table_columns || number_of_timers != visible_count
                    || reverse != table_reverse;

  // Remove old
  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      int id = current_content[i];
      if (id != -1 && (id != new_content[i] || remove_all))
        {
          TRACE_MSG("remove {} {}", i, id);
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
      remove(*sheep_eventbox);
      visible_count = -1;
    }

  TRACE_VAR(rows, table_rows, columns, table_columns);
  //  if (rows != table_rows || columns != table_columns || number_of_timers != visible_count)
  {
    TRACE_MSG("resize");
    resize(rows, columns);
    set_spacings(0);
    // show_all();

    table_columns = columns;
    table_rows = rows;
    table_reverse = reverse;
  }

  // Add sheep.
  if (number_of_timers == 0 && visible_count != 0)
    {
      TRACE_MSG("add sheep");
      attach(*sheep_eventbox, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
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

          attach(*labels[id], cur_col, cur_col + 1, cur_row, cur_row + 1, Gtk::SHRINK, Gtk::EXPAND);

          int bias = 1;
          if (reverse)
            {
              bias = -1;
            }

          cur_row = (2 * item + bias) / columns;
          cur_col = (2 * item + bias) % columns;

          attach(*bars[id], cur_col, cur_col + 1, cur_row, cur_row + 1, Gtk::FILL | Gtk::EXPAND, Gtk::EXPAND);
        }
    }

  for (int i = number_of_timers; i < BREAK_ID_SIZEOF; i++)
    {
      current_content[i] = -1;
    }

  visible_count = number_of_timers;

  show_all();

  get_preferred_size(my_size, natural_size);
  TRACE_MSG("my_size = {} {}", my_size.width, my_size.height);
  TRACE_MSG("natural_size = {} {}", natural_size.width, natural_size.height);
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
  bar->set_text(Text::time_to_string(value));
  bar->set_bar_color(primary_color);
  bar->set_progress(primary_val, primary_max);
  bar->set_secondary_bar_color(secondary_color);
  bar->set_secondary_progress(secondary_val, secondary_max);
}

void
TimerBoxGtkView::set_icon(OperationModeIcon icon)
{
  string file;
  switch (icon)
    {
    case OperationModeIcon::Normal:
      file = GtkUtil::get_image_filename("workrave-icon-medium.png");
      break;

    case OperationModeIcon::Quiet:
      file = GtkUtil::get_image_filename("workrave-quiet-icon-medium.png");
      break;

    case OperationModeIcon::Suspended:
      file = GtkUtil::get_image_filename("workrave-suspended-icon-medium.png");
      break;
    }

  if (!file.empty())
    {
      sheep->set(file);
    }
}

void
TimerBoxGtkView::update_view()
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
  (void)enabled;
  // Status window disappears, no need to do anything here.
}

void
TimerBoxGtkView::set_sheep_only(bool sheep_only)
{
  TRACE_ENTRY_PAR(sheep_only);
  if (this->sheep_only != sheep_only)
    {
      this->sheep_only = sheep_only;
      reconfigure = true;
      update_view();
    }
}

bool
TimerBoxGtkView::is_sheep_only() const
{
  return sheep_only || get_number_of_timers() == 0;
}

bool
TimerBoxGtkView::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
  if (transparent)
    {
      cr->set_source_rgba(0, 0, 0, 0);
#if CAIROMM_CHECK_VERSION(1, 15, 4)
      cr->set_operator(Cairo::Context::Operator::SOURCE);
#else
      cr->set_operator(Cairo::OPERATOR_SOURCE);
#endif
      cr->paint();
    };

  return Gtk::Widget::on_draw(cr);
}
