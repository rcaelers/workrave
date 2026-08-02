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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "EventButton.hh"

#if GTK_CHECK_VERSION(4, 0, 0)
EventButton::EventButton()
{
  click_gesture = Gtk::GestureClick::create();
  click_gesture->set_button(0);
  click_gesture->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
  click_gesture->signal_pressed().connect(sigc::mem_fun(*this, &EventButton::on_button_pressed));
  add_controller(click_gesture);
}

void
EventButton::on_button_pressed(int /* n_press */, double /* x */, double /* y */)
{
  bool handled = button_pressed.emit(static_cast<int>(click_gesture->get_current_button()));
  click_gesture->set_state(handled ? Gtk::EventSequenceState::CLAIMED : Gtk::EventSequenceState::DENIED);
}
#else
EventButton::EventButton() = default;

bool
EventButton::on_button_press_event(GdkEventButton *event)
{
  bool handled = button_pressed.emit(event->button);
  bool ret = true;

  if (!handled)
    {
      ret = Gtk::Button::on_button_press_event(event);
    }

  return ret;
}
#endif
