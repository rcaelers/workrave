// MainWindow.cc --- Main info Window
//
// Copyright (C) 2001, 2002 Rob Caelers & Raymond Penners
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

static const char rcsid[] = "$Id$";

// TODO: only when needed.
#define NOMINMAX

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.hh"

#include <unistd.h>
#include <iostream>

#ifdef HAVE_GNOME
#include <gnome.h>
#else
#include "gnome-about.h"
#endif

#include "MainWindow.hh"
#include "PreferencesDialog.hh"
#include "StatisticsDialog.hh"
#include "WindowHints.hh"
#include "TimeBar.hh"
#include "GUI.hh"
#include "GUIControl.hh"
#include "Util.hh"

#include "Configurator.hh"
#include "TimerInterface.hh"
#include "ControlInterface.hh"
#include "ActivityMonitorInterface.hh"
#include "Statistics.hh"

using std::cout;
using SigC::slot;

//! Constructor.
/*!
 *  \param gui the main GUI entry point.
 *  \param control Interface to the controller.
 */
MainWindow::MainWindow(GUI *g, ControlInterface *c) :
  core_control(c),
  gui(g),
  timers_box(NULL),
  timer_names(NULL),
  timer_times(NULL),
  monitor_suspended(false),
  always_on_top(true)
{
  init();
}


//! Destructor.
MainWindow::~MainWindow()
{
  TRACE_ENTER("MainWindow::~MainWindow");

  if (delete_connection.connected())
    {
      delete_connection.disconnect();
    }
  
  if (timer_times)
    {
      delete [] timer_times;
    }

  if (timer_names)
    {
      delete [] timer_names;
    }

  TRACE_EXIT();
}


//! Initializes the main window.
void
MainWindow::init()
{
  TRACE_ENTER("MainWindow::init");

  set_title("Workrave");
  
  delete_connection =  signal_delete_event().connect(SigC::slot(*this, &MainWindow::on_delete_event));

  // Load config and store it again
  // (in case no config was found and defaults were used)
  load_config();
  store_config();
  
  Configurator *config = gui->get_configurator();
  if (config != NULL)
    {
      config->add_listener(GUIControl::CFG_KEY_MAIN_WINDOW, this);
    }
  
  create_menu();

  set_border_width(2);
  timers_box = manage(new Gtk::Table(GUIControl::TIMER_ID_SIZEOF, 2, false));
  timers_box->set_spacings(2);
  
  timer_names = new Gtk::Widget*[GUIControl::TIMER_ID_SIZEOF];
  timer_times = new TimeBar*[GUIControl::TIMER_ID_SIZEOF];

  for (int count = 0; count < GUIControl::TIMER_ID_SIZEOF; count++)
    {
      GUIControl::TimerData *timer = &GUIControl::get_instance()->timers[count];
      Gtk::Image *img = manage(new Gtk::Image(timer->icon));
      Gtk::Widget *w;
      if (count == GUIControl::TIMER_ID_REST_BREAK)
	{
	  Gtk::Button *b = manage(new Gtk::Button());
          // GTK_WIDGET_UNSET_FLAGS(b->gobj(), GTK_CAN_FOCUS);
	  b->set_relief(Gtk::RELIEF_NONE);
	  b->set_border_width(0);
	  b->add(*img);
	  b->signal_clicked().connect(SigC::slot(*this, &MainWindow::on_menu_restbreak_now));
	  w = b;
	}
      else
	{
	  w = img;
	}
      timer_names[count] = w;
      timers_box->attach(*w, 0, 1, count, count + 1, Gtk::FILL);
      
      timer_times[count] = manage(new TimeBar);
      
      timer_times[count]->set_text_alignment(1);
      timer_times[count]->set_progress(0, 60);
      timer_times[count]->set_text("   Wait   ");
      
      timers_box->attach(*timer_times[count], 1, 2, count, count + 1, Gtk::EXPAND | Gtk::FILL);
    }
  
  add(*timers_box);
  show_all();

  set_resizable(false);

  Glib::RefPtr<Gdk::Window> window = get_window();
  window->set_functions(Gdk::FUNC_CLOSE|Gdk::FUNC_MOVE|Gdk::FUNC_MINIMIZE);
  WindowHints::set_tool_window(Gtk::Widget::gobj(), true);
  WindowHints::set_skip_winlist(Gtk::Widget::gobj(), true);
  setup();
  stick();
  
  TRACE_EXIT();
}


//! Setup configuration settings.
void
MainWindow::setup()
{
  TRACE_ENTER("MainWindow::setup");

  WindowHints::set_always_on_top(Gtk::Widget::gobj(), always_on_top);
  
  if (always_on_top)
    {
      raise();
    }

  TRACE_EXIT()
}


