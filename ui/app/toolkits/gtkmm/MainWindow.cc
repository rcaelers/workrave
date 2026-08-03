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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#if defined(PLATFORM_OS_WINDOWS)
#  include <gdk/gdkwin32.h>
#  include <shellapi.h>
#  undef ERROR
#  undef IN
#  undef OUT
#  undef WINDING
#endif

#include "debug.hh"

#include "MainWindow.hh"

#include <list>
#include <gtkmm.h>

#include "TimerBoxGtkView.hh"
#include "ui/TimerBoxControl.hh"
#include "ui/GUIConfig.hh"
#include "GtkUtil.hh"

#include "IToolkitPrivate.hh"
#include "ToolkitMenu.hh"
#include "commonui/MenuDefs.hh"

using namespace std;

MainWindow::MainWindow(std::shared_ptr<IApplicationContext> app)
  : app(app)
{
  init();
}

MainWindow::~MainWindow()
{
  TRACE_ENTRY();
  delete timer_box_control;

#if defined(PLATFORM_OS_UNIX) && !GTK_CHECK_VERSION(4, 0, 0)
  delete leader;
#endif
}

void
MainWindow::open_window()
{
  TRACE_ENTRY();
  // Head count is 0, when monitor is powered off due to sleeping on Sway.
  if (timer_box_view->get_visible_count() > 0 && app->get_toolkit()->get_head_count() > 0)
    {
      GtkCompat::show_all(*this);

#if GTK_CHECK_VERSION(4, 0, 0)
      present();
      // Gtk::Window::stick(), set_position()/set_gravity() and move() were
      // all removed in GTK4; the compositor owns window placement.
#else
      stick();
      deiconify();

      set_position(Gtk::WIN_POS_NONE);
      set_gravity(Gdk::GRAVITY_NORTH_WEST);

      move_to_start_position();
#endif

      GUIConfig::timerbox_enabled("main_window").set(true);

      bool always_on_top = GUIConfig::main_window_always_on_top()();
      GtkUtil::set_always_on_top(this, always_on_top);

#if !GTK_CHECK_VERSION(4, 0, 0)
      if (always_on_top)
        {
          raise();
        }
#endif
    }
}

void
MainWindow::close_window()
{
  TRACE_ENTRY();
  if (can_close)
    {
      TRACE_MSG("hide");
      hide();
    }
  else
    {
      TRACE_MSG("iconify");
#if GTK_CHECK_VERSION(4, 0, 0)
      minimize();
#else
      iconify();
#endif
    }

  GUIConfig::timerbox_enabled("main_window").set(false);
}

void
MainWindow::set_can_close(bool can_close)
{
  TRACE_ENTRY_PAR(can_close);
  this->can_close = can_close;

  TRACE_VAR("enabled", enabled);
  TRACE_VAR("visible", is_visible());

  if (!can_close && !is_visible())
    {
      open_window();
    }
}

void
MainWindow::update()
{
  timer_box_control->update();
}

