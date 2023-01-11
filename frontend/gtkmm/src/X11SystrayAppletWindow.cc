// X11SystrayAppletWindow.cc --- Applet info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011, 2012, 2013 Rob Caelers & Raymond Penners
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

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include "X11SystrayAppletWindow.hh"

#include "CoreFactory.hh"
#include "IConfigurator.hh"
#include "GUI.hh"
#include "TimerBoxGtkView.hh"
#include "TimerBoxControl.hh"
#include "Menus.hh"

#include "gtktrayicon.h"

//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
X11SystrayAppletWindow::X11SystrayAppletWindow()
{
  IConfigurator *config = CoreFactory::get_configurator();
  config->add_listener(GUIConfig::CFG_KEY_APPLET_FALLBACK_ENABLED, this);
  read_configuration();
}

//! Destructor.
X11SystrayAppletWindow::~X11SystrayAppletWindow()
{
  delete plug;
  delete container;
  delete timer_box_control;
  delete timer_box_view;
}

void
X11SystrayAppletWindow::static_notify_callback(GObject *gobject, GParamSpec *arg, gpointer user_data)
{
  (void)gobject;
  (void)arg;
  X11SystrayAppletWindow *applet = (X11SystrayAppletWindow *)user_data;
  applet->notify_callback();
}

void
X11SystrayAppletWindow::notify_callback()
{
  TRACE_ENTER("X11SystrayAppletWindow::notify_callback");
  if (tray_icon != NULL && embedded)
    {
      GtkOrientation o = wrgtk_tray_icon_get_orientation(tray_icon);
      Orientation orientation;

      if (o != GTK_ORIENTATION_VERTICAL)
        {
          orientation = ORIENTATION_UP;
          TRACE_MSG("up");
        }
      else
        {
          orientation = ORIENTATION_LEFT;
          TRACE_MSG("left");
        }

      if (applet_orientation != orientation)
        {
          TRACE_MSG("orientation " << orientation);
          applet_orientation = orientation;
          view->set_geometry(applet_orientation, applet_size);
        }
    }
  TRACE_EXIT();
}

//! Initializes the applet.
void
X11SystrayAppletWindow::activate()
{
  TRACE_ENTER("X11SystrayAppletWindow::activate");

#if defined(GDK_WINDOWING_X11) && defined(HAVE_GTK3)
  GdkDisplay *display = gdk_display_manager_get_default_display(gdk_display_manager_get());
  if (!GDK_IS_X11_DISPLAY(display))
    {
      TRACE_EXIT();
      return;
    }
#endif

  if (applet_active)
    {
      TRACE_RETURN("already active, embedded: " << embedded);
      return;
    }

  tray_icon = wrgtk_tray_icon_new("Workrave Tray Icon");

  if (tray_icon != NULL)
    {
      g_signal_connect(tray_icon, "notify", G_CALLBACK(static_notify_callback), this);

      plug = Glib::wrap(GTK_PLUG(tray_icon));

      Gtk::EventBox *eventbox = new Gtk::EventBox;
      eventbox->set_visible_window(false);
      eventbox->set_events(eventbox->get_events() | Gdk::BUTTON_PRESS_MASK);
      eventbox->signal_button_press_event().connect(sigc::mem_fun(*this, &X11SystrayAppletWindow::on_button_press_event));
      container = eventbox;

      view = new TimerBoxGtkView(Menus::MENU_MAINAPPLET);
      timer_box_view = view;
      timer_box_control = new TimerBoxControl("applet", *timer_box_view);
      timer_box_control->update();

      Gtk::VBox *box = manage(new Gtk::VBox());
      box->set_spacing(1);
      box->pack_start(*view, true, true, 0);

      container->add(*box);

      plug->signal_embedded().connect(sigc::mem_fun(*this, &X11SystrayAppletWindow::on_embedded));
      plug->signal_delete_event().connect(sigc::mem_fun(*this, &X11SystrayAppletWindow::on_delete_event));
      plug->signal_size_allocate().connect(sigc::mem_fun(*this, &X11SystrayAppletWindow::on_size_allocate));

      plug->add(*container);
      plug->show_all();

      applet_orientation = ORIENTATION_UP;

#ifdef HAVE_GTK3
      GtkRequisition min_size;
      GtkRequisition natural_size;
      plug->get_preferred_size(min_size, natural_size);
      applet_size = min_size.height;
#else
      Gtk::Requisition req;
      plug->size_request(req);
      applet_size = req.height;
#endif

      view->set_geometry(applet_orientation, applet_size);

      applet_active = true;
    }

  TRACE_EXIT();
  return;
}

