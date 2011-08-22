// GenericDBusApplet.hh --- X11 Applet Window
//
// Copyright (C) 2001 - 2009, 2011 Rob Caelers & Raymond Penners
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

#ifndef GENERICDBUSAPPLET_HH
#define GENERICDBUSAPPLET_HH

#include "config.h"

#include <string>
#include <stdio.h>

#include <gtk/gtk.h>

#include "ITimerBoxView.hh"
#include "ITimeBar.hh"
#include "AppletWindow.hh"
#include "Menus.hh"
#include "Menu.hh"

class AppletControl;

class GenericDBusApplet : public AppletWindow , public ITimerBoxView, public Menu
{
public:
  struct TimerData
  {
    std::string bar_text;
    int slot;
    int bar_secondary_color;
    int bar_secondary_val;
    int bar_secondary_max;
    int bar_primary_color;
    int bar_primary_val;
    int bar_primary_max;
  };

  enum MenuItemFlags
    {
      MENU_ITEM_FLAG_NONE = 0,
      MENU_ITEM_FLAG_SUBMENU_BEGIN = 1,
      MENU_ITEM_FLAG_SUBMENU_END = 2,
      MENU_ITEM_FLAG_CHECK = 4,
      MENU_ITEM_FLAG_RADIO = 8,
      MENU_ITEM_FLAG_ACTIVE = 16,
    };

  struct MenuItem
  {
    std::string text;
    int command;
    int flags;
    int group;
  };

  typedef std::list<MenuItem> MenuItems;
 
  GenericDBusApplet(AppletControl *control);
  virtual ~GenericDBusApplet();

  virtual AppletState activate_applet();
  virtual void deactivate_applet();

  virtual void set_applet_enabled(bool enable);
  virtual void get_menu_items(MenuItems &out) const;
  virtual void applet_command(int command);

  virtual void set_slot(BreakId  id, int slot);
  virtual void set_time_bar(BreakId id,
                            std::string text,
                            ITimeBar::ColorId primary_color,
                            int primary_value, int primary_max,
                            ITimeBar::ColorId secondary_color,
                            int secondary_value, int secondary_max);
  virtual void set_tip(std::string tip);
  virtual void set_icon(IconType icon);
  virtual void update_view();
  virtual void set_enabled(bool enabled);
  virtual void set_geometry(Orientation orientation, int size);

  
private:
  virtual void init();
  virtual void add_accel(Gtk::Window &window);
  virtual void popup(const guint button, const guint activate_time);
  virtual void resync(workrave::OperationMode mode, workrave::UsageMode usage, bool show_log);
  
  void add_menu_item(const char *text, int command, int flags);
  
private:
  bool enabled;
  TimerData data[BREAK_ID_SIZEOF];
  MenuItems items;

  //! Controller
  AppletControl *control;
};

#endif // GENERICDBUSAPPLET_HH
