// TimerBoxGtkView.cc --- Timers Widgets
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2011, 2012 Rob Caelers & Raymond Penners
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

#include "preinclude.h"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <iostream>

#include <gtkmm/image.h>
#include <gtkmm/sizegroup.h>
#include <gtkmm/eventbox.h>

#include "nls.h"
#include "debug.hh"

#include "EventButton.hh"
#include "TimerBoxGtkView.hh"
#include "TimeBar.hh"
#include "utils/Util.hh"
#include "commonui/Text.hh"
#include "Menus.hh"
#include "GUI.hh"
#include "GtkUtil.hh"

#include "core/CoreFactory.hh"
#include "config/IConfigurator.hh"
#include "core/IBreak.hh"

using namespace std;

//! Constructor.
TimerBoxGtkView::TimerBoxGtkView(Menus::MenuKind menu, bool transparent)
  : menu(menu)
  , transparent(transparent)
{
  init();
}

//! Destructor.
TimerBoxGtkView::~TimerBoxGtkView()
{
  TRACE_ENTER("TimerBoxGtkView::~TimerBoxGtkView");
  IConfigurator *config = CoreFactory::get_configurator();
  config->remove_listener(this);

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      if (labels[i] != nullptr)
        labels[i]->unreference();
      delete labels[i];

      if (bars[i] != nullptr)
        bars[i]->unreference();
      delete bars[i];
    }

  if (sheep != nullptr)
    sheep->unreference();

  if (sheep_eventbox != nullptr)
    {
      sheep_eventbox->unreference();
      // FIXME: check if this is needed/Okay.
      delete sheep_eventbox;
    }
  TRACE_EXIT();
}

//! Sets the geometry of the timerbox.
void
TimerBoxGtkView::set_geometry(Orientation orientation, int size)
{
  TRACE_ENTER_MSG("TimerBoxGtkView::set_geometry", orientation << " " << size);
  this->orientation = orientation;
  this->size = size;

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      bars[i]->queue_resize();
    }

  init_table();
  TRACE_EXIT();
}

//! Initializes the timerbox.
void
TimerBoxGtkView::init()
{
  TRACE_ENTER("TimerBoxGtkView::init");

  if (sheep != nullptr)
    sheep->unreference();
  if (sheep_eventbox != nullptr)
    sheep_eventbox->unreference();

  sheep_eventbox = new Gtk::EventBox;
  sheep_eventbox->set_events(sheep_eventbox->get_events() | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);

  sheep_eventbox->property_visible_window() = false;

  string sheep_file = Util::complete_directory("workrave-icon-medium.png", Util::SEARCH_PATH_IMAGES);
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

  IConfigurator *config = CoreFactory::get_configurator();
  config->add_listener(GUIConfig::CFG_KEY_ICONTHEME, this);

  reconfigure = true;
  TRACE_EXIT();
}

//! Initializes the widgets.
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

          EventButton *b = new EventButton();
          b->set_relief(Gtk::RELIEF_NONE);
          b->set_border_width(0);
          b->add(*Gtk::manage(img));

#if GTK_CHECK_VERSION(3, 0, 0)
          static const char button_style[] =
            "* {\n"
            "padding-top: 1px;\n"
            "padding-bottom: 1px;\n"
            "}";

          Glib::RefPtr<Gtk::CssProvider> css_provider = Gtk::CssProvider::create();
          Glib::RefPtr<Gtk::StyleContext> style_context = b->get_style_context();

          css_provider->load_from_data(button_style);
          style_context->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
#endif

          b->set_tooltip_text(_("Take rest break now"));

          IGUI *gui = GUI::get_instance();
          Menus *menus = gui->get_menus();

          b->signal_clicked().connect(sigc::mem_fun(*menus, &Menus::on_menu_restbreak_now));
          b->button_pressed.connect(sigc::mem_fun(*this, &TimerBoxGtkView::on_restbreak_button_press_event));
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

//! Update the widgets.
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

