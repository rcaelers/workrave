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

#ifndef WORKRAVE_UI_GTK_COMPAT_HH
#define WORKRAVE_UI_GTK_COMPAT_HH

#include <gtkmm.h>

namespace GtkCompat
{
#if GTK_CHECK_VERSION(4, 0, 0)
  class Box : public Gtk::Box
  {
  public:
    using Gtk::Box::Box;
    // Orientation orientation =  Orientation::HORIZONTAL, int spacing =  0

    void pack_start(Gtk::Widget &child, bool expand, bool fill, guint padding = 0)
    {
      prepend(child);
      child.property_hexpand() = expand;
      child.property_halign() = fill ? Gtk::Align::FILL : Gtk::Align::END;
    }

    void pack_end(Widget &child, bool expand, bool fill, guint padding = 0)
    {
      append(child);
      child.property_hexpand() = expand;
      child.property_halign() = fill ? Gtk::Align::FILL : Gtk::Align::END;
    }
  };

  class ButtonBox : public Box
  {
  };

  class HButtonBox : public Box
  {
  };

#else
  using Box = ::Gtk::Box;
  using Buttonbox = ::Gtk::ButtonBox;
  using HButtonbox = ::Gtk::HButtonBox;
#endif
} // namespace GtkCompat

#endif // WORKRAVE_UI_GTK_COMPAT_HH
