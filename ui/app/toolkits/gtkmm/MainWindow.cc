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

#ifdef PLATFORM_OS_WINDOWS
#  include <gdk/gdkwin32.h>
#  include <shellapi.h>
#endif

#include "commonui/nls.h"
#include "debug.hh"

#include "MainWindow.hh"

#include <list>
#include <gtkmm.h>

#include "config/IConfigurator.hh"

#include "TimerBoxGtkView.hh"
#include "ui/TimerBoxControl.hh"
#include "ui/GUIConfig.hh"
#include "GtkUtil.hh"

#include "IToolkitPrivate.hh"
#include "ToolkitMenu.hh"
#include "commonui/MenuDefs.hh"

#ifdef PLATFORM_OS_WINDOWS_LEGACY
const char *WIN32_MAIN_CLASS_NAME = "Workrave";
#endif

using namespace std;

MainWindow::MainWindow(std::shared_ptr<IApplication> app)
  : app(app)
  , toolkit(app->get_toolkit())
{
  menu = std::make_shared<ToolkitMenu>(app->get_menu_model(), [](menus::Node::Ptr menu) { return menu->get_id() != MenuId::OPEN; });
}

MainWindow::~MainWindow()
{
  TRACE_ENTRY();
#ifdef PLATFORM_OS_WINDOWS_LEGACY
  if (timeout_connection.connected())
    {
      timeout_connection.disconnect();
    }
#endif

  delete timer_box_control;
#ifdef PLATFORM_OS_WINDOWS_LEGACY
  win32_exit();
#endif
#ifdef PLATFORM_OS_UNIX
  delete leader;
#endif
}

// bool
// MainWindow::is_visible() const
// {
// #if defined(PLATFORM_OS_WINDOWS_LEGACY)
//   const GtkWidget *window = Gtk::Widget::gobj();
//   GdkWindow *gdk_window = gtk_widget_get_window(GTK_WIDGET(window));
//   HWND hwnd = (HWND)GDK_WINDOW_HWND(gdk_window);
//   return IsWindowVisible(hwnd);
// #else
//   return get_visible();
// #endif
// }

void
MainWindow::toggle_window()
{
  TRACE_ENTRY();
  bool visible = is_visible();
  if (visible)
    {
      close_window();
    }
  else
    {
      open_window();
    }
}

//! Opens the main window.
void
MainWindow::open_window()
{
  TRACE_ENTRY();
  if (timer_box_view->get_visible_count() > 0)
    {
#ifdef PLATFORM_OS_WINDOWS_LEGACY
      win32_show(true);
      show_all();
#else
      show_all();
      deiconify();
#endif

      int x, y, head;
      set_position(Gtk::WIN_POS_NONE);
      set_gravity(Gdk::GRAVITY_STATIC);
      get_start_position(x, y, head);

      GtkRequisition min_size;
      GtkRequisition natural_size;
      get_preferred_size(min_size, natural_size);

      bound_head(x, y, min_size.width, min_size.height, head);

      map_from_head(x, y, head);
      TRACE_MSG("moving to {} {}", x, y);
      move(x, y);

      bool always_on_top = GUIConfig::main_window_always_on_top()();
      GtkUtil::set_always_on_top(this, always_on_top);
      GUIConfig::timerbox_enabled("main_window").set(true);
    }
}

//! Closes the main window.
void
MainWindow::close_window()
{
  TRACE_ENTRY();
#if defined(PLATFORM_OS_WINDOWS_LEGACY)
  win32_show(false);
#elif defined(PLATFORM_OS_WINDOWS)
  hide();
#elif defined(PLATFORM_OS_MACOS)
  hide();
#else
  if (can_close)
    {
      TRACE_MSG("hide");
      hide();
    }
  else
    {
      TRACE_MSG("iconify");
      iconify();
    }
#endif

  GUIConfig::timerbox_enabled("main_window").set(false);
}

void
MainWindow::set_can_close(bool can_close)
{
  TRACE_ENTRY_PAR(can_close);
  this->can_close = can_close;

  TRACE_VAR(enabled);
  TRACE_VAR(is_visible());
  if (!is_visible())
    {
      if (can_close)
        {
          TRACE_MSG("hide");
          hide();
        }
      else
        {
          TRACE_MSG("iconify");
          iconify();
          show_all();
        }
    }
}