//! Initializes the applet.
void
TimerBoxGtkView::init_table()
{
  TRACE_ENTER("TimerBoxGtkView::init_table");

  // Compute number of visible breaks.
  int number_of_timers = get_number_of_timers();
  TRACE_MSG("number_of_timers = " << number_of_timers);

  // Compute table dimensions.
  int rows = number_of_timers;
  int columns = 1;
  int reverse = false;
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

#ifdef HAVE_GTK3
  GtkRequisition natural_size;
  labels[0]->get_preferred_size(label_size, natural_size);
  get_preferred_size(my_size, natural_size);
  TRACE_MSG("my_size = " << my_size.width << " " << my_size.height);
  TRACE_MSG("natural_size = " << natural_size.width << " " << natural_size.height);
#else
  labels[0]->size_request(label_size);
  size_request(my_size);
#endif

  for (int i = 0; i < BREAK_ID_SIZEOF; i++)
    {
      bars[i]->set_rotation(0);
    }

  bars[0]->get_preferred_size(bar_size.width, bar_size.height);
  TRACE_MSG("bar_size = " << bar_size.width << " " << bar_size.height);
  TRACE_MSG("label_size = " << label_size.width << " " << label_size.height);

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
      TRACE_MSG("size request = " << tsize);
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

  TRACE_MSG("c/r " << columns << " " << rows << " " << rotation);

  bool remove_all = rows != table_rows || columns != table_columns || number_of_timers != visible_count || reverse != table_reverse;

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
      remove(*sheep_eventbox);
      visible_count = -1;
    }

  TRACE_MSG(rows << " " << table_rows << " " << columns << " " << table_columns);
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

#ifdef HAVE_GTK3
  get_preferred_size(my_size, natural_size);
  TRACE_MSG("my_size = " << my_size.width << " " << my_size.height);
  TRACE_MSG("natural_size = " << natural_size.width << " " << natural_size.height);
#endif

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
                              std::string text,
                              TimeBar::ColorId primary_color,
                              int primary_val,
                              int primary_max,
                              TimeBar::ColorId secondary_color,
                              int secondary_val,
                              int secondary_max)
{
  TRACE_ENTER_MSG("TimerBoxGtkView::set_time_bar", id);

  TRACE_MSG(text);
  TRACE_MSG(primary_val << " " << primary_max << " " << int(primary_color));
  TRACE_MSG(secondary_val << " " << secondary_max << " " << int(secondary_color));

  TimeBar *bar = bars[id];
  bar->set_text(text);
  bar->set_bar_color(primary_color);
  bar->set_progress(primary_val, primary_max);
  bar->set_secondary_bar_color(secondary_color);
  bar->set_secondary_progress(secondary_val, secondary_max);
  TRACE_EXIT();
}

void
TimerBoxGtkView::set_tip(string tip)
{
  sheep_eventbox->set_tooltip_text(tip.c_str());
}

void
TimerBoxGtkView::set_icon(IconType icon)
{
  string file;
  switch (icon)
    {
    case ICON_NORMAL:
      file = GtkUtil::get_image_filename("workrave-icon-medium.png");
      break;

    case ICON_QUIET:
      file = GtkUtil::get_image_filename("workrave-quiet-icon-medium.png");
      break;

    case ICON_SUSPENDED:
      file = GtkUtil::get_image_filename("workrave-suspended-icon-medium.png");
      break;
    }

  if (file != "")
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
  TRACE_ENTER_MSG("TimerBoxGtkView::set_sheep_only", sheep_only);
  if (this->sheep_only != sheep_only)
    {
      this->sheep_only = sheep_only;
      reconfigure = true;
      update_view();
    }
  TRACE_EXIT();
}

bool
TimerBoxGtkView::is_sheep_only() const
{
  return sheep_only || get_number_of_timers() == 0;
}

//! User pressed some mouse button in the main window.
bool
TimerBoxGtkView::on_restbreak_button_press_event(int button)
{
  bool ret = false;

  if (button == 3 && menu != Menus::MENU_NONE)
    {
      IGUI *gui = GUI::get_instance();
      Menus *menus = gui->get_menus();
      menus->popup(menu, 0 /*event->button */, 0);
      ret = true;
    }

  return ret;
}

#ifdef HAVE_GTK3
bool
TimerBoxGtkView::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
  if (transparent)
    {
      cr->set_source_rgba(0, 0, 0, 0);
      cr->set_operator(Cairo::OPERATOR_SOURCE);
      cr->paint();
    };

  return Gtk::Widget::on_draw(cr);
}
#endif

void
TimerBoxGtkView::config_changed_notify(const string &key)
{
  TRACE_ENTER_MSG("MainWindow::config_changed_notify", key);
  if (key == GUIConfig::CFG_KEY_ICONTHEME)
    {
      update_widgets();
    }
  TRACE_EXIT();
}
