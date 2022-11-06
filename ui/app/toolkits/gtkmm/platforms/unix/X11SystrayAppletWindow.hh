// Copyright (C) 2001 - 2013 Rob Caelers & Raymond Penners
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

#ifndef X11SYSTRAYAPPLETWINDOW_HH
#define X11SYSTRAYAPPLETWINDOW_HH

#include <sigc++/trackable.h>
#include <gtkmm.h>
#include <gtkmm/plug.h>
#include "gtktrayicon.h"

#include "utils/Signals.hh"
#include "ui/UiTypes.hh"
#include "ui/TimerBoxControl.hh"

#include "ui/Plugin.hh"
#include "ui/IPluginContext.hh"
#include "ui/IToolkit.hh"
#include "ToolkitMenu.hh"
#include "TimerBoxGtkView.hh"
#include "ui/AppHold.hh"

class X11SystrayAppletWindow
  : public sigc::trackable
  , public Plugin<X11SystrayAppletWindow>
{
public:
  X11SystrayAppletWindow(std::shared_ptr<IPluginContext> context);
  ~X11SystrayAppletWindow() override;

  std::string get_plugin_id() const override
  {
    return "workrave.X11SystrayAppletWindow";
  }

private:
  std::shared_ptr<IPluginContext> context;
  AppHold apphold;
  std::shared_ptr<ToolkitMenu> menu;

  TimerBoxGtkView *view{nullptr};
  std::shared_ptr<TimerBoxControl> control;

  Gtk::Plug *plug{nullptr};
  Gtk::Bin *container{nullptr};
  WRGtkTrayIcon *tray_icon{nullptr};

  Orientation applet_orientation{ORIENTATION_UP};
  int applet_size{0};
  bool applet_active{false};
  bool embedded{false};
  bool enabled{false};

  workrave::utils::Trackable tracker;

private:
  void activate();
  void deactivate();

  static void static_notify_callback(GObject *gobject, GParamSpec *arg, gpointer user_data);
  void notify_callback();
  void on_menu_restbreak_now();
  void button_clicked(int button);

  // Events.
  void on_embedded();
  bool on_button_press_event(GdkEventButton *event);
  bool on_delete_event(GdkEventAny *);
  void on_size_allocate(Gtk::Allocation &allocation);
  void on_enabled_changed();
};

#endif // X11SYSTRAYAPPLETWINDOW_HH
