// Copyright (C) 2003 - 2012 Raymond Penners <raymond@dotsphinx.com>
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

#include <gtkmm.h>

#include <string>

#include "ui/IApplicationContext.hh"
#include "core/ICore.hh"
#include "ui/prefwidgets/gtkmm/BoxWidget.hh"

class HeadInfo;
class EventImage;

class GtkUtil
{
public:
  static Gtk::Button *create_custom_stock_button(const char *label_text, const char *icon);

  static Gtk::Button *create_image_button(const char *label_text, const char *image_file, bool label = true);

  static void update_custom_stock_button(Gtk::Button *btn, const char *label_text, const char *icon);

  static Gtk::Widget *create_label_with_icon(std::string text, const char *icon);

  static Gtk::Label *create_label(std::string text, bool bold);

  static Gtk::Widget *create_label_with_tooltip(std::string text, std::string tooltip);

  static EventImage *create_image_with_tooltip(std::string file, std::string tooltip);

  static Gtk::Widget *create_label_for_break(workrave::BreakId id);

  static Glib::RefPtr<Gdk::Pixbuf> flip_pixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, bool horizontal, bool vertical);

  static void table_attach_aligned(Gtk::Table &table, Gtk::Widget &child, guint left_attach, guint top_attach, bool left);

  static void table_attach_left_aligned(Gtk::Table &table, Gtk::Widget &child, guint left_attach, guint top_attach);

  static void table_attach_right_aligned(Gtk::Table &table, Gtk::Widget &child, guint left_attach, guint top_attach);

  static void center_window(Gtk::Window &window, HeadInfo &head);

  static std::pair<int, int> get_centered_position(Gtk::Window &window, HeadInfo &head);

  static bool has_button_images();

  static void update_mnemonic(Gtk::Widget *widget, Glib::RefPtr<Gtk::AccelGroup>);

  static GtkWindow *get_visible_tooltip_window();

  static void set_theme_fg_color(Gtk::Widget *widget);

  static std::string get_image_filename(const std::string &image);

  static Glib::RefPtr<Gdk::Pixbuf> create_pixbuf(const std::string &name);

  static Gtk::Image *create_image(const std::string &name);

  static void set_always_on_top(Gtk::Window *window, bool ontop);

  static void add_plugin_widgets(std::shared_ptr<IApplicationContext> app,
                                 std::shared_ptr<ui::prefwidgets::gtkmm::BoxWidget> frame);

private:
  static Glib::Quark *label_quark;
};

// clang-format off
#define GLIBMM_CHECK_VERSION(major,minor,micro)                            \
    (GLIBMM_MAJOR_VERSION > (major) ||                                     \
    (GLIBMM_MAJOR_VERSION == (major) && GLIBMM_MINOR_VERSION > (minor)) ||  \
    (GLIBMM_MAJOR_VERSION == (major) && GLIBMM_MINOR_VERSION == (minor) &&  \
     GLIBMM_MICRO_VERSION >= (micro)))

#define CAIROMM_CHECK_VERSION(major,minor,micro)                            \
    (CAIROMM_MAJOR_VERSION > (major) ||                                     \
    (CAIROMM_MAJOR_VERSION == (major) && CAIROMM_MINOR_VERSION > (minor)) ||  \
    (CAIROMM_MAJOR_VERSION == (major) && CAIROMM_MINOR_VERSION == (minor) &&  \
     CAIROMM_MICRO_VERSION >= (micro)))
// clang-format on

#endif // GTKUTIL_HH
