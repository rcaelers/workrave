// EventLabel.hh --- Label that receives events.
//
// Copyright (C) 2003, 2004 Rob Caelers <robc@krandor.org>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id$
//

#ifndef EVENTLABEL_HH
#define EVEVTLABEL_HH

#include <gtkmm/label.h>

class EventLabel : public Gtk::Label
{
public:
  EventLabel()
  {
  }
  
  EventLabel(const Glib::ustring& label, bool mnemonic = false) :
    Gtk::Label(label, mnemonic)
  {
  }
  
private:
  void on_realize();
  void on_unrealize();
  bool on_map_event(GdkEventAny *event);
  bool on_unmap_event(GdkEventAny *event);
#ifdef HAVE_GTKMM24
  void on_size_allocate(Gtk::Allocation &allocation);
#else
  void on_size_allocate(GtkAllocation *allocation);
#endif

  GdkWindow *event_window;
};


#endif // EVENTLABEL_HH
