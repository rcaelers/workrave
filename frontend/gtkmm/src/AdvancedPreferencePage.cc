// AdvancedPreferencePage.cc --- Advanced preferences
//
// Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
// Copyright (C) 2007 Rob Caelers <robc@krandor.org>
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
#include <gtkmm/treemodel.h>
#include <gtkmm/treestore.h>
#include <gtkmm/button.h>
#include <gtkmm/notebook.h>
#include <gtkmm/eventbox.h>

#include "AdvancedPreferencePage.hh"

#include "Hig.hh"
#include "GtkUtil.hh"
#include "EventLabel.hh"

#include "ICore.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"
#include "GUI.hh"


AdvancedPreferencePage::AdvancedPreferencePage()
  : Gtk::VBox( false, 6 )
{
  TRACE_ENTER( "AdvancedPreferencePage::AdvancedPreferencePage" );

#if 1 // defined(WIN32)

  Gtk::Notebook *notebook = manage( new Gtk::Notebook() );
  //notebook->set_tab_pos( Gtk::POS_TOP );

  HigCategoriesPanel *main_panel = manage( new HigCategoriesPanel() );
  main_panel->set_border_width( 12 );
  
  Gtk::Label *exp_label = manage( new Gtk::Label(
                _( "<b>Experimental preferences. Restart required for changes to take effect</b>" ) ) );
  exp_label->set_use_markup( true );
  main_panel->add( *exp_label );

  forcebox = manage(new Gtk::CheckButton(_("Force Break Window Focus")));
  forcebox->signal_toggled().connect(MEMBER_SLOT(*this,
                                                 &AdvancedPreferencePage::forcebox_signal_toggled));
  main_panel->add( *forcebox );
  
  string force_focus_tip =
    _("A break window can lose focus due to either an operating\n"
      "system or application compatibility issue. As a result, the\n"
      "user can no longer skip or postpone the break. This option\n"
      "make sure that break windows automatically regain focus.");
  GUI::get_instance()->get_tooltips()->set_tip(*forcebox, force_focus_tip);
  
  // Monitoring
  monitor_combo = manage(new Gtk::ComboBox());
  monitor_model = Gtk::ListStore::create(monitor_columns);
  monitor_combo->set_model(monitor_model);

  Gtk::TreeModel::Row row = *(monitor_model->append());
  row[monitor_columns.col_id] = "normal";
  row[monitor_columns.col_name] = _("Normal monitor");

  row = *(monitor_model->append());
  row[monitor_columns.col_id] = "lowlevel";
  row[monitor_columns.col_name] = _("Alternate monitor using low-level global hooks");

  if (1) // FIXME: LOBYTE( LOWORD( GetVersion() ) ) >= 5)
    {
      row = *(monitor_model->append());
      row[monitor_columns.col_id] = "nohook";
      row[monitor_columns.col_name] = _("Alternate monitor -- no hooks");
    }
  
  monitor_combo->pack_start(monitor_columns.col_name);
  monitor_combo->signal_changed().connect(MEMBER_SLOT(*this,
                &AdvancedPreferencePage::monitor_signal_changed));

  string info_text =

    _("Workrave's default keyboard/mouse monitor uses global hooks.\n"
      "This monitoring method is not compatible with all applications.\n"
      "Workrave might not be able to monitor your activity in certain\n"
      "applications if you are using the default monitor.\n"
      "\n"
      "You can try enabling an alternate monitor instead:\n"
      "\n"
      "The alternate low-level hook monitor is experimental. It processes\n"
      "information faster to avoid any mouse or keyboard lag that may\n"
      "(rarely) occur when using the default monitor. It is 64-bit\n"
      "compatible and can properly monitor your mouse & keyboard activity\n"
      "in Internet Explorer on Vista.\n"
      "\n"
      "The other alternate monitor does not use any hooks at all to monitor\n"
      "your activity, instead relying on the OS to determine when you are\n"
      "active. This monitor is stable, and it is also 64-bit compatible.\n"
      "It is recommended for gamers, and Vista users. On some computers,\n"
      "Workrave might enable this monitor automatically. There are no mouse\n"
      "or keyboard statistics available when this monitor is enabled.\n"
      "\n"
      "Please note: To disable hooks completely, you must also disable\n"
      "block input: User interface > Block mode: > No blocking\n"
      );

  EventLabel *label = manage(new EventLabel("Monitoring method:"));
  label->set_alignment(0.0);
  GUI::get_instance()->get_tooltips()->set_tip(*label, info_text);

  Gtk::HBox *box = manage(new Gtk::HBox());
  box->set_spacing(6);
  box->pack_start(*label, false, true, 0);
  box->pack_start(*monitor_combo, false, false, 0);

  main_panel->add( *box );
  
  notebook->append_page( *main_panel, _( "Troubleshooting" ) );
  pack_start( *notebook, true, true, 0 );

  notebook->show_all();
  notebook->set_current_page( 0 );

  init();

  show_all();

#endif
  
  TRACE_EXIT();
}

AdvancedPreferencePage::~AdvancedPreferencePage()
{
  TRACE_ENTER( "AdvancedPreferencePage::~AdvancedPreferencePage" );
  TRACE_EXIT();
}

#if 1 //defined(WIN32)
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


void
AdvancedPreferencePage::monitor_signal_changed()
{
  monitor_combo->set_sensitive( false );

  Gtk::TreeModel::iterator iter = monitor_combo->get_active();
  if (iter)
  {
    Gtk::TreeModel::Row row = *iter;
    if (row)
    {
      Glib::ustring id = row[monitor_columns.col_id];
      string x = id;
      
      CoreFactory::get_configurator()->
        set_value_on_quit( "advanced/monitor", id);
    }
  }

  monitor_combo->set_sensitive( true );
}


string AdvancedPreferencePage::monitor_get_config()
{
  string monitor;

  // if monitor has a value that will be set on quit,
  // that value will be returned, not the actual value.
  if( CoreFactory::get_configurator()->
      get_value_on_quit( "advanced/monitor", &monitor ) == true )
          return monitor;
  
  CoreFactory::get_configurator()->
      get_value_default( "advanced/monitor", &monitor, "normal" );
  
  return monitor;
}


void AdvancedPreferencePage::init()
{
  forcebox->set_focus_on_click(false);

  // Set monitor methods
  string monitor = monitor_get_config();

  typedef Gtk::TreeStore::Children type_children;
  type_children children = monitor_model->children();
  for (type_children::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
      Gtk::TreeModel::Row row = *iter;

      if (row[monitor_columns.col_id] == monitor)
        {
          monitor_combo->set_active(iter);
        };
    }

  forcebox->set_active(forcebox_get_config());
}

#endif
