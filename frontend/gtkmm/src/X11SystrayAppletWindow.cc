// X11SystrayAppletWindow.cc --- Applet info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011 Rob Caelers & Raymond Penners
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
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <gtkmm/alignment.h>

#include "X11SystrayAppletWindow.hh"

#include "TimerBoxGtkView.hh"
#include "TimerBoxControl.hh"
#include "AppletControl.hh"
#include "Menus.hh"
#include "System.hh"

#include "gtktrayicon.h"

//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
X11SystrayAppletWindow::X11SystrayAppletWindow(AppletControl *control) :
  view(NULL),
  plug(NULL),
  container(NULL),
  applet_orientation(ORIENTATION_UP),
  applet_size(0),
  applet_active(false),
  embedded(false),
  control(control),
  tray_icon(NULL)
{
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
X11SystrayAppletWindow::static_notify_callback(GObject    *gobject,
                                               GParamSpec *arg,
                                               gpointer    user_data)
{
  (void) gobject;
  (void) arg;
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
        }
      else
        {
          orientation = ORIENTATION_LEFT;
        }

      if (applet_orientation != orientation)
        {
          applet_orientation = orientation;
          view->set_geometry(applet_orientation, applet_size);
          TRACE_MSG("orientation " << applet_orientation);
        }
    }
  TRACE_EXIT();
}

//! Initializes the applet.
AppletWindow::AppletState
X11SystrayAppletWindow::activate_applet()
{
  TRACE_ENTER("X11SystrayAppletWindow::activate_applet");

  if (applet_active)
    {
      TRACE_EXIT();
      return APPLET_STATE_VISIBLE;
    }

  tray_icon = wrgtk_tray_icon_new("Workrave Tray Icon");
  AppletState ret =  APPLET_STATE_DISABLED;

  if (tray_icon != NULL)
    {
      g_signal_connect(tray_icon, "notify",
                       G_CALLBACK (static_notify_callback),
                       this);

      plug = Glib::wrap(GTK_PLUG(tray_icon));

      Gtk::EventBox *eventbox = new Gtk::EventBox;
      eventbox->set_visible_window(false);
      eventbox->set_events(eventbox->get_events() | Gdk::BUTTON_PRESS_MASK);
      eventbox->signal_button_press_event().connect(sigc::mem_fun(*this,
                                                                  &X11SystrayAppletWindow::on_button_press_event));
      container = eventbox;

      view = new TimerBoxGtkView(Menus::MENU_APPLET);
      timer_box_view = view;
      timer_box_control = new TimerBoxControl("applet", *timer_box_view);

      Gtk::VBox *box = manage(new Gtk::VBox());
      box->set_spacing(1);
      box->pack_start(*view, true, true, 0);
      
      if (System::is_kde())
        {
          timer_box_control->set_force_empty(true);
        }

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
  return ret;
}


//! Destroys the applet.
void
X11SystrayAppletWindow::deactivate_applet()
{
  TRACE_ENTER("X11SystrayAppletWindow::destroy_applet");

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

      control->set_applet_state(AppletControl::APPLET_TRAY,
                                AppletWindow::APPLET_STATE_DISABLED);
    }

  applet_active = false;

  TRACE_EXIT();
}


//! Applet window is deleted. Destroy applet.
bool
X11SystrayAppletWindow::on_delete_event(GdkEventAny *event)
{
  (void) event;
  deactivate_applet();
  control->set_applet_state(AppletControl::APPLET_TRAY,
                            AppletWindow::APPLET_STATE_DISABLED);
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

      if (o == GTK_ORIENTATION_VERTICAL)
        {
          orientation = ORIENTATION_UP;
        }
      else
        {
          orientation = ORIENTATION_LEFT;
        }

      embedded = true;
      applet_size = 24;
      
      view->set_geometry(applet_orientation, applet_size);
    }

  control->set_applet_state(AppletControl::APPLET_TRAY,
                            AppletWindow::APPLET_STATE_VISIBLE);

  TRACE_EXIT();
}

//! User pressed some mouse button in the main window.
bool
X11SystrayAppletWindow::on_button_press_event(GdkEventButton *event)
{
  bool ret = false;

  if (applet_active &&
      event->type == GDK_BUTTON_PRESS &&
      embedded)
    {
      if (event->button == 3)
        {
          Menus::get_instance()->popup(Menus::MENU_APPLET,
                                       0 /*event->button */, event->time);
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
  (void) button;
  timer_box_control->force_cycle();
}


void
X11SystrayAppletWindow::on_size_allocate(Gtk::Allocation& allocation)
{
  TRACE_ENTER("X11SystrayAppletWindow::on_size_allocate");

  if (embedded)
    {
      TRACE_MSG(allocation.get_x() << " " <<
                allocation.get_y() << " " <<
                allocation.get_width() << " " <<
                allocation.get_height());
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
  
      if (orientation == ORIENTATION_UP ||
          orientation == ORIENTATION_DOWN)
        {
          if (applet_size != allocation.get_width())
            {
              applet_size = allocation.get_width();
              TRACE_MSG("New size = " << allocation.get_width() << " " << allocation.get_height() << " " << applet_orientation);
              view->set_geometry(applet_orientation, applet_size);
            }
        }
      else
        {
          if (applet_size != allocation.get_height())
            {
              applet_size = allocation.get_height();
              TRACE_MSG("New size = " << allocation.get_width() << " " << allocation.get_height() << " " << applet_orientation);
              view->set_geometry(applet_orientation, applet_size);
            }
        }
    }
  TRACE_EXIT();
}