//! Destroys the applet.
void
X11SystrayAppletWindow::deactivate()
{
  TRACE_ENTER("X11SystrayAppletWindow::deactivate");

  if (applet_active)
    {
      if (plug != NULL)
        {
          plug->remove();
          delete plug;
          plug = NULL;
        }
      if (container != NULL)
        {
          container->remove();
          delete container;
          container = NULL;
        }

      delete timer_box_control;
      timer_box_control = NULL;

      delete timer_box_view;
      timer_box_view = NULL;
      view = NULL;

      visibility_changed_signal.emit(false);
    }

  applet_active = false;

  TRACE_EXIT();
}

//! Applet window is deleted. Destroy applet.
bool
X11SystrayAppletWindow::on_delete_event(GdkEventAny *event)
{
  (void)event;
  deactivate();
  visibility_changed_signal.emit(false);
  return true;
}

//! Notification of the system tray that the applet has been embedded.
void
X11SystrayAppletWindow::on_embedded()
{
  TRACE_ENTER("X11SystrayAppletWindow::on_embedded");

  if (applet_active)
    {
      GtkOrientation o = wrgtk_tray_icon_get_orientation(tray_icon);
      Orientation orientation;

      if (o != GTK_ORIENTATION_VERTICAL)
        {
          orientation = ORIENTATION_UP;
        }
      else
        {
          orientation = ORIENTATION_LEFT;
        }

      embedded = true;
      applet_size = 24;
      applet_orientation = orientation;

      view->set_geometry(applet_orientation, applet_size);

#ifdef HAVE_GTK3
      gint icon_size = wrgtk_tray_icon_get_icon_size(tray_icon);
      TRACE_MSG("icon size " << icon_size);
#endif
    }

  visibility_changed_signal.emit(true);
  TRACE_EXIT();
}

//! User pressed some mouse button in the main window.
bool
X11SystrayAppletWindow::on_button_press_event(GdkEventButton *event)
{
  bool ret = false;

  if (applet_active && event->type == GDK_BUTTON_PRESS && embedded)
    {
      if (event->button == 3)
        {
          IGUI *gui = GUI::get_instance();
          Menus *menus = gui->get_menus();
          menus->popup(Menus::MENU_MAINAPPLET, 0 /*event->button */, event->time);
          ret = true;
        }
      if (event->button == 1)
        {
          button_clicked(1);
          ret = true;
        }
    }

  return ret;
}

//! User clicked left mouse button.
void
X11SystrayAppletWindow::button_clicked(int button)
{
  (void)button;
  timer_box_control->force_cycle();
}

void
X11SystrayAppletWindow::on_size_allocate(Gtk::Allocation &allocation)
{
  TRACE_ENTER("X11SystrayAppletWindow::on_size_allocate");

  if (embedded)
    {
      TRACE_MSG(allocation.get_x() << " " << allocation.get_y() << " " << allocation.get_width() << " " << allocation.get_height());
#ifdef HAVE_GTK3
      gint icon_size = wrgtk_tray_icon_get_icon_size(tray_icon);
      TRACE_MSG("icon size " << icon_size);
#endif
      GtkOrientation o = wrgtk_tray_icon_get_orientation(tray_icon);
      Orientation orientation;

      if (o == GTK_ORIENTATION_VERTICAL)
        {
          orientation = ORIENTATION_UP;
        }
      else
        {
          orientation = ORIENTATION_LEFT;
        }

      if (orientation == ORIENTATION_UP || orientation == ORIENTATION_DOWN)
        {
          if (applet_size != allocation.get_width())
            {
              applet_size = allocation.get_width();
              TRACE_MSG("New size = " << applet_size);
              view->set_geometry(applet_orientation, applet_size);
            }
        }
      else
        {
          if (applet_size != allocation.get_height())
            {
              applet_size = allocation.get_height();
              TRACE_MSG("New size = " << applet_size);
              view->set_geometry(applet_orientation, applet_size);
            }
        }

#ifdef HAVE_GTK3
      Gtk::Requisition my_size;
      GtkRequisition natural_size;
      view->get_preferred_size(my_size, natural_size);
      TRACE_MSG("my_size = " << my_size.width << " " << my_size.height);
      TRACE_MSG("natural_size = " << natural_size.width << " " << natural_size.height);

      // hack...
      if (!view->is_sheep_only())
        {
          view->set_sheep_only(allocation.get_width() < my_size.width || allocation.get_height() < my_size.height);
        }
#endif
    }
  TRACE_EXIT();
}

void
X11SystrayAppletWindow::read_configuration()
{
  bool previous_enabled = enabled;
  enabled = GUIConfig::is_applet_fallback_enabled();

  if (!previous_enabled && enabled)
    {
      activate();
    }
  else if (previous_enabled && !enabled)
    {
      deactivate();
    }
}

void
X11SystrayAppletWindow::config_changed_notify(const std::string &key)
{
  TRACE_ENTER_MSG("X11SystrayAppletWindow::config_changed_notify", key);
  (void)key;
  read_configuration();
  TRACE_EXIT();
}

bool
X11SystrayAppletWindow::is_visible() const
{
  return applet_active && embedded;
}
