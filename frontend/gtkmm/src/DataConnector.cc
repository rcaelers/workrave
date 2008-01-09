// DataConnector.cc --- Connect Gtkmm widget with Configuration items
//
// Copyright (C) 2007 Rob Caelers <robc@krandor.nl>
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

#include <gtkmm/entry.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/adjustment.h>
#include "TimeEntry.hh"

#include "DataConnector.hh"

#include "IBreak.hh"
#include "CoreFactory.hh"
#include "IConfigurator.hh"

using namespace workrave;
using namespace std;

DEFINE_DATA_TYPE(Gtk::Entry, DataConnectionGtkEntry);
DEFINE_DATA_TYPE(Gtk::CheckButton, DataConnectionGtkCheckButton);
DEFINE_DATA_TYPE(Gtk::SpinButton, DataConnectionGtkSpinButton);
DEFINE_DATA_TYPE(Gtk::OptionMenu, DataConnectionGtkOptionMenu);
DEFINE_DATA_TYPE(Gtk::Adjustment, DataConnectionGtkAdjustment);
DEFINE_DATA_TYPE(TimeEntry, DataConnectionTimeEntry);

namespace dc
{
  DataConnectionGtkEntryTwin *wrap(Gtk::Entry *w1, Gtk::Entry *w2)
  {
    return new DataConnectionGtkEntryTwin(w1, w2);
  }
}

//!
DataConnector::DataConnector()
{
  config = CoreFactory::get_configurator();
  last_connection = NULL;
}

//!
DataConnector::~DataConnector()
{
  for (WidgetIter i = connections.begin(); i != connections.end(); i++)
    {
      delete i->connection;
    }
}


//!
void
DataConnector::connect(BreakId id, const string &setting,
                       DataConnection *connection, dc::Flags flags)
{
  string str = setting;
  string::size_type pos = 0;

  ICore *core = CoreFactory::get_core();
  IBreak *b = core->get_break(id);
  string name = b->get_name();

  while ((pos = str.find("%b", pos)) != string::npos)
    {
      str.replace(pos, 2, name);
      pos++;
    }

  connect(str, connection, flags);
}

//!
void
DataConnector::connect(const string &setting, DataConnection *connection,
                       dc::Flags flags)
{
  if (connection == NULL)
    {
      last_connection = NULL;
      return;
    }

  MonitoredWidget mw;

  mw.connection = connection;

  connection->set(flags, setting);
  connection->init();

  connections.push_back(mw);

  last_connection = connection;

}


void
DataConnector::intercept_last(sigc::slot<bool, const string &, bool> slot)
{
  if (last_connection != NULL)
    {
      last_connection->intercept.connect(slot);
    }
}

DataConnection::DataConnection()
{
  config = CoreFactory::get_configurator();
}


DataConnection::~DataConnection()
{
  config->remove_listener(key, this);
}


void
DataConnection::set(dc::Flags flags, const string &key)
{
  this->flags = flags;
  this->key = key;

  if ((flags & dc::NO_CONFIG) == 0)
    {
      config->add_listener(key, this);
    }
}


void
DataConnectionGtkEntry::init()
{
  widget->signal_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkEntry::widget_changed_notify));
  config_changed_notify(key);
}


void
DataConnectionGtkEntry::widget_changed_notify()
{
  string value = widget->get_text();

  config->set_value(key, value);
}

void
DataConnectionGtkEntry::config_changed_notify(const string &key)
{
  string value;
  bool ok = config->get_value(key, value);
  if (ok)
    {
      widget->set_text(value);
    }
}


void
DataConnectionGtkSpinButton::widget_changed_notify()
{
  int value = widget->get_value_as_int();

  config->set_value(key, value);
}


void
DataConnectionGtkSpinButton::init()
{
  widget->signal_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkSpinButton::widget_changed_notify));
  widget->signal_value_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkSpinButton::widget_changed_notify));
  config_changed_notify(key);
}

void
DataConnectionGtkSpinButton::config_changed_notify(const string &key)
{
  int value;
  bool ok = config->get_value(key, value);
  if (ok)
    {
      widget->set_value(value);
    }
}

void
DataConnectionGtkCheckButton::init()
{
  widget->signal_toggled()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkCheckButton::widget_changed_notify));
  config_changed_notify(key);
}


void
DataConnectionGtkCheckButton::widget_changed_notify()
{
  bool skip = false;

  if (!intercept.empty())
    {
      skip = intercept.emit(key, true);
    }

  if (!skip)
    {
      bool value = widget->get_active();
      config->set_value(key, value);
    }
}


void
DataConnectionGtkCheckButton::config_changed_notify(const string &key)
{
  bool skip = false;
  if (!intercept.empty())
    {
      skip = intercept.emit(key, false);
    }

  if (!skip)
    {
      bool value;
      bool ok = config->get_value(key, value);
      if (ok)
        {
          widget->set_active(value);
        }
    }
}


void
DataConnectionGtkOptionMenu::init()
{
  widget->signal_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkOptionMenu::widget_changed_notify));
  config_changed_notify(key);
}


void
DataConnectionGtkOptionMenu::widget_changed_notify()
{
  int value = widget->get_history();

  config->set_value(key, value);
}

void
DataConnectionGtkOptionMenu::config_changed_notify(const string &key)
{
  int value;
  bool ok = config->get_value(key, value);
  if (ok)
    {
      widget->set_history(value);
    }
}


void
DataConnectionGtkAdjustment::init()
{
  widget->signal_value_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkAdjustment::widget_changed_notify));
  config_changed_notify(key);
}

void
DataConnectionGtkAdjustment::widget_changed_notify()
{
  int value = (int)widget->get_value();

  config->set_value(key, value);
}

void
DataConnectionGtkAdjustment::config_changed_notify(const string &key)
{
  int value;
  bool ok = config->get_value(key, value);
  if (ok)
    {
      widget->set_value(value);
    }
}



void
DataConnectionTimeEntry::init()
{
  widget->signal_value_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionTimeEntry::widget_changed_notify));
  config_changed_notify(key);
}

void
DataConnectionTimeEntry::widget_changed_notify()
{
  int value = widget->get_value();

  config->set_value(key, value);
}

void
DataConnectionTimeEntry::config_changed_notify(const string &key)
{
  int value;
  bool ok = config->get_value(key, value);
  if (ok)
    {
      widget->set_value(value);
    }
}


void
DataConnectionGtkEntryTwin::init()
{
  widget1->signal_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkEntryTwin::widget_changed_notify));
  widget2->signal_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkEntryTwin::widget_changed_notify));
  config_changed_notify(key);
}


void
DataConnectionGtkEntryTwin::config_changed_notify(const string &key)
{
  string value;
  bool ok = config->get_value(key, value);
  if (ok)
    {
      widget1->set_text(value);
      widget2->set_text(value);
    }
}


void
DataConnectionGtkEntryTwin::widget_changed_notify()
{
  string value1 = widget1->get_text();
  string value2 = widget2->get_text();
  bool verified = true;

  if (value1 == value2)
    {
      widget1->unset_base(Gtk::STATE_NORMAL);
      widget2->unset_base(Gtk::STATE_NORMAL);
    }
  else
    {
      widget1->modify_base(Gtk::STATE_NORMAL, Gdk::Color("orange"));
      widget2->modify_base(Gtk::STATE_NORMAL, Gdk::Color("orange"));
      verified = false;
    }

  if (verified)
    {
      config->set_value(key, value1);
    }
}