//! Updates the main window.
void
MainWindow::update()
{
  for (unsigned int count = 0; count < GUIControl::TIMER_ID_SIZEOF; count++)
    {
      TimerInterface *timer = GUIControl::get_instance()->timers[count].timer;
      TimeBar *bar = timer_times[count];

      if (timer == NULL)
        {
          // FIXME: error handling.
          continue;
        }
      
      TimerInterface::TimerState timerState = timer->get_state();

      // Collect some data.
      time_t maxActiveTime = timer->get_limit();
      time_t activeTime = timer->get_elapsed_time();
      time_t breakDuration = timer->get_auto_reset();
      time_t idleTime = timer->get_elapsed_idle_time();
      bool overdue = (maxActiveTime < activeTime);
          
      // Set the text
      if (timer->is_limit_enabled() && maxActiveTime != 0)
        {
          bar->set_text(TimeBar::time_to_string(maxActiveTime - activeTime));
        }
      else
        {
          bar->set_text(TimeBar::time_to_string(activeTime));
        }

      // And set the bar.
      bar->set_secondary_progress(0, 0);

      if (timerState == TimerInterface::STATE_INVALID)
        {
          bar->set_bar_color(TimeBar::COLOR_ID_INACTIVE);
          bar->set_progress(0, 60);
          bar->set_text("Wait");
        }
      else
        {
          // Timer is running, show elapsed time.
          bar->set_progress(activeTime, maxActiveTime);
          
          if (overdue)
            {
              bar->set_bar_color(TimeBar::COLOR_ID_OVERDUE);
            }
          else
            {
              bar->set_bar_color(TimeBar::COLOR_ID_ACTIVE);
            }

	  if (timerState == TimerInterface::STATE_STOPPED &&
	      timer->is_auto_reset_enabled() && breakDuration != 0)
	    {
	      // resting.
	      bar->set_secondary_bar_color(TimeBar::COLOR_ID_INACTIVE);
	      bar->set_secondary_progress(idleTime, breakDuration);
	    }
        }
      bar->update();
    }

  return;
}


//! User has closed the main window.
bool
MainWindow::on_delete_event(GdkEventAny *)
{
  TRACE_ENTER("MainWindow::on_delete_event");

  gui->terminate();

  TRACE_EXIT();
  return true;
}



//! Create the popup-menu
void
MainWindow::create_menu()
{
  popup_menu = manage(new Gtk::Menu());
  
  Gtk::Menu::MenuList &menulist = popup_menu->items();

  Gtk::Menu *mode_menu = manage(new Gtk::Menu());
  Gtk::Menu::MenuList &modemenulist = mode_menu->items();

  // Mode menu item
  Gtk::MenuItem *mode_menu_item = manage(new Gtk::MenuItem("_Mode",true));
  mode_menu_item->set_submenu(*mode_menu);
  mode_menu_item->show();

  Gtk::RadioMenuItem::Group gr;
  // Suspend menu item.
  normal_menu_item = manage(new Gtk::RadioMenuItem(gr, "_Normal", true));
  normal_menu_item->signal_toggled().connect(SigC::slot(*this, &MainWindow::on_menu_normal));
  normal_menu_item->show();
  modemenulist.push_back(*normal_menu_item);

  // Suspend menu item.
  suspend_menu_item = manage(new Gtk::RadioMenuItem(gr, "_Suspended", true));
  suspend_menu_item->signal_toggled().connect(SigC::slot(*this, &MainWindow::on_menu_suspend));
  suspend_menu_item->show();
  modemenulist.push_back(*suspend_menu_item);

  // Quiet menu item.
  quiet_menu_item = manage(new Gtk::RadioMenuItem(gr, "Q_uiet", true));
  quiet_menu_item->signal_toggled().connect(SigC::slot(*this, &MainWindow::on_menu_quiet));
  quiet_menu_item->show();
  modemenulist.push_back(*quiet_menu_item);
  
  // FIXME: add separators, etc...
  menulist.push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::PREFERENCES,
                                                      SigC::slot(*this, &MainWindow::on_menu_preferences)));


  Gtk::Image *img = manage(new Gtk::Image(GUIControl::get_instance()->timers[GUIControl::TIMER_ID_REST_BREAK].icon));
  menulist.push_back(Gtk::Menu_Helpers::ImageMenuElem
                     ("_Rest break",
                      Gtk::Menu::AccelKey("<control>r"),
                      *img,
                      SigC::slot(*this, &MainWindow::on_menu_restbreak_now)));

  menulist.push_back(*mode_menu_item);

#ifndef NDEBUG
  menulist.push_back(Gtk::Menu_Helpers::MenuElem("Statistics",
                                                 SigC::slot(*this, &MainWindow::on_menu_statistics)));
  
  menulist.push_back(Gtk::Menu_Helpers::MenuElem("_Test",
                                                 SigC::slot(*this, &MainWindow::on_test_me)));
#endif
  