//! Updates the main window.
void
MainWindow::update()
{
  timer_box_control->update();
}

//! Initializes the main window.
void
MainWindow::init()
{
  TRACE_ENTRY();
  set_border_width(2);
  set_resizable(false);

  std::list<Glib::RefPtr<Gdk::Pixbuf>> icons;

  const char *icon_files[] = {
#ifndef PLATFORM_OS_WINDOWS
    // This causes a crash on windows
    "scalable" G_DIR_SEPARATOR_S "workrave.svg",
#endif
    "16x16" G_DIR_SEPARATOR_S "workrave.png",
    "24x24" G_DIR_SEPARATOR_S "workrave.png",
    "32x32" G_DIR_SEPARATOR_S "workrave.png",
    "48x48" G_DIR_SEPARATOR_S "workrave.png",
    "64x64" G_DIR_SEPARATOR_S "workrave.png",
    "96x96" G_DIR_SEPARATOR_S "workrave.png",
    "128x128" G_DIR_SEPARATOR_S "workrave.png",
  };

  for (auto &icon_file: icon_files)
    {
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = GtkUtil::create_pixbuf(icon_file);
      if (pixbuf)
        {
          icons.push_back(pixbuf);
        }
    }

  Glib::ListHandle<Glib::RefPtr<Gdk::Pixbuf>> icon_list(icons);
  Gtk::Window::set_default_icon_list(icon_list);
  // Gtk::Window::set_default_icon_name("workrave");

  timer_box_view = Gtk::manage(new TimerBoxGtkView(app));
  timer_box_control = new TimerBoxControl(app, "main_window", timer_box_view);
  timer_box_view->set_geometry(ORIENTATION_LEFT, -1);
  timer_box_control->update();

  Gtk::EventBox *eventbox = new Gtk::EventBox;
  eventbox->set_visible_window(false);
  eventbox->set_events(eventbox->get_events() | Gdk::BUTTON_PRESS_MASK);

  //#ifndef PLATFORM_OS_MACOS
  // No popup menu on OS X
  eventbox->signal_button_press_event().connect(sigc::mem_fun(*this, &MainWindow::on_timer_view_button_press_event));
  //#endif

  eventbox->add(*timer_box_view);
  add(*eventbox);

  // Necessary for popup menu
  realize_if_needed();

  Glib::RefPtr<Gdk::Window> window = get_window();

  // Window decorators
  window->set_decorations(Gdk::DECOR_BORDER | Gdk::DECOR_TITLE | Gdk::DECOR_MENU);
  // This used to be W32 only:
  //   window->set_functions(Gdk::FUNC_CLOSE|Gdk::FUNC_MOVE);

  // (end window decorators)

#ifdef PLATFORM_OS_UNIX
  // HACK. this sets a different group leader in the WM_HINTS....
  // Without this hack, metacity makes ALL windows on-top.
  leader = new Gtk::Window(Gtk::WINDOW_POPUP);
  gtk_widget_realize(GTK_WIDGET(leader->gobj()));
  Glib::RefPtr<Gdk::Window> leader_window = leader->get_window();
  window->set_group(leader_window);
#endif

  stick();
  setup();

#ifdef PLATFORM_OS_WINDOWS_LEGACY

  win32_init();
  set_gravity(Gdk::GRAVITY_STATIC);
  set_position(Gtk::WIN_POS_NONE);

#  ifdef HAVE_NOT_PROPER_SIZED_MAIN_WINDOW_ON_STARTUP
  // This is the proper code, see hacked code below.
  if (!enabled)
    {
      move(-1024, 0);
      show_all();
      win32_show(false);
      move_to_start_position();
    }
  else
    {
      move_to_start_position();
      show_all();
    }
#  else // Hack deprecated: Since GTK+ 2.10 no longer necessary

  // Hack: since GTK+ 2.2.4 the window is too wide, it incorporates room
  // for the "_ [ ] [X]" buttons somehow. This hack fixes just that.
  move(-1024, 0); // Move off-screen to reduce wide->narrow flickering
  show_all();
  HWND hwnd = (HWND)GDK_WINDOW_HWND(window->gobj());
  SetWindowPos(hwnd, NULL, 0, 0, 1, 1, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE);
  if (!enabled)
    {
      win32_show(false);
    }
  move_to_start_position();
  // (end of hack)
#  endif

#else
  set_gravity(Gdk::GRAVITY_STATIC);
  set_position(Gtk::WIN_POS_NONE);
  show_all();
  move_to_start_position();

  if (!enabled && can_close) //  || get_start_in_tray())
    {
#  ifdef PLATFORM_OS_UNIX
      iconify();
#  endif
      close_window();
    }
#endif
  setup();
  set_title("Workrave");

  GUIConfig::key_timerbox("main_window").connect(this, [this]() { setup(); });

  menu->get_menu()->attach_to_widget(*this);
  insert_action_group("app", menu->get_action_group());
}

