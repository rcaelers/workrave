// GtkUtil.hh --- Gtk utilities
//
// Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2011 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef GTKUTIL_HH
#define GTKUTIL_HH

#include <gtkmm/button.h>
#include <gtkmm/stockid.h>
#include <gtkmm/table.h>
#include <gtkmm/window.h>
#include <gtkmm/image.h>
#include <gtkmm/box.h>
#include <gtkmm/alignment.h>
#include <glibmm/quark.h>
#include <gtkmm/accelgroup.h>

#include <string>

#include "ICore.hh"

class HeadInfo;
class EventImage;

using namespace workrave;

class GtkUtil
{
public:
  static Gtk::Button *
  create_custom_stock_button(const char *label_text,
                             const Gtk::StockID& stock_id);

  static Gtk::Button *
  create_image_button(const char *label_text, const char *image_file, bool label = true);

  static void
  update_custom_stock_button(Gtk::Button *btn, const char *label_text,
                             const Gtk::StockID& stock_id);

  static Gtk::Widget *
  create_label_with_icon(std::string text, const char *icon);

  static Gtk::Label *
  create_label(std::string text, bool bold);

  static Gtk::Widget *
  create_label_with_tooltip(std::string text,
                            std::string tooltip);

  static EventImage *
  create_image_with_tooltip(std::string file,
                            std::string tooltip);

  static Gtk::Widget *
  create_label_for_break(workrave::BreakId id);

  static Glib::RefPtr<Gdk::Pixbuf>
  flip_pixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, bool horizontal, bool vertical);

  static void
  table_attach_aligned(Gtk::Table &table, Gtk::Widget &child,
                       guint left_attach, guint top_attach, bool left);

  static void
  table_attach_left_aligned(Gtk::Table &table, Gtk::Widget &child,
                            guint left_attach, guint top_attach);

  static void
  table_attach_right_aligned(Gtk::Table &table, Gtk::Widget &child,
                             guint left_attach, guint top_attach);

  static void center_window(Gtk::Window &window, HeadInfo &head);

  static bool has_button_images();

  static void update_mnemonic(Gtk::Widget *widget, Glib::RefPtr<Gtk::AccelGroup>);

  static GtkWindow *GtkUtil::get_visible_tooltip_window();

private:
  static Glib::Quark *label_quark;
};

#endif // GTKMMGUI_HH
