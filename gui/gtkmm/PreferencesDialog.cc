// PreferencesDialog.cc --- Preferences dialog
//
// Copyright (C) 2002, 2003 Raymond Penners <raymond@dotsphinx.com>
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
// $Id$

// TODO: only when needed.
#define NOMINMAX

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"

#include "nls.h"
#include "debug.hh"

#include <unistd.h>
#include <assert.h>

#include "PreferencesDialog.hh"
#include "TimerPreferencesPanel.hh"
#include "Configurator.hh"
#include "SoundPlayer.hh"
#include "TimeEntry.hh"
#include "Util.hh"
#include "MainWindow.hh"
#include "GtkUtil.hh"
#include "Hig.hh"

#ifdef HAVE_DISTRIBUTION
#include "NetworkPreferencePage.hh"
#endif
#ifdef HAVE_X
#include "AppletPreferencePage.hh"
#endif



PreferencesDialog::PreferencesDialog()
  : HigDialog(_("Preferences"), false, false)
{
  TRACE_ENTER("PreferencesDialog::PreferencesDialog");

  // Pages
  Gtk::Widget *gui_page = manage(create_gui_page());
  Gtk::Widget *timer_page = manage(create_timer_page());
#ifdef HAVE_X
  Gtk::Widget *applet_page = manage(create_applet_page());
#endif
#ifdef HAVE_DISTRIBUTION
  Gtk::Widget *network_page = manage(create_network_page());
#endif
  
  // Notebook
  Gtk::Notebook *notebook = manage(new Gtk::Notebook());
  notebook->set_tab_pos (Gtk::POS_TOP);  
  notebook->pages().push_back(Gtk::Notebook_Helpers::TabElem
                              (*timer_page, _("Timers")));
  notebook->pages().push_back(Gtk::Notebook_Helpers::TabElem
                              (*gui_page, _("User interface")));
#ifdef HAVE_X
  notebook->pages().push_back(Gtk::Notebook_Helpers::TabElem
                              (*applet_page, _("Applet")));
#endif  
#ifdef HAVE_DISTRIBUTION
  notebook->pages().push_back(Gtk::Notebook_Helpers::TabElem
                              (*network_page, _("Network")));
#endif
  
  // Dialog
  get_vbox()->pack_start(*notebook, false, false, 0);
  add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

  show_all();


  TRACE_EXIT();
}


//! Destructor.
PreferencesDialog::~PreferencesDialog()
{
  TRACE_ENTER("PreferencesDialog::~PreferencesDialog");
  TRACE_EXIT();
}



Gtk::Widget *
PreferencesDialog::create_gui_page()
{
  // Always-on-top
  ontop_cb = manage
    (new Gtk::CheckButton
     (_("The main window stays always on top of other windows")));
  ontop_cb->signal_toggled().connect(SigC::slot(*this, &PreferencesDialog::on_always_on_top_toggled));
  ontop_cb->set_active(MainWindow::get_always_on_top());

  // Sound types
  sound_button  = manage(new Gtk::OptionMenu());
  Gtk::Menu *sound_menu = manage(new Gtk::Menu());
  Gtk::Menu::MenuList &sound_list = sound_menu->items();
  sound_button->set_menu(*sound_menu);
  sound_list.push_back(Gtk::Menu_Helpers::MenuElem(_("No sounds")));
  sound_list.push_back(Gtk::Menu_Helpers::MenuElem
                       (_("Play sounds using sound card")));
  sound_list.push_back(Gtk::Menu_Helpers::MenuElem
                       (_("Play sounds using built-in speaker")));
  int idx;
  if (! SoundPlayer::is_enabled())
    idx = 0;
  else
    {
      if (SoundPlayer::DEVICE_SPEAKER == SoundPlayer::get_device())
        idx = 2;
      else
        idx = 1;
    }
  sound_button->set_history(idx);
  sound_button->signal_changed().connect(SigC::slot(*this, &PreferencesDialog::on_sound_changed));
  
  // Tray start
  start_in_tray_cb
    = manage(new Gtk::CheckButton(_("Hide main window at start-up")));
  start_in_tray_cb->signal_toggled()
    .connect(SigC::slot(*this,
			&PreferencesDialog::on_start_in_tray_toggled));
  start_in_tray_cb->set_active(MainWindow::get_start_in_tray());

  // Options
  HigCategoryPanel *panel = manage(new HigCategoryPanel(_("Options")));
  panel->add(*ontop_cb);
  panel->add(*start_in_tray_cb);
  panel->add(_("Sound:"), *sound_button);

  // Page
  Gtk::VBox *gui_page
    = create_page
    (_("You can configure the user interface related settings from here."),
     "display.png");
  gui_page->pack_start(*panel, false, false, 0);

  return gui_page;
}