//! Setup configuration settings.
void
MainWindow::setup()
{
  TRACE_ENTRY();
  bool new_enabled = GUIConfig::timerbox_enabled("main_window")();
  bool always_on_top = GUIConfig::main_window_always_on_top()();

  TRACE_MSG("can_close {}", new_enabled);
  TRACE_MSG("enabled {}", new_enabled);
  TRACE_MSG("on top {}", always_on_top);

  if (enabled != new_enabled)
    {
      enabled = new_enabled;
      if (enabled)
        {
          TRACE_MSG("open");
          open_window();
        }
      else
        {
          TRACE_MSG("close");
          close_window();
        }
    }

  bool visible = is_visible();

  if (visible)
    {
      GtkUtil::set_always_on_top(this, always_on_top);
    }

  if (visible && always_on_top)
    {
      raise();
    }
}

//! User has closed the main window.
bool
MainWindow::on_delete_event(GdkEventAny *)
{
  TRACE_ENTRY();
#if defined(PLATFORM_OS_WINDOWS_LEGACY)
  win32_show(false);
  closed_signal.emit();
  GUIConfig::timerbox_enabled("main_window").set(false);
#else
  if (can_close)
    {
      close_window();
      GUIConfig::timerbox_enabled("main_window").set(false);
    }
  else
    {
      app->get_toolkit()->terminate();
    }
#endif

  return true;
}

bool
MainWindow::on_timer_view_button_press_event(const GdkEventButton *event)
{
  TRACE_ENTRY();
  bool ret = false;

  (void)event;

  if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
    {
      menu->get_menu()->popup_at_pointer((const GdkEvent *)event);
      ret = true;
    }

  return ret;
}

#ifdef PLATFORM_OS_WINDOWS_LEGACY
void
MainWindow::win32_show(bool b)
{
  TRACE_ENTRY_PAR(b);
  bool retry = false;

  // Gtk's hide() seems to quit the program.
  GtkWidget *window = Gtk::Widget::gobj();
  GdkWindow *gdk_window = gtk_widget_get_window(window);
  HWND hwnd = (HWND)GDK_WINDOW_HWND(gdk_window);
  ShowWindow(hwnd, b ? SW_SHOWNORMAL : SW_HIDE);

  if (b)
    {
      present();

      if (hwnd != GetForegroundWindow())
        {
          if (show_retry_count == 0)
            {
              show_retry_count = 20;
            }
          else
            {
              show_retry_count--;
            }

          TRACE_MSG("2 {}", show_retry_count);
          retry = true;
        }
    }

  if (retry)
    {
      if (show_retry_count > 0)
        {
          timeout_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &MainWindow::win32_show_retry), 50);
        }
    }
  else
    {
      show_retry_count = 0;
    }
}

bool
MainWindow::win32_show_retry()
{
  TRACE_ENTRY();
  if (show_retry_count > 0)
    {
      TRACE_MSG("retry");
      win32_show(true);
    }
  return false;
}

