// KdeAppletWindow.cc --- Applet info Window
//
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 Rob Caelers & Raymond Penners
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

static const char rcsid[] = "$Id$";

#include "preinclude.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "nls.h"
#include "debug.hh"

#include <gtkmm/alignment.h>

#include "KdeAppletWindow.hh"
#include "AppletControl.hh"

#include "TimerBoxGtkView.hh"
#include "TimerBoxControl.hh"
#include "Menus.hh"
#include "System.hh"

#include "ICore.hh"
#include "CoreFactory.hh"

#include <gdk/gdkcolor.h>

#include "KdeWorkraveControl.hh"

#ifdef HAVE_KDE
#include <dcopclient.h>
#include <kapp.h>
#include <kde_applet/kworkraveapplet_stub.h>
#endif


//! Constructor.
/*!
 *  \param control Interface to the controller.
 */
KdeAppletWindow::KdeAppletWindow(AppletControl *control) :
  plug(NULL),
  container(NULL),
  applet_orientation(ORIENTATION_UP),
  applet_size(0),
  control(control),
  applet_active(false)
{
  KdeWorkraveControl::init();
}


//! Destructor.
KdeAppletWindow::~KdeAppletWindow()
{
  delete plug;
  delete container;
  delete timer_box_control;
  delete timer_box_view;
}


//! Initializes the native kde applet.
AppletWindow::AppletState
KdeAppletWindow::activate_applet()
{
  TRACE_ENTER("KdeAppletWindow::activate_applet");

  if (applet_active)
    {
      TRACE_EXIT();
      return AppletWindow::APPLET_STATE_VISIBLE;

    }
  bool ok = true;

  ok = get_orientation(applet_orientation);

  if (ok)
    {
      ok = get_size(applet_size);
    }

  if (ok)
    {
      // Gtk::Alignment *frame = new Gtk::Alignment(1.0, 1.0, 0.0, 0.0);
      // frame->set_border_width(2);

      Gtk::EventBox *eventbox = new Gtk::EventBox;
      eventbox->set_events(eventbox->get_events() | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
      eventbox->signal_button_press_event().connect(sigc::mem_fun(*this, &KdeAppletWindow::on_button_press_event));
      container = eventbox;

      // container = frame;

      plug = new Gtk::Plug((unsigned int)0);
      plug->add(*container);

      plug->signal_embedded().connect(sigc::mem_fun(*this, &KdeAppletWindow::on_embedded));
      plug->signal_delete_event().connect(sigc::mem_fun(*this, &KdeAppletWindow::delete_event));

      // Gtkmm does not wrap this event....
      g_signal_connect(G_OBJECT(plug->gobj()), "destroy-event",
                       G_CALLBACK(KdeAppletWindow::destroy_event), this);

      view = new TimerBoxGtkView();
      timer_box_view = view;

      timer_box_control = new TimerBoxControl("applet", *timer_box_view);
      view->set_geometry(applet_orientation, applet_size);
      view->show_all();

      container->add(*view);
      container->show_all();
      plug->show_all();

      Gtk::Requisition req;
      container->size_request(req);
      TRACE_MSG("Size = " << req.width << " " << req.height << " " << applet_orientation);

      plug_window(plug->get_id());

      // somehow, signal_embedded is never triggered...
      applet_active = true;
    }

  TRACE_EXIT();

  return ok ?
    AppletWindow::APPLET_STATE_VISIBLE :
    AppletWindow::APPLET_STATE_DISABLED;
}


//! Destroys the native gnome applet.
void
KdeAppletWindow::deactivate_applet()
{
  if (applet_active)
    {
      // Cleanup Widgets.
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
    }
  applet_active = false;
}


//! Applet window is deleted. Destroy applet.
bool
KdeAppletWindow::delete_event(GdkEventAny *event)
{
  (void) event;
  deactivate_applet();
  control->set_applet_state(AppletControl::APPLET_KDE,
                            AppletWindow::APPLET_STATE_DISABLED);
  return true;
}


//! Fires up the applet (as requested by the native kde applet).
void
KdeAppletWindow::fire_kde_applet()
{
  control->show(AppletControl::APPLET_KDE);
}


//! Updates the applet window.
void
KdeAppletWindow::update_applet()
{
  TRACE_ENTER("KdeAppletWindow::update");
  if (applet_active)
    {
      timer_box_control->update();

      Gtk::Requisition req;
      container->size_request(req);

      TRACE_MSG("Size = " << req.width << " " << req.height << " " << applet_orientation);
      if (req.width != last_size.width || req.height != last_size.height)
        {
          last_size = req;
          KdeAppletWindow::set_size(last_size.width, last_size.height);
        }
    }

  TRACE_EXIT();
}


//! Destroy notification.
gboolean
KdeAppletWindow::destroy_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  (void) event;
  (void) widget;

  if (user_data != NULL)
    {
      KdeAppletWindow *applet = (KdeAppletWindow *) user_data;
      applet->delete_event(NULL);
    }
  return true;
}


