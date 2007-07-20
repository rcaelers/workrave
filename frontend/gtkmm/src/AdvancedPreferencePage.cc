// AdvancedPreferencePage.hh --- Advanced preferences
//
// Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
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
//
// This is the GTK setup for the Advanced Preference Page.
// If someone wants to expand this and add troubleshooting 
// options for operating systems other than Windows, I can 
// move Win32 specific to a separate file.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "preinclude.h"

#include "debug.hh"
#include "nls.h"

#include <sstream>
#include <unistd.h>

#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/button.h>
#include <gtkmm/notebook.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treeviewcolumn.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/stock.h>
#include <gtkmm/textview.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/enums.h>

#include <gtk/gtktextbuffer.h>

#include "AdvancedPreferencePage.hh"

#include "Hig.hh"
#include "GtkUtil.hh"

#include "ICore.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"



AdvancedPreferencePage::AdvancedPreferencePage()
  : Gtk::VBox( false, 6 )
{
  TRACE_ENTER( "AdvancedPreferencePage::AdvancedPreferencePage" );
  
  Gtk::Notebook *notebook = manage( new Gtk::Notebook() );
  //notebook->set_tab_pos( Gtk::POS_TOP );
  
  Gtk::ScrolledWindow *scroller = manage( new Gtk::ScrolledWindow() );
  scroller->set_shadow_type( Gtk::SHADOW_ETCHED_OUT );
  scroller->set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
  scroller->set_border_width( 4 );
  
  HigCategoriesPanel *main_panel = manage( new HigCategoriesPanel() );
  main_panel->set_border_width( 12 );
  
  
#if defined(WIN32)
  forcebox = manage(new Gtk::CheckButton( 
          _( "Force Break Window Focus (experimental)" ) ) );
  forcebox->signal_toggled().connect( MEMBER_SLOT( *this, 
          &AdvancedPreferencePage::forcebox_signal_toggled ) );
  forcebox->signal_focus_out_event().connect( MEMBER_SLOT( *this, 
          &AdvancedPreferencePage::forcebox_signal_focus_out_event ) );
  HigCategoryPanel *forcebox_panel = manage( new HigCategoryPanel( *forcebox ) );
  Gtk::Label *forcebox_label = manage( new Gtk::Label(
          _( "A break window can lose focus due to either an operating system or application compatibility issue. If you enable this option, your input to other applications is disrupted during your breaks. Workrave has to be restarted when this options if changed." ) ) );
  forcebox_label->set_line_wrap( true );
  //forcebox_label->set_max_width_chars( 100 );
  forcebox_panel->add( *forcebox_label );
  main_panel->add( *forcebox_panel );
  
  nohooksbox = manage( new Gtk::CheckButton( 
          _( "Enable Alternate Activity Monitor (experimental)" ) ) );
  nohooksbox->signal_toggled().connect( MEMBER_SLOT( *this, 
          &AdvancedPreferencePage::nohooksbox_signal_toggled ) );
  HigCategoryPanel *nohooksbox_panel = manage( new HigCategoryPanel( *nohooksbox ) );
  Gtk::Label *nohooksbox_label = manage( new Gtk::Label(
          _( "Some applications aren't compatible with the default keyboard/mouse monitor because it uses global hooks. If you enable this option, an alternate monitor is enabled. Mouse & keyboard statistics are unavailable when the alternate monitor is enabled. Workrave has to be restarted when this options if changed." ) ) );
  nohooksbox_label->set_line_wrap( true );
  nohooksbox_panel->add( *nohooksbox_label );
  main_panel->add( *nohooksbox_panel );
#endif
  
  scroller->add( *main_panel );
  notebook->append_page( *scroller, _( "Troubleshooting" ) );
  pack_start( *notebook, true, true, 0 );
  
  notebook->show_all();
  notebook->set_current_page( 0 );
  
  init();
  TRACE_EXIT();
}

AdvancedPreferencePage::~AdvancedPreferencePage()
{
  TRACE_ENTER( "AdvancedPreferencePage::~AdvancedPreferencePage" );
  TRACE_EXIT();
}

#if defined(WIN32)
void AdvancedPreferencePage::forcebox_signal_toggled()
{
  bool enabled = forcebox->get_active();
  CoreFactory::get_configurator()->set_value( "advanced/force_focus", enabled );
}

bool AdvancedPreferencePage::
forcebox_signal_focus_out_event(GdkEventFocus *test)
{
  /*
    The value returned from the signal handler indicates whether it 
    has fully "handled" the event. If the value is false then gtkmm 
    will pass the event on to the next signal handler. If the value 
    is true then no other signal handlers will need to be called.
    http://www.gtkmm.org/docs/gtkmm-2.4/docs/tutorial/html/apbs06.html
  */
  return false;
}

bool AdvancedPreferencePage::forcebox_get_config() 
{
  bool enabled;
  if( CoreFactory::get_configurator()->get_value( "advanced/force_focus", &enabled ) == 0 )
    enabled = false;
  return enabled;
}

void AdvancedPreferencePage::nohooksbox_signal_toggled()
{
  bool enabled = nohooksbox->get_active();

  if( LOBYTE( LOWORD( GetVersion() ) ) < 5 )
    {
      enabled = false;
      nohooksbox->set_active( false );
    }
  CoreFactory::get_configurator()->set_value( "advanced/nohooks", enabled );
}

bool AdvancedPreferencePage::nohooksbox_get_config() 
{
  bool enabled;
  if( CoreFactory::get_configurator()->get_value( "advanced/nohooks", &enabled ) == 0 )
    enabled = false;
  return enabled;
}
#endif


void AdvancedPreferencePage::init()
{
#if defined(WIN32)
  forcebox->set_active( forcebox_get_config() );
  nohooksbox->set_active( nohooksbox_get_config() );
  if( LOBYTE( LOWORD( GetVersion() ) ) < 5 )
    // Alternate monitor can't be used if OS < Win2000.
    nohooksbox->set_sensitive( false );
#endif
}
