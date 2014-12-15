// AppletPreferencesPanel.hh --- Preferences widgets for a timer
//
// Copyright (C) 2002, 2003, 2005, 2006, 2007, 2011, 2012 Rob Caelers <robc@krandor.nl>
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

#ifndef TIMERBOXPREFERENCEPAGE_HH
#define TIMERBOXPREFERENCEPAGE_HH

class Configurator;
namespace Gtk
{
  class ComboBoxText;
  class SpinButton;
  class CheckButton;
}

#include "ICore.hh"

#include "utils/ScopedConnections.hh"

#include <string>
#include <gtkmm/box.h>

using namespace workrave;
using namespace workrave::config;
using namespace std;

class TimerBoxPreferencePage
  : public Gtk::HBox
{
public:
  TimerBoxPreferencePage(string name);
  ~TimerBoxPreferencePage();

private:
  void create_page();
  void init_page_values();
  void init_page_callbacks();
  void enable_buttons();
  void on_enabled_toggled();
  void on_place_changed();
  void on_display_changed(int break_id);
  void on_cycle_time_changed();
  void on_always_on_top_toggled();

  string name;

  Gtk::CheckButton *ontop_cb;
  Gtk::CheckButton *enabled_cb;
  Gtk::ComboBoxText *place_button;
  Gtk::ComboBoxText *timer_display_button[BREAK_ID_SIZEOF];
  Gtk::SpinButton *cycle_entry;
  scoped_connections connections;
};

#ifndef GTKMM_CHECK_VERSION
# define GTKMM_CHECK_VERSION(major, minor, micro) \
   (GTKMM_MAJOR_VERSION > (major)                     \
    || (GTKMM_MAJOR_VERSION == (major)                \
        && (GTKMM_MINOR_VERSION > (minor)             \
            || (GTKMM_MINOR_VERSION == (minor)        \
                && GTKMM_MICRO_VERSION >= (micro)))))
#endif

#endif // TIMERBOXPREFERENCEPAGE_HH
