// DataConnector.cc --- Connect Gtkmm widget with Configuration items
//
// Copyright (C) 2007, 2008, 2011 Rob Caelers <robc@krandor.nl>
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

#include <gtkmm.h>
#include "TimeEntry.hh"

#include "DataConnector.hh"

#include "CoreFactory.hh"
#include "IConfigurator.hh"

using namespace workrave;
using namespace std;

// Define connector for standard gtkmm widgets.
DEFINE_DATA_TYPE(Gtk::Entry *, DataConnectionGtkEntry);
DEFINE_DATA_TYPE(Gtk::CheckButton *, DataConnectionGtkCheckButton);
DEFINE_DATA_TYPE(Gtk::SpinButton *, DataConnectionGtkSpinButton);
DEFINE_DATA_TYPE(Gtk::ComboBox *, DataConnectionGtkComboBox);

#ifdef HAVE_GTK3
DEFINE_DATA_TYPE(Glib::RefPtr<Gtk::Adjustment>, DataConnectionGtkAdjustment);
#else
DEFINE_DATA_TYPE(Gtk::Adjustment *, DataConnectionGtkAdjustment);
#endif

DEFINE_DATA_TYPE(TimeEntry *, DataConnectionTimeEntry);

namespace dc
{
  DataConnectionGtkEntryTwin *wrap(Gtk::Entry *w1, Gtk::Entry *w2)
  {
    return new DataConnectionGtkEntryTwin(w1, w2);
  }
}


//! Construct a new data connector.
DataConnector::DataConnector()
{
  config = CoreFactory::get_configurator();
}


//! Destruct data connector.
DataConnector::~DataConnector()
{
  for (WidgetIter i = connections.begin(); i != connections.end(); i++)
    {
      delete i->connection;
    }
}


//! Connect a widget to a configuration item.
void
DataConnector::connect(const string &setting,
                       DataConnection *connection,
                       dc::Flags flags)
{
  if (connection != NULL)
    {
      MonitoredWidget mw;

      mw.connection = connection;

      connection->set(flags, setting);
      connection->init();

      connections.push_back(mw);
    }
}


//! Connect a widget to a configuration item.
void
DataConnector::connect(const string &setting,
                       DataConnection *connection,
                       sigc::slot<bool, const string &, bool> slot,
                       dc::Flags flags)
{
  if (connection != NULL)
    {
      MonitoredWidget mw;

      mw.connection = connection;

      connection->set(flags, setting);
      connection->intercept.connect(slot);
      connection->init();

      connections.push_back(mw);
    }
}


//! Construct a new data connection
DataConnection::DataConnection()
{
  config = CoreFactory::get_configurator();
}


//! Destruct data connection.
DataConnection::~DataConnection()
{
  config->remove_listener(key, this);
}


//! Set connection flags and configuration key.
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


/***********************************************************************
 *                                                                     *
 * Text Entry                                                          *
 *                                                                     *
 ***********************************************************************/


//! Initialize connection.
void
DataConnectionGtkEntry::init()
{
  widget->signal_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkEntry::widget_changed_notify));
  config_changed_notify(key);
}

//! Widget changed value.
void
DataConnectionGtkEntry::widget_changed_notify()
{
  bool skip = false;

  if (!intercept.empty())
    {
      skip = intercept.emit(key, true);
    }

  if (!skip)
    {
      string value = widget->get_text();
      config->set_value(key, value);
    }
}

//! Configuration item changed value.
void
DataConnectionGtkEntry::config_changed_notify(const string &key)
{
  bool skip = false;
  if (!intercept.empty())
    {
      skip = intercept.emit(key, false);
    }

  if (!skip)
    {
      string value;
      if (config->get_value(key, value))
        {
          widget->set_text(value);
        }
    }
}


/***********************************************************************
 *                                                                     *
 * Spin Button                                                         *
 *                                                                     *
 ***********************************************************************/

//! Initialize connection.
void
DataConnectionGtkSpinButton::init()
{
  widget->signal_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkSpinButton::widget_changed_notify));
  widget->signal_value_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkSpinButton::widget_changed_notify));
  config_changed_notify(key);
}

//! Widget changed value.
void
DataConnectionGtkSpinButton::widget_changed_notify()
{
  bool skip = false;

  if (!intercept.empty())
    {
      skip = intercept.emit(key, true);
    }

  if (!skip)
    {
      int value = widget->get_value_as_int();
      config->set_value(key, value);
    }
}

//! Configuration item changed value.
void
DataConnectionGtkSpinButton::config_changed_notify(const string &key)
{
  int value;

  bool skip = false;
  if (!intercept.empty())
    {
      skip = intercept.emit(key, false);
    }

  if (!skip)
    {
      if (config->get_value(key, value))
        {
          widget->set_value(value);
        }
    }
}


/***********************************************************************
 *                                                                     *
 * Spin Button                                                         *
 *                                                                     *
 ***********************************************************************/