Gtk::Widget *
PreferencesDialog::create_timer_page()
{
  // Timers page
  Gtk::VBox *timer_page
    = create_page
    (_("This dialog allows you to change the settings of the timers.  Each unit\n"
     "of time is broken down into hours, minutes and seconds (also known as\n"
     "the \"hh:mm:ss\" format).  These can all be controlled individually."),
     "time.png");
  Gtk::Notebook *tnotebook = manage(new Gtk::Notebook());
  tnotebook->set_tab_pos (Gtk::POS_TOP);  
  for (int i = 0; i < GUIControl::BREAK_ID_SIZEOF; i++)
    {
      // Label
      Gtk::Widget *box = manage(GtkUtil::create_label_for_break
                                ((GUIControl::BreakId) i));
      TimerPreferencesPanel *tp = manage(new TimerPreferencesPanel(GUIControl::BreakId(i)));
      box->show_all();
      tnotebook->pages().push_back(Gtk::Notebook_Helpers::TabElem(*tp, *box));
    }
  timer_page->pack_start(*tnotebook, false, false, 0);

  return timer_page;
}


#ifdef HAVE_X
Gtk::Widget *
PreferencesDialog::create_applet_page()
{
  // Timers page
  Gtk::VBox *applet_page
    = create_page
    (_("You can configure the applet related settings from here."),
     "applet.png");

  Gtk::Widget *page = manage(new AppletPreferencePage());
  
  applet_page->pack_start(*page, true, true, 0);

  return applet_page;
}
#endif


#ifdef HAVE_DISTRIBUTION
Gtk::Widget *
PreferencesDialog::create_network_page()
{
  // Timers page
  Gtk::VBox *network_page
    = create_page
    (_("You can connect several instances of Workrave in a network. All connected\n"
       "instances share the same timer information, meaning you will be reminded\n"
       "of your breaks even if you switch computers."),
     "network.png");

  Gtk::Widget *page = manage(new NetworkPreferencePage());
  
  network_page->pack_start(*page, true, true, 0);

  return network_page;
}
#endif

Gtk::VBox *
PreferencesDialog::create_page(const char *label, const char *image)
{
  Gtk::HBox *info_box = manage(new Gtk::HBox(false));
  string icon = Util::complete_directory(image, Util::SEARCH_PATH_IMAGES);
  Gtk::Image *info_img = manage(new Gtk::Image(icon));
  Gtk::Label *info_lab = manage(new Gtk::Label(label));
  info_box->pack_start(*info_img, false, false, 6);
  info_box->pack_start(*info_lab, false, true, 6);

  Gtk::VBox *page = manage(new Gtk::VBox(false, 6));
  page->pack_start(*info_box, false, true, 0);
  page->set_border_width(6);
  return page;
}

void
PreferencesDialog::on_always_on_top_toggled()
{
  MainWindow::set_always_on_top(ontop_cb->get_active());
}

void
PreferencesDialog::on_sound_changed()
{
  int idx = sound_button->get_history();
  SoundPlayer::set_enabled(idx > 0);
  if (idx > 0)
    {
      SoundPlayer::Device dev = idx == 1
        ? SoundPlayer::DEVICE_SOUNDCARD
        : SoundPlayer::DEVICE_SPEAKER;
      SoundPlayer::set_device(dev);
    }
}

void
PreferencesDialog::on_start_in_tray_toggled()
{
  MainWindow::set_start_in_tray(start_in_tray_cb->get_active());
}


int
PreferencesDialog::run()
{
//   int id = Gtk::Dialog::run();
//   GUIControl::get_instance()->get_configurator()->save();

  show_all();
  return 0;
}


bool
PreferencesDialog::on_focus_in_event(GdkEventFocus *event)
{ 
  TRACE_ENTER("PreferencesDialog::focus_in");

  GUIControl *gui_control = GUIControl::get_instance();
  mode = gui_control->set_operation_mode(GUIControl::OPERATION_MODE_QUIET);

  TRACE_EXIT();
}


bool
PreferencesDialog::on_focus_out_event(GdkEventFocus *event)
{ 
  TRACE_ENTER("PreferencesDialog::focus_out");
  GUIControl *gui_control = GUIControl::get_instance();
  gui_control->set_operation_mode(mode);
  TRACE_EXIT();
}
