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

  Gtk::Notebook *notebook = manage( new Gtk::Notebook() );
  //notebook->set_tab_pos( Gtk::POS_TOP );

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
  main_panel->add( *forcebox );
  
  string force_focus_tip =
    _("A break window can lose focus due to either an operating\n"
      "system or application compatibility issue. As a result,\n"
      "the user can no longer skip or postpone the break.\n"
      "This option makes sure that break windows\n"
      "automatically regain focus.");
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
  row[monitor_columns.col_name] = _("Alternate monitor using low-level global hook");

  if (LOBYTE( LOWORD( GetVersion() ) ) >= 5)
    {
      row = *(monitor_model->append());
      row[monitor_columns.col_id] = "nohook";
      row[monitor_columns.col_name] = _("Alternate monitor without global hooks");
    }
  
  monitor_combo->pack_start(monitor_columns.col_name);
  monitor_combo->signal_changed().connect(MEMBER_SLOT(*this,
            &AdvancedPreferencePage::monitor_signal_changed));

  string info_text =
    _("Some applications are incompatible with the default keyboard\n"
      "and mouse monitoring (because it uses global hooks). Workrave\n"
      "offers two experimental alternate monitoring methods.\n"
      "\n"
      "The first alternate method uses low-level hooks exclusively.\n"
      "This method allows monitoring of administrative applications on\n"
      "Vista. On rare occasions, you might notice very slight mouse or\n"
      "keyboard lag.\n"
      "\n"
      "The second one does not use any hooks at all. This method also\n"
      "allows monitoring of administrative applications on Vista but\n"
      "mouse &amp; keyboard statistics are unavailable when this\n"
      "monitor is selected.\n"
      "\n"
      "Vista users: By default, Workrave does not monitor your\n"
      "interaction with administrative applications. This is due to\n"
      "security restrictions in Vista. If you would like Workrave to\n"
      "monitor your interaction with these applications,\n"
      "please enable the second alternate activity monitor.");

  EventLabel *label = manage(new EventLabel("Monitoring method:"));
  label->set_alignment(0.0);
  GUI::get_instance()->get_tooltips()->set_tip(*label, info_text);

  Gtk::HBox *box = manage(new Gtk::HBox());
  box->set_spacing(6);
  box->pack_start(*label, false, true, 0);
  box->pack_start(*monitor_combo, false, false, 0);
  main_panel->add( *box );
#endif
  
  notebook->append_page( *main_panel, _( "Troubleshooting" ) );
  pack_start( *notebook, true, true, 0 );

  notebook->show_all();
  notebook->set_current_page( 0 );

  init();

  show_all();
  
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

  // if lowlevel_monitor has a value that will be set on quit,
  // that value will be returned, not the actual value.
  if( CoreFactory::get_configurator()->
      get_value_on_quit( "advanced/monitor", &monitor ) == true )
          return monitor;
  
  CoreFactory::get_configurator()->
      get_value_default( "advanced/monitor", &monitor, "normal" );
  
  return monitor;
}
#endif


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
