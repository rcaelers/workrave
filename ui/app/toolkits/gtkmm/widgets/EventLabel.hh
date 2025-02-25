// Copyright (C) 2003, 2004, 2007, 2008 Rob Caelers <robc@krandor.nl>
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

#ifndef EVENTLABEL_HH
#define EVENTLABEL_HH

#include <gtkmm/label.h>

class EventLabel : public Gtk::Label
{
public:
  EventLabel() = default;

  EventLabel(const Glib::ustring &label, bool mnemonic = false)
    : Gtk::Label(label, mnemonic)
  {
  }

private:
  void on_realize() override;
  void on_unrealize() override;
  bool on_map_event(GdkEventAny *event) override;
  bool on_unmap_event(GdkEventAny *event) override;
  void on_size_allocate(Gtk::Allocation &allocation) override;

  GdkWindow *event_window{nullptr};
};

#endif // EVENTLABEL_HH
