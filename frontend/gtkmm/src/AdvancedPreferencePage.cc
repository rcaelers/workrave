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
#include <gtkmm/treemodel.h>
#include <gtkmm/treestore.h>
#include <gtkmm/button.h>
#include <gtkmm/notebook.h>

#include "AdvancedPreferencePage.hh"

#include "Hig.hh"
#include "GtkUtil.hh"

#include "ICore.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"
#include "GUI.hh"


AdvancedPreferencePage::AdvancedPreferencePage()
  : Gtk::VBox( false, 6 )
{
  TRACE_ENTER( "AdvancedPreferencePage::AdvancedPreferencePage" );

#ifdef WIN32  
  Gtk::Notebook *notebook = manage(new Gtk::Notebook());

  // Force focus
  
  force_focus_cb = manage(new Gtk::CheckButton(_("Force Break Window Focus" )));

  string force_focus_tip =
    _("A break window can lose focus due to either an operating system\n"
      "or application compatibility issue. As a result, the user can no\n"
      "longer skip or postpone the break. This option makes sure that\n"
      "break windows automatically regain focus.");

  GUI::get_instance()->get_tooltips()->set_tip(*force_focus_cb, force_focus_tip);
  
  // Monitoring
  monitor_model = Gtk::ListStore::create(monitor_columns);
  monitor_combo.set_model(monitor_model);

  Gtk::TreeModel::Row row = *(monitor_model->append());
  row[monitor_columns.col_id] = "normal";
  row[monitor_columns.col_name] = _("Normal monitor");

  row = *(monitor_model->append());
  row[monitor_columns.col_id] = "lowlevel";
  row[monitor_columns.col_name] = _("Alternate monitor using low-level global hook");

  if (1) // FIXME: LOBYTE( LOWORD( GetVersion() ) ) >= 5)
    {
      row = *(monitor_model->append());
      row[monitor_columns.col_id] = "nohook";
      row[monitor_columns.col_name] = _("Alternate monitor without global hooks");
    }
  
  //  monitor_combo.pack_start(monitor_columns.col_id);
  monitor_combo.pack_start(monitor_columns.col_name);

  string info_text =
    _("Some applications are incompatible with the default keyboard/mouse monitoring.\n"
      "Workrave offers two experimental alternate monitoring methods.\n"
      "\n"
      "The first alternate method uses low-level hooks exclusively. This method allows\n"
      "monitoring of administrative applications on Vista but may cause slight mouse or\n"
      "keyboard lag.\n"
      "\n"
      "The second one does not use any hook at all. This method also allows\n"
      "monitoring of administrative applications on Vista but mouse &amp; keyboard\n"
      "statistics are unavailable when this monitor is selected.\n"
      "\n"
      "Vista users: By default, Workrave does not monitor your interaction \n"
      "with administrative applications. This is due to security restrictions in Vista.\n"
      "If you would like Workrave to monitor your interaction with these applications, \n"
      "please enable the second alternate activity monitor.");

  GUI::get_instance()->get_tooltips()->set_tip(monitor_combo, info_text);

  // Notebook
  HigCategoryPanel *panel = manage(new HigCategoryPanel(
                                     _("Experimental Options (restart required for changes to take effect)")));
  panel->set_border_width(12);
  panel->add(*force_focus_cb);
  panel->add("Monitoring method:", monitor_combo);

  notebook->append_page(*panel, _("Troubleshooting"));
  notebook->show_all();
  notebook->set_current_page(0);

  pack_start(*notebook, true, true, 0);

  init();

  force_focus_cb->signal_toggled().connect(MEMBER_SLOT(*this,
                                                       &AdvancedPreferencePage::on_force_focus_changed));
  monitor_combo.signal_changed().connect(MEMBER_SLOT(*this, &AdvancedPreferencePage::on_monitor_changed));


  show_all();

  TRACE_EXIT();
#endif
}

AdvancedPreferencePage::~AdvancedPreferencePage()
{
  TRACE_ENTER( "AdvancedPreferencePage::~AdvancedPreferencePage" );
  TRACE_EXIT();
}

#ifdef WIN32
void
AdvancedPreferencePage::on_monitor_changed()
{
  Gtk::TreeModel::iterator iter = monitor_combo.get_active();
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
}


void
AdvancedPreferencePage::on_force_focus_changed()
{
  force_focus_cb->set_sensitive(false);
  CoreFactory::get_configurator()->
      set_value_on_quit("advanced/force_focus", force_focus_cb->get_active());
  force_focus_cb->set_sensitive(true);
}


void
AdvancedPreferencePage::init()
{
  // Set monitor methods
  string monitor;
  if (!CoreFactory::get_configurator()->get_value_on_quit("advanced/monitor", &monitor))
    {
      CoreFactory::get_configurator()->get_value_default("advanced/monitor", &monitor, "normal");
    }

  typedef Gtk::TreeStore::Children type_children;
  type_children children = monitor_model->children();
  for (type_children::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
      Gtk::TreeModel::Row row = *iter;

      if (row[monitor_columns.col_id] == monitor)
        {
          monitor_combo.set_active(iter);
        };
    }

  bool force_focus = false;
  if (!CoreFactory::get_configurator()->get_value_on_quit("advanced/force_focus", &force_focus))
    {
      CoreFactory::get_configurator()->get_value_default("advanced/force_focus", &force_focus, false);
    }
  
  force_focus_cb->set_active(force_focus);
}
#endif