#ifdef HAVE_GNOME
  menulist.push_back(Gtk::Menu_Helpers::StockMenuElem
		     (Gtk::StockID(GNOME_STOCK_ABOUT),
		      SigC::slot(*this, &MainWindow::on_menu_about)));
#else
  menulist.push_back(Gtk::Menu_Helpers::MenuElem
		     ("About...",
		      SigC::slot(*this, &MainWindow::on_menu_about)));
#endif

  menulist.push_back(Gtk::Menu_Helpers::StockMenuElem(Gtk::Stock::QUIT,
                                                 SigC::slot(*this, &MainWindow::on_menu_quit)));

  // And register button callback
  set_events(get_events() | Gdk::BUTTON_PRESS_MASK);
  signal_button_press_event().connect(SigC::slot(*this, &MainWindow::on_button_event));
}


void
MainWindow::load_config()
{
  assert(gui != NULL);
  
  Configurator *config = gui->get_configurator();
  if (config != NULL)
    {
      bool onTop;
      if (config->get_value(GUIControl::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, &onTop))
        {
          always_on_top = onTop;
        }
    }
}


void
MainWindow::store_config()
{
  assert(gui != NULL);
  
  Configurator *config = gui->get_configurator();
  if (config != NULL)
    {
      config->set_value(GUIControl::CFG_KEY_MAIN_WINDOW_ALWAYS_ON_TOP, always_on_top);
      config->save();
    }
}


void
MainWindow::config_changed_notify(string key)
{
  TRACE_ENTER("MainWindow::config_changed_notify");

  load_config();
  setup();

  TRACE_EXIT();
}


//! Users pressed some mouse button in the main window.
bool
MainWindow::on_button_event(GdkEventButton *event)
{
  TRACE_ENTER("MainWindow::on_button_event");
  bool ret = false;

  // TODO: magic number.
  if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3))
    {
      popup_menu->popup(event->button, event->time);
      ret = true;
    }

  TRACE_EXIT();
  return ret;
}


//! User requested application quit....
void
MainWindow::on_menu_quit()
{
  TRACE_ENTER("MainWindow::on_menu_quit");

  gui->terminate();

  TRACE_EXIT();
}


//! User requested immediate restbreak.
void
MainWindow::on_menu_restbreak_now()
{
  gui->restbreak_now();
}


//! User requested immediate restbreak.
void
MainWindow::on_menu_quiet()
{
  TRACE_ENTER("MainWindow::on_menu_quiet");

  gui->set_operation_mode(GUIControl::OPERATION_MODE_QUIET);

  TRACE_EXIT();
}


//! User requested immediate restbreak.
void
MainWindow::on_menu_suspend()
{
  TRACE_ENTER("MainWindow::on_menu_suspend");

  gui->set_operation_mode(GUIControl::OPERATION_MODE_SUSPENDED);

  TRACE_EXIT();
}

void
MainWindow::on_menu_normal()
{
  gui->set_operation_mode(GUIControl::OPERATION_MODE_NORMAL);
}


#ifndef NDEBUG
//! User test code.
void
MainWindow::on_test_me()
{
  core_control->test_me();

  Statistics *stats = Statistics::get_instance();
  stats->dump();
}
#endif

//! Preferences Dialog.
void
MainWindow::on_menu_preferences()
{
  GUIControl::OperationMode mode;
  GUIControl *ctrl = GUIControl::get_instance();
  mode = ctrl->set_operation_mode(GUIControl::OPERATION_MODE_QUIET);

  PreferencesDialog *dialog = new PreferencesDialog();
  dialog->run();
  delete dialog;

  ctrl->set_operation_mode(mode);
}


//! Preferences Dialog.
void
MainWindow::on_menu_statistics()
{
  GUIControl::OperationMode mode;
  GUIControl *ctrl = GUIControl::get_instance();
  mode = ctrl->set_operation_mode(GUIControl::OPERATION_MODE_QUIET);

  StatisticsDialog *dialog = new StatisticsDialog();
  dialog->run();
  delete dialog;

  ctrl->set_operation_mode(mode);
}


//! About Dialog.
void
MainWindow::on_menu_about()
{
  const gchar *authors[] = {
   "Rob Caelers <robc@krandor.org>",
   "Raymond Penners <raymond@dotsphinx.com>",
   NULL
  };
  string icon = Util::complete_directory("workrave.png",
                                         Util::SEARCH_PATH_IMAGES);
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(icon.c_str(), NULL);  
  gtk_widget_show (gnome_about_new
                   ("Workrave", VERSION,
                    "Copyright 2001-2002 Rob Caelers & Raymond Penners",
                    "This program assists in the prevention and recovery"
                    " of Repetitive Strain Injury (RSI).",
                    (const gchar **) authors,
                    (const gchar **) NULL,
                    NULL,
                    pixbuf));
}