//! Initialize connection.
void
DataConnectionGtkCheckButton::init()
{
  widget->signal_toggled()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkCheckButton::widget_changed_notify));
  config_changed_notify(key);
}


//! Configuration item changed value.
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


//! Configuration item changed value.
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
      if (config->get_value(key, value))
        {
          widget->set_active(value);
        }
    }
}


/***********************************************************************
 *                                                                     *
 * Spin Button                                                         *
 *                                                                     *
 ***********************************************************************/

//! Initialize connection.
void
DataConnectionGtkComboBox::init()
{
  widget->signal_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkComboBox::widget_changed_notify));
  config_changed_notify(key);
}


//! Configuration item changed value.
void
DataConnectionGtkComboBox::widget_changed_notify()
{
  bool skip = false;

  if (!intercept.empty())
    {
      skip = intercept.emit(key, true);
    }

  if (!skip)
    {
      int value = widget->get_active_row_number();

      config->set_value(key, value);
    }
}

//! Configuration item changed value.
void
DataConnectionGtkComboBox::config_changed_notify(const string &key)
{
  bool skip = false;
  if (!intercept.empty())
    {
      skip = intercept.emit(key, false);
    }

  if (!skip)
    {
      int value;
      if (config->get_value(key, value))
        {
          widget->set_active(value);
        }
    }
}

/***********************************************************************
 *                                                                     *
 * Spin Button                                                         *
 *                                                                     *
 ***********************************************************************/

//! Initialize connection.
void
DataConnectionGtkAdjustment::init()
{
  widget->signal_value_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkAdjustment::widget_changed_notify));
  config_changed_notify(key);
}

//! Widget changed value.
void
DataConnectionGtkAdjustment::widget_changed_notify()
{
  bool skip = false;

  if (!intercept.empty())
    {
      skip = intercept.emit(key, true);
    }

  if (!skip)
    {
      int value = (int)widget->get_value();
      
      config->set_value(key, value);
    }
}

//! Configuration item changed value.
void
DataConnectionGtkAdjustment::config_changed_notify(const string &key)
{
  bool skip = false;
  if (!intercept.empty())
    {
      skip = intercept.emit(key, false);
    }

  if (!skip)
    {
      int value;
      if (config->get_value(key, value))
        {
          widget->set_value(value);
        }
    }
}

/***********************************************************************
 *                                                                     *
 * Spin Button                                                         *
 *                                                                     *
 ***********************************************************************/


//! Initialize connection.
void
DataConnectionTimeEntry::init()
{
  widget->signal_value_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionTimeEntry::widget_changed_notify));
  config_changed_notify(key);
}

//! Widget changed value.
void
DataConnectionTimeEntry::widget_changed_notify()
{
  bool skip = false;

  if (!intercept.empty())
    {
      skip = intercept.emit(key, true);
    }

  if (!skip)
    {
      time_t value = widget->get_value();

      config->set_value(key, (int)value);
    }
}

//! Configuration item changed value.
void
DataConnectionTimeEntry::config_changed_notify(const string &key)
{
  bool skip = false;
  if (!intercept.empty())
    {
      skip = intercept.emit(key, false);
    }

  if (!skip)
    {
      int value;
      if (config->get_value(key, value))
        {
          widget->set_value(value);
        }
    }
}

/***********************************************************************
 *                                                                     *
 * Spin Button                                                         *
 *                                                                     *
 ***********************************************************************/

//! Initialize connection.
void
DataConnectionGtkEntryTwin::init()
{
  widget1->signal_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkEntryTwin::widget_changed_notify));
  widget2->signal_changed()
    .connect(sigc::mem_fun(*this, &DataConnectionGtkEntryTwin::widget_changed_notify));
  config_changed_notify(key);
}


//! Widget changed value.
void
DataConnectionGtkEntryTwin::widget_changed_notify()
{
  bool skip = false;

  if (!intercept.empty())
    {
      skip = intercept.emit(key, true);
    }

  if (!skip)
    {
      string value1 = widget1->get_text();
      string value2 = widget2->get_text();
      bool verified = true;

#ifdef HAVE_GTK3      
      if (value1 == value2)
        {
          widget1->unset_background_color();
          widget2->unset_background_color();
        }
      else
        {
          widget1->override_background_color(Gdk::RGBA("orange"));
          widget2->override_background_color(Gdk::RGBA("orange"));
          verified = false;
        }
#else
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
#endif
      
      if (verified)
        {
          config->set_value(key, value1);
        }
    }
}

//! Configuration item changed value.
void
DataConnectionGtkEntryTwin::config_changed_notify(const string &key)
{
  bool skip = false;
  if (!intercept.empty())
    {
      skip = intercept.emit(key, false);
    }

  if (!skip)
    {
      string value;
      if (config->get_value(key, value))
        {
          widget1->set_text(value);
          widget2->set_text(value);
        }
    }
}