//! User pressed some mouse button in the main window.
bool
KdeAppletWindow::on_button_press_event(GdkEventButton *event)
{
  bool ret = false;

  if (event->type == GDK_BUTTON_PRESS)
    {
      if (event->button == 3)
        {
          Menus::get_instance()->popup(Menus::MENU_APPLET,
                                       event->button, event->time);
          ret = true;
        }
      if (event->button == 1) // FIXME:  && visible_count == 0)
        {
          button_clicked(1);
          ret = true;
        }
    }

  return ret;
}


//! User clicked left mouse button.
void
KdeAppletWindow::button_clicked(int button)
{
  (void) button;
  timer_box_control->force_cycle();
}


//! Notification of the system tray that the applet has been embedded.
void
KdeAppletWindow::on_embedded()
{
  TRACE_ENTER("KdeAppletWindow::on_embedded");
  if (applet_active)
    {
      container->set_size_request(-1,-1);
      container->size_request(last_size);

      TRACE_MSG("Size = " << last_size.width << " " << last_size.height << " " << applet_orientation);
      view->set_geometry(applet_orientation, applet_size);

      TRACE_MSG(applet_size);
      KdeAppletWindow::set_size(last_size.width, last_size.height);
    }

  control->set_applet_state(AppletControl::APPLET_KDE,
                            AppletWindow::APPLET_STATE_VISIBLE);

  TRACE_EXIT();
}

bool
KdeAppletWindow::plug_window(int w)
{
  TRACE_ENTER("KdeAppletWindow::plug_window");
  KWorkraveApplet_stub dcop("kworkrave", "KWorkrave");
  dcop.embed_window(w);
  TRACE_MSG(dcop.ok());
  TRACE_EXIT();

  return dcop.ok();
}


bool
KdeAppletWindow::get_size(int &size)
{
  TRACE_ENTER("KdeAppletWindow::get_size");
  KWorkraveApplet_stub dcop("kworkrave", "KWorkrave");
  size = dcop.get_size();
  TRACE_MSG(dcop.ok() << " " << size);
  TRACE_EXIT();

  return dcop.ok();
}

bool
KdeAppletWindow::get_orientation(Orientation &orientation)
{
  TRACE_ENTER("KdeAppletWindow::get_orientation");
  KWorkraveApplet_stub dcop("kworkrave", "KWorkrave");
  orientation = (Orientation) dcop.get_orientation();
  TRACE_MSG(dcop.ok() << " " << orientation);
  TRACE_EXIT();

  return dcop.ok();
}

bool
KdeAppletWindow::set_size(int width, int height)
{
  TRACE_ENTER_MSG("KdeAppletWindow::set_size", width << " " << height);
  KWorkraveApplet_stub dcop("kworkrave", "KWorkrave");
  dcop.set_size(width, height);
  TRACE_MSG(dcop.ok());
  TRACE_EXIT();

  return dcop.ok();
}

