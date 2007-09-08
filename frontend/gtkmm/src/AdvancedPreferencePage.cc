// AdvancedPreferencePage.cc --- Advanced preferences
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
  scroller->set_shadow_type( Gtk::SHADOW_NONE );
  scroller->set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
  scroller->set_border_width( 4 );

  HigCategoriesPanel *main_panel = manage( new HigCategoriesPanel() );
  main_panel->set_border_width( 12 );
  
  Gtk::Label *restart_required_label = manage( new Gtk::Label(
          _( "<b>Restart required for all preferences.</b>" ) ) );
  restart_required_label->set_line_wrap( true );
  restart_required_label->set_use_markup( true );
  main_panel->add( *restart_required_label );
  
#if defined(WIN32)
  forcebox = manage(new Gtk::CheckButton(
          _( "Force Break Window Focus (experimental)" ) ) );
  forcebox->signal_toggled().connect( MEMBER_SLOT( *this,
          &AdvancedPreferencePage::forcebox_signal_toggled ) );
  HigCategoryPanel *forcebox_panel = manage( new HigCategoryPanel( *forcebox ) );
  Gtk::Label *forcebox_label = manage( new Gtk::Label(
          _( "A break window can lose focus due to either an operating system or application compatibility issue. If you enable this option, your input to other applications is disrupted during your breaks." ) ) );
  forcebox_label->set_line_wrap( true );
  //forcebox_label->set_max_width_chars( 100 );
  forcebox_panel->add( *forcebox_label );
  main_panel->add( *forcebox_panel );
  
  llhooksbox = manage( new Gtk::CheckButton(
          _( "Enable Alternate Low-Level Hook Activity Monitor (experimental)" ) ) );
  llhooksbox->signal_toggled().connect( MEMBER_SLOT( *this,
          &AdvancedPreferencePage::llhooksbox_signal_toggled ) );
  HigCategoryPanel *llhooksbox_panel = manage( new HigCategoryPanel( *llhooksbox ) );
  Gtk::Label *llhooksbox_label = manage( new Gtk::Label(
          _( "The default keyboard/mouse monitor uses global hooks. On rare occasion, you might notice very slight mouse or keyboard lag. This alternate monitor uses low-level hooks exclusively, and processes information with a higher priority." ) ) );
  llhooksbox_label->set_line_wrap( true );
  llhooksbox_panel->add( *llhooksbox_label );
  main_panel->add( *llhooksbox_panel );
  
  nohooksbox = manage( new Gtk::CheckButton(
          _( "Enable Alternate Activity Monitor (recommended for gamers)" ) ) );
  nohooksbox->signal_toggled().connect( MEMBER_SLOT( *this,
          &AdvancedPreferencePage::nohooksbox_signal_toggled ) );
  HigCategoryPanel *nohooksbox_panel = manage( new HigCategoryPanel( *nohooksbox ) );
  Gtk::Label *nohooksbox_label = manage( new Gtk::Label(
          _( "Some applications aren't compatible with the default keyboard/mouse monitor because it uses global hooks. If you enable this option, an alternate monitor is enabled. Mouse & keyboard statistics are unavailable when the alternate monitor is enabled." ) ) );
  nohooksbox_label->set_line_wrap( true );
  nohooksbox_panel->add( *nohooksbox_label );
  main_panel->add( *nohooksbox_panel );
  
  Gtk::Label *more_information_label = manage( new Gtk::Label(
          _( "<b>Vista users:</b> By default, Workrave does not monitor your interaction with administrative applications. This is due to security restrictions in Vista. If you would like Workrave to monitor your interaction with these applications, please enable the alternate activity monitor that's recommended for gamers." ) ) );
  more_information_label->set_line_wrap( true );
  more_information_label->set_use_markup( true );
  main_panel->add( *more_information_label );
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
  forcebox->set_sensitive( false );
  CoreFactory::get_configurator()->
      set_value_on_quit( "advanced/force_focus", forcebox->get_active() );
  forcebox->set_sensitive( true );
}


bool AdvancedPreferencePage::forcebox_get_config()
{
  bool enabled;

  // if force_focus has a value that will be set on quit,
  // that value will be returned, not the actual value.
  if( CoreFactory::get_configurator()->
      get_value_on_quit( "advanced/force_focus", &enabled ) == true )
          return enabled;

  CoreFactory::get_configurator()->
      get_value_default( "advanced/force_focus", &enabled, false );

  return enabled;
}


void AdvancedPreferencePage::nohooksbox_signal_toggled()
{
  llhooksbox->set_sensitive( false );
  nohooksbox->set_sensitive( false );
  
  bool enabled = nohooksbox->get_active();
  
  CoreFactory::get_configurator()->
      set_value_on_quit( "advanced/nohooks", enabled );
  
  // enable only one alternate monitor, not both...
  if( enabled && llhooksbox->get_active() )
      llhooksbox->set_active( false );
      //llhooksbox->toggled();
  
  llhooksbox->set_sensitive( true );
  nohooksbox->set_sensitive( true );
}

bool AdvancedPreferencePage::nohooksbox_get_config()
{
  bool enabled;

  // if nohooks has a value that will be set on quit,
  // that value will be returned, not the actual value.
  if( CoreFactory::get_configurator()->
      get_value_on_quit( "advanced/nohooks", &enabled ) == true )
          return enabled;

  CoreFactory::get_configurator()->
      get_value_default( "advanced/nohooks", &enabled, false );

  return enabled;
}


void AdvancedPreferencePage::llhooksbox_signal_toggled()
{
  nohooksbox->set_sensitive( false );
  llhooksbox->set_sensitive( false );
  
  bool enabled = llhooksbox->get_active();
  
  CoreFactory::get_configurator()->
      set_value_on_quit( "advanced/lowlevel_monitor", enabled );
  
  // enable only one alternate monitor, not both...
  if( enabled && nohooksbox->get_active() )
      nohooksbox->set_active( false );
      //nohooksbox->toggled();
  
  nohooksbox->set_sensitive( true );
  llhooksbox->set_sensitive( true );
}

bool AdvancedPreferencePage::llhooksbox_get_config()
{
  bool enabled;
  
  // if lowlevel_monitor has a value that will be set on quit,
  // that value will be returned, not the actual value.
  if( CoreFactory::get_configurator()->
      get_value_on_quit( "advanced/lowlevel_monitor", &enabled ) == true )
          return enabled;
  
  CoreFactory::get_configurator()->
      get_value_default( "advanced/lowlevel_monitor", &enabled, false );
  
  return enabled;
}
#endif


void AdvancedPreferencePage::init()
{
#if defined(WIN32)
  forcebox->set_focus_on_click( false );
  nohooksbox->set_focus_on_click( false );
  llhooksbox->set_focus_on_click( false );
  forcebox->set_active( forcebox_get_config() );
  nohooksbox->set_active( nohooksbox_get_config() );
  llhooksbox->set_active( llhooksbox_get_config() );
  if( LOBYTE( LOWORD( GetVersion() ) ) < 5 )
    // Alternate monitor can't be used if OS < Win2000.
    nohooksbox->set_sensitive( false );
#endif
}
