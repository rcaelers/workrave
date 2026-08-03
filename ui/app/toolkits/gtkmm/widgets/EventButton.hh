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

#ifndef EVENTBUTTON_HH
#define EVENTBUTTON_HH

#include <gtkmm/button.h>
#if GTK_CHECK_VERSION(4, 0, 0)
#  include <gtkmm/gestureclick.h>
#endif

class EventButton : public Gtk::Button
{
public:
  using pressed_signal_t = sigc::signal<bool(int)>;

  EventButton();

  pressed_signal_t button_pressed;

private:
#if GTK_CHECK_VERSION(4, 0, 0)
  void on_button_pressed(int n_press, double x, double y);

  Glib::RefPtr<Gtk::GestureClick> click_gesture;
#else
  bool on_button_press_event(GdkEventButton *event) override;
#endif
};

#endif // EVENTBUTTON_HH