void
MainWindow::win32_init()
{
  TRACE_ENTRY();
  win32_hinstance = (HINSTANCE)GetModuleHandle(NULL);

  WNDCLASSEXA wclass =
    {sizeof(WNDCLASSEXA), 0, win32_window_proc, 0, 0, win32_hinstance, NULL, NULL, NULL, NULL, WIN32_MAIN_CLASS_NAME, NULL};
  /* ATOM atom = */ RegisterClassExA(&wclass);

  win32_main_hwnd = CreateWindowExA(WS_EX_TOOLWINDOW,
                                    WIN32_MAIN_CLASS_NAME,
                                    "Workrave",
                                    WS_OVERLAPPED,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    (HWND)NULL,
                                    (HMENU)NULL,
                                    win32_hinstance,
                                    (LPSTR)NULL);
  ShowWindow(win32_main_hwnd, SW_HIDE);

  // User data
  SetWindowLongPtr(win32_main_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

  // Reassign ownership
  GtkWidget *window = Gtk::Widget::gobj();
  GdkWindow *gdk_window = gtk_widget_get_window(window);
  HWND hwnd = (HWND)GDK_WINDOW_HWND(gdk_window);
  SetWindowLongPtr(hwnd, GWLP_HWNDPARENT, (LONG_PTR)win32_main_hwnd);
}

void
MainWindow::win32_exit()
{
  DestroyWindow(win32_main_hwnd);
  UnregisterClassA(WIN32_MAIN_CLASS_NAME, GetModuleHandle(NULL));
}

#endif

void
MainWindow::get_start_position(int &x, int &y, int &head)
{
  TRACE_ENTRY();
  x = GUIConfig::main_window_x()();
  y = GUIConfig::main_window_y()();
  head = GUIConfig::main_window_head()();
  if (head < 0)
    {
      head = 0;
    }
  TRACE_VAR(x, y, head);
}

void
MainWindow::set_start_position(int x, int y, int head)
{
  TRACE_ENTRY_PAR(x, y, head);
  GUIConfig::main_window_x().set(x);
  GUIConfig::main_window_y().set(y);
  GUIConfig::main_window_head().set(head);
}

void
MainWindow::move_to_start_position()
{
  TRACE_ENTRY();
  int x, y, head;
  get_start_position(x, y, head);

  GtkRequisition min_size;
  GtkRequisition natural_size;
  get_preferred_size(min_size, natural_size);

  bound_head(x, y, min_size.width, min_size.height, head);

  window_head_location.set_x(x);
  window_head_location.set_y(y);

  map_from_head(x, y, head);

  TRACE_MSG("Main window size {} {}", min_size.width, min_size.height);

  window_location.set_x(x);
  window_location.set_y(y);
  window_relocated_location.set_x(x);
  window_relocated_location.set_y(y);
  TRACE_MSG("moving to {} {}", x, y);

  move(x, y);
}

bool
MainWindow::on_configure_event(GdkEventConfigure *event)
{
  TRACE_ENTRY_PAR(event->x, event->y);
  locate_window(event);
  bool ret = Widget::on_configure_event(event);
  return ret;
}

void
MainWindow::locate_window(GdkEventConfigure *event)
{
  TRACE_ENTRY();
  int x, y;
  int width, height;

  (void)event;

  Glib::RefPtr<Gdk::Window> window = get_window();
  if ((window->get_state() & (Gdk::WINDOW_STATE_ICONIFIED | Gdk::WINDOW_STATE_WITHDRAWN)) != 0)
    {
      return;
    }

#ifndef PLATFORM_OS_WINDOWS
  // Returns bogus results on windows...sometime.
  if (event != nullptr)
    {
      x = event->x;
      y = event->y;
      width = event->width;
      height = event->height;
    }
  else
#endif
    {
      (void)event;

      get_position(x, y);

      GtkRequisition min_size;
      GtkRequisition natural_size;
      get_preferred_size(min_size, natural_size);

      width = min_size.width;
      height = min_size.height;
    }

  TRACE_MSG("main window = {} {}", x, y);

  if (x <= 0 && y <= 0)
    {
      return;
    }

  if (x != window_relocated_location.get_x() || y != window_relocated_location.get_y())
    {
      window_location.set_x(x);
      window_location.set_y(y);
      window_relocated_location.set_x(x);
      window_relocated_location.set_y(y);

      int head = map_to_head(x, y);
      TRACE_MSG("main window head = {} {} {}", x, y, head);

      bool rc = bound_head(x, y, width, height, head);
      TRACE_MSG("main window bounded = {} {}", x, y);

      window_head_location.set_x(x);
      window_head_location.set_y(y);
      set_start_position(x, y, head);

      if (rc)
        {
          move_to_start_position();
        }
    }
}

void
MainWindow::relocate_window(int width, int height)
{
  TRACE_ENTRY_PAR(width, height);
  int x = window_location.get_x();
  int y = window_location.get_y();

  if (x <= 0 || y <= 0)
    {
      TRACE_MSG("invalid {} {}", x, y);
    }
  else if (x <= width && y <= height)
    {
      TRACE_VAR(x, y);
      TRACE_MSG("fits, moving to");
      set_position(Gtk::WIN_POS_NONE);
      move(x, y);
    }
  else
    {
      TRACE_MSG("move to differt head");
      x = window_head_location.get_x();
      y = window_head_location.get_y();

      int num_heads = toolkit->get_head_count();
      for (int i = 0; i < num_heads; i++)
        {
          GtkRequisition min_size;
          GtkRequisition natural_size;
          get_preferred_size(min_size, natural_size);

          bound_head(x, y, min_size.width, min_size.height, i);

          map_from_head(x, y, i);
          break;
        }

      if (x < 0)
        {
          x = 0;
        }
      if (y < 0)
        {
          y = 0;
        }

      TRACE_MSG("moving to {} {}", x, y);
      window_relocated_location.set_x(x);
      window_relocated_location.set_y(y);

      set_position(Gtk::WIN_POS_NONE);
      move(x, y);
    }
}

#ifdef PLATFORM_OS_WINDOWS_LEGACY

LRESULT CALLBACK
MainWindow::win32_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  TRACE_ENTRY_PAR(uMsg, wParam);
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif

sigc::signal<void> &
MainWindow::signal_closed()
{
  return closed_signal;
}

int
MainWindow::map_to_head(int &x, int &y)
{
  int ret = -1;

  auto toolkit_priv = std::dynamic_pointer_cast<IToolkitPrivate>(toolkit);

  for (int i = 0; i < toolkit->get_head_count(); i++)
    {
      int left, top, width, height;

      HeadInfo head = toolkit_priv->get_head_info(i);

      left = head.get_x();
      top = head.get_y();
      width = head.get_width();
      height = head.get_height();

      if (x >= left && y >= top && x < left + width && y < top + height)
        {
          x -= left;
          y -= top;

          // Use coordinates relative to right and butto edges of the
          // screen if the mainwindow is closer to those edges than to
          // the left/top edges.

          if (x >= width / 2)
            {
              x -= width;
            }
          if (y >= height / 2)
            {
              y -= height;
            }
          ret = i;
          break;
        }
    }

  if (ret < 0)
    {
      ret = 0;
      x = y = 256;
    }
  return ret;
}

void
MainWindow::map_from_head(int &x, int &y, int head)
{
  auto toolkit_priv = std::dynamic_pointer_cast<IToolkitPrivate>(toolkit);
  HeadInfo h = toolkit_priv->get_head_info(head);
  if (x < 0)
    {
      x += h.get_width();
    }
  if (y < 0)
    {
      y += h.get_height();
    }

  x += h.get_x();
  y += h.get_y();
}

bool
MainWindow::bound_head(int &x, int &y, int width, int height, int &head)
{
  bool ret = false;
  auto toolkit_priv = std::dynamic_pointer_cast<IToolkitPrivate>(toolkit);

  if (head >= toolkit->get_head_count())
    {
      head = 0;
    }

  HeadInfo h = toolkit_priv->get_head_info(head);
  if (x < -h.get_width())
    {
      x = 0;
      ret = true;
    }
  if (y < -h.get_height())
    {
      y = 0;
      ret = true;
    }

  // Make sure something remains visible..
  if (x > -10 && x < 0)
    {
      x = -10;
      ret = true;
    }
  if (y > -10 && y < 0)
    {
      y = -10;
      ret = true;
    }

  if (x + width >= h.get_width())
    {
      x = h.get_width() - width - 10;
      ret = true;
    }

  if (y + height >= h.get_height())
    {
      y = h.get_height() - height - 10;
      ret = true;
    }

  return ret;
}