void
MainWindow::init()
{
  TRACE_ENTRY();

  GtkCompat::set_border_width(*this, 2);
  set_resizable(false);
#if !GTK_CHECK_VERSION(4, 0, 0)
  set_gravity(Gdk::GRAVITY_NORTH_WEST);
  set_position(Gtk::WIN_POS_NONE);
#endif
  set_title("Workrave");

#if GTK_CHECK_VERSION(4, 0, 0)
  // GTK4 dropped the pixbuf-list based icon API in favour of icon-theme
  // lookups; the "workrave" icon is installed into the hicolor theme at
  // the same sizes the pixbuf list used to load individually.
  Gtk::Window::set_default_icon_name("workrave");
#else
#  if GLIBMM_CHECK_VERSION(2, 68, 0)
  std::vector<Glib::RefPtr<Gdk::Pixbuf>> icons;
#  else
  std::list<Glib::RefPtr<Gdk::Pixbuf>> icons;
#  endif

  const char *icon_files[] = {
#  if !defined(PLATFORM_OS_WINDOWS)
    // This causes a crash on windows
    "scalable" G_DIR_SEPARATOR_S "apps" G_DIR_SEPARATOR_S "workrave.svg",
    "16x16" G_DIR_SEPARATOR_S "apps" G_DIR_SEPARATOR_S "workrave.png",
    "24x24" G_DIR_SEPARATOR_S "apps" G_DIR_SEPARATOR_S "workrave.png",
    "32x32" G_DIR_SEPARATOR_S "apps" G_DIR_SEPARATOR_S "workrave.png",
    "48x48" G_DIR_SEPARATOR_S "apps" G_DIR_SEPARATOR_S "workrave.png",
    "64x64" G_DIR_SEPARATOR_S "apps" G_DIR_SEPARATOR_S "workrave.png",
    "96x96" G_DIR_SEPARATOR_S "apps" G_DIR_SEPARATOR_S "workrave.png",
    "128x128" G_DIR_SEPARATOR_S "apps" G_DIR_SEPARATOR_S "workrave.png",
#  else
    "16x16" G_DIR_SEPARATOR_S "workrave.png",
    "24x24" G_DIR_SEPARATOR_S "workrave.png",
    "32x32" G_DIR_SEPARATOR_S "workrave.png",
    "48x48" G_DIR_SEPARATOR_S "workrave.png",
    "64x64" G_DIR_SEPARATOR_S "workrave.png",
    "96x96" G_DIR_SEPARATOR_S "workrave.png",
    "128x128" G_DIR_SEPARATOR_S "workrave.png",
#  endif
  };

  for (auto &icon_file: icon_files)
    {
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = GtkUtil::create_pixbuf(icon_file);
      if (pixbuf)
        {
          icons.push_back(pixbuf);
        }
    }

#  if GLIBMM_CHECK_VERSION(2, 68, 0)
  Gtk::Window::set_default_icon_list(icons);
#  else
  Glib::ListHandle<Glib::RefPtr<Gdk::Pixbuf>> icon_list(icons);
  Gtk::Window::set_default_icon_list(icon_list);
#  endif
#endif

  timer_box_view = Gtk::manage(new TimerBoxGtkView(app->get_core()));
  timer_box_control = new TimerBoxControl(app->get_core(), "main_window", timer_box_view);
  timer_box_view->set_geometry(ORIENTATION_HORIZONTAL, -1);
  timer_box_control->update();

#if GTK_CHECK_VERSION(4, 0, 0)
  timer_view_eventbox = Gtk::manage(new GtkCompat::EventBox);
  timer_view_eventbox->add(*timer_box_view);
  GtkCompat::set_child(*this, *timer_view_eventbox);

  timer_view_click_gesture = Gtk::GestureClick::create();
  timer_view_click_gesture->set_button(0);
  timer_view_click_gesture->signal_pressed().connect(sigc::mem_fun(*this, &MainWindow::on_timer_view_button_press));
  timer_view_eventbox->add_controller(timer_view_click_gesture);
#else
  auto *eventbox = Gtk::manage(new Gtk::EventBox);
  eventbox->set_visible_window(false);
  eventbox->set_events(eventbox->get_events() | Gdk::BUTTON_PRESS_MASK);
  eventbox->signal_button_press_event().connect(sigc::mem_fun(*this, &MainWindow::on_timer_view_button_press_event), false);
  eventbox->add(*timer_box_view);
  add(*eventbox);

  realize_if_needed();
  Glib::RefPtr<Gdk::Window> window = get_window();
  window->set_decorations(Gdk::DECOR_BORDER | Gdk::DECOR_TITLE | Gdk::DECOR_MENU);
#endif

#if defined(PLATFORM_OS_WINDOWS) && !GTK_CHECK_VERSION(4, 0, 0)
  HWND hwnd = (HWND)GDK_WINDOW_HWND(gtk_widget_get_window(Gtk::Widget::gobj()));
  SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX));
#endif

#if defined(PLATFORM_OS_UNIX) && !GTK_CHECK_VERSION(4, 0, 0)
  // HACK. this sets a different group leader in the WM_HINTS....
  // Without this hack, metacity makes ALL windows on-top.
  leader = new Gtk::Window(Gtk::WINDOW_POPUP);
  gtk_widget_realize(GTK_WIDGET(leader->gobj()));
  Glib::RefPtr<Gdk::Window> leader_window = leader->get_window();
  window->set_group(leader_window);
#endif

  menu = std::make_shared<ToolkitMenu>(app->get_menu_model(),
                                       [](menus::Node::Ptr menu) { return menu->get_id() != MenuId::OPEN; });
#if GTK_CHECK_VERSION(4, 0, 0)
  menu->get_menu()->set_parent(*timer_view_eventbox);
#else
  menu->get_menu()->attach_to_widget(*this);
#endif
  insert_action_group("app", menu->get_action_group());

#if !GTK_CHECK_VERSION(4, 0, 0)
  signal_configure_event().connect(
    [this](GdkEventConfigure *event) {
      locate_window(event);
      return false;
    },
    false);
#endif

  GUIConfig::key_timerbox("main_window").connect(this, [this]() { on_enabled_changed(); });

  enabled = GUIConfig::timerbox_enabled("main_window")();
  if (enabled)
    {
      open_window();
    }

  GUIConfig::main_window_always_on_top().attach(this, [&](bool enabled) { GtkUtil::set_always_on_top(this, enabled); });
}

void
MainWindow::on_enabled_changed()
{
  TRACE_ENTRY();
  bool new_enabled = GUIConfig::timerbox_enabled("main_window")();
  TRACE_MSG("enabled changed from {} to {}", enabled, new_enabled);

  if (enabled != new_enabled)
    {
      enabled = new_enabled;
      if (enabled)
        {
          open_window();
        }
      else
        {
          close_window();
        }
    }
}

