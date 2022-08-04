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

#ifndef HIG_HH
#define HIG_HH

#include <gtkmm.h>

#if GTK_CHECK_VERSION(4,0,0)
namespace Gtk
{
  class HBox: public Gtk::Box
  {
    public:
      HBox()
      {
        set_orientation(Gtk::Orientation::HORIZONTAL);
      }

  };

  class VBox: public Gtk::Box
  {
    public:
      VBox()
      {
        set_orientation(Gtk::Orientation::VERTICAL);
      }

  };
}
#endif

class HigDialog : public Gtk::Dialog
{
public:
  HigDialog();
  HigDialog(const Glib::ustring &title, bool modal = false, bool use_separator = false);
  Gtk::VBox *get_vbox();

private:
  void set_hig_defaults();

  Gtk::VBox *vbox{nullptr};
};

class HigCategoryPanel : public Gtk::VBox
{
public:
  HigCategoryPanel(Gtk::Widget &lab, bool fill = false);
  HigCategoryPanel(const char *lab, bool fill = false);
  HigCategoryPanel();
  Gtk::Label *add_label(const char *lab, Gtk::Widget &widget, bool expand = false, bool fill = false);
  void add_label(Gtk::Label &label, Gtk::Widget &widget, bool expand = false, bool fill = false);
  void add_widget(Gtk::Widget &widget, bool expand = false, bool fill = false);

  void add_caption(Gtk::Widget &lab);
  void add_caption(const char *lab);

private:
  void init(Gtk::Widget &lab, bool fill = false);

  Gtk::VBox *options_box{nullptr};
  Glib::RefPtr<Gtk::SizeGroup> size_group;
};

class HigCategoriesPanel : public Gtk::VBox
{
public:
  HigCategoriesPanel();
  void add(Gtk::Widget &panel)
#if !GTK_CHECK_VERSION(4,0,0)
    override
#endif
    ;
};

class HigUtil
{
public:
  static Glib::ustring create_alert_text(const char *caption, const char *body);
};

#endif // HIG_HH