#if GTK_CHECK_VERSION(4, 0, 0)
bool
MainWindow::on_close_request()
{
  TRACE_ENTRY();
  if (can_close)
    {
      close_window();
    }
  closed_signal();
  return true;
}
#else
bool
MainWindow::on_delete_event(GdkEventAny * /*any_event*/)
{
  TRACE_ENTRY();
  if (can_close)
    {
      close_window();
    }
  closed_signal();
  return true;
}
#endif

#if GTK_CHECK_VERSION(4, 0, 0)
void
MainWindow::on_timer_view_button_press(int /* n_press */, double x, double y)
{
  TRACE_ENTRY();

  if (timer_view_click_gesture->get_current_button() == 3)
    {
      bool taking = app->get_core()->is_taking();
      if (!taking || (GUIConfig::block_mode()() != BlockMode::All && GUIConfig::block_mode()() != BlockMode::Input))
        {
          // Workaround for https://gitlab.gnome.org/GNOME/gtk/-/issues/5238
          app->get_menu_model()->update();

          menu->get_menu()->set_pointing_to(Gdk::Rectangle(static_cast<int>(x), static_cast<int>(y), 1, 1));
          menu->get_menu()->popup();
        }
    }
}
#else
bool
MainWindow::on_timer_view_button_press_event(const GdkEventButton *event)
{
  TRACE_ENTRY();

  if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
    {
      bool taking = app->get_core()->is_taking();
      if (!taking || (GUIConfig::block_mode()() != BlockMode::All && GUIConfig::block_mode()() != BlockMode::Input))
        {
          // Workaround for https://gitlab.gnome.org/GNOME/gtk/-/issues/5238
          app->get_menu_model()->update();

          menu->get_menu()->popup_at_pointer((const GdkEvent *)event);
          return true;
        }
    }

  return false;
}
#endif

#if !GTK_CHECK_VERSION(4, 0, 0)
void
MainWindow::move_to_start_position()
{
  TRACE_ENTRY();

  int x = GUIConfig::main_window_x()();
  int y = GUIConfig::main_window_y()();
  int head = GUIConfig::main_window_head()();

  int width = 0;
  int height = 0;
  get_size(width, height);

  if (head >= app->get_toolkit()->get_head_count())
    {
      head = 0;
    }
  convert_monitor_to_display(x, y, head);

  TRACE_MSG("moving to {} {}", x, y);
  move(x, y);
}

void
MainWindow::locate_window(GdkEventConfigure *event)
{
  TRACE_ENTRY();
  (void)event;

  Glib::RefPtr<Gdk::Window> window = get_window();
  if ((window->get_state() & (Gdk::WINDOW_STATE_ICONIFIED | Gdk::WINDOW_STATE_WITHDRAWN)) != 0)
    {
      return;
    }

  int x = 0;
  int y = 0;
  get_position(x, y);

  int width = 0;
  int height = 0;
  get_size(width, height);

  TRACE_MSG("main window = ({} {}) x ({} {})", x, y, width, height);

  if (x <= 0 && y <= 0)
    {
      return;
    }

  int head = convert_display_to_monitor(x, y);
  TRACE_MSG("main window head = ({}, {}) {}", x, y, head);

  GUIConfig::main_window_x().set(x);
  GUIConfig::main_window_y().set(y);
  GUIConfig::main_window_head().set(head);
}
#endif

int
MainWindow::convert_display_to_monitor(int &x, int &y)
{
  auto toolkit_priv = std::dynamic_pointer_cast<IToolkitPrivate>(app->get_toolkit());

  for (int i = 0; i < app->get_toolkit()->get_head_count(); i++)
    {
      auto optional_head = toolkit_priv->get_head_info(i);
      if (!optional_head)
        {
          continue;
        }
      HeadInfo head = *optional_head;

      int left = head.get_x();
      int top = head.get_y();
      int width = head.get_width();
      int height = head.get_height();

      if (x >= left && y >= top && x < left + width && y < top + height)
        {
          x -= left;
          y -= top;

          if (x >= width / 2)
            {
              x -= width;
            }
          if (y >= height / 2)
            {
              y -= height;
            }
          return i;
        }
    }

  x = y = 100;
  return 0;
}

void
MainWindow::convert_monitor_to_display(int &x, int &y, int head)
{
  auto toolkit_priv = std::dynamic_pointer_cast<IToolkitPrivate>(app->get_toolkit());
  auto optional_head = toolkit_priv->get_head_info(head);
  if (!optional_head)
    {
      return;
    }
  HeadInfo h = *optional_head;

  if (x < 0)
    {
      x += h.get_width();
    }

  if (y < 0)
    {
      y += h.get_height();
    }

  x = std::clamp(x, 0, h.get_width());
  y = std::clamp(y, 0, h.get_height());

  x += h.get_x();
  y += h.get_y();
}

MainWindow::closed_signal_t &
MainWindow::signal_closed()
{
  return closed_signal;
}
