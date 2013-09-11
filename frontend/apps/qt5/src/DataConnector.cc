// DataConnector.cc --- Connect Gtkmm widget with Configuration items
//
// Copyright (C) 2007, 2008, 2011, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#include "DataConnector.hh"

#include "nls.h"
#include "TimeEntry.hh"

#include "config/IConfigurator.hh"
#include "CoreFactory.hh"

using namespace workrave;
using namespace workrave::config;
using namespace std;

// Define connector for standard qt widgets.
DEFINE_DATA_TYPE_PTR(QCheckBox, DataConnectionQCheckBox);
DEFINE_DATA_TYPE_PTR(QSpinBox,  DataConnectionQSpinBox);
DEFINE_DATA_TYPE_PTR(QComboBox, DataConnectionQComboBox);

DEFINE_DATA_TYPE_PTR(TimeEntry, DataConnectionTimeEntry);

// namespace dc
// {
//   DataConnectionGtkEntryTwin *wrap(Gtk::Entry *w1, Gtk::Entry *w2)
//   {
//     return new DataConnectionGtkEntryTwin(w1, w2);
//   }
// }


//! Construct a new data connector.
DataConnector::DataConnector()
{
  config = CoreFactory::get_configurator();
}


//! Destruct data connector.
DataConnector::~DataConnector()
{
  for (auto &connection : connections)
    {
      delete connection.connection;
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
                       boost::function<bool(const string &, bool)> cb,
                       dc::Flags flags)
{
  if (connection != NULL)
    {
      MonitoredWidget mw;

      mw.connection = connection;

      connection->set(flags, setting);
      connection->intercept = cb;
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
 * Spin Button                                                         *
 *                                                                     *
 ***********************************************************************/

//! Initialize connection.
void
DataConnectionQSpinBox::init()
{
  void (QSpinBox:: *signal)(int) = &QSpinBox::valueChanged;
  QObject::connect(widget, signal, this, &DataConnectionQSpinBox::widget_changed_notify);
  config_changed_notify(key);
}

//! Widget changed value.
void
DataConnectionQSpinBox::widget_changed_notify()
{
  bool skip = false;

  if (intercept)
    {
      skip = intercept(key, true);
    }

  if (!skip)
    {
      int value = widget->value();
      config->set_value(key, value);
    }
}

//! Configuration item changed value.
void
DataConnectionQSpinBox::config_changed_notify(const string &key)
{
  int value;

  bool skip = false;
  if (intercept)
    {
      skip = intercept(key, false);
    }

  if (!skip)
    {
      if (config->get_value(key, value))
        {
          widget->setValue(value);
        }
    }
}


/***********************************************************************
 *                                                                     *
 * Check Button                                                        *
 *                                                                     *
 ***********************************************************************/

//! Initialize connection.
void
DataConnectionQCheckBox::init()
{
  QObject::connect(widget, &QCheckBox::stateChanged, this, &DataConnectionQCheckBox::widget_changed_notify);
  config_changed_notify(key);
}


//! Configuration item changed value.
void
DataConnectionQCheckBox::widget_changed_notify()
{
  bool skip = false;

  if (intercept)
    {
      skip = intercept(key, true);
    }

  if (!skip)
    {
      bool value = widget->checkState() == Qt::Checked;
      config->set_value(key, value);
    }
}


//! Configuration item changed value.
void
DataConnectionQCheckBox::config_changed_notify(const string &key)
{
  bool skip = false;
  if (intercept)
    {
      skip = intercept(key, false);
    }

  if (!skip)
    {
      bool value;
      if (config->get_value(key, value))
        {
          widget->setCheckState(value ? Qt::Checked : Qt::Unchecked);
        }
    }
}


// /***********************************************************************
//  *                                                                     *
//  * ComboBox Button                                                         *
//  *                                                                     *
//  ***********************************************************************/

//! Initialize connection.
void
DataConnectionQComboBox::init()
{
  void (QComboBox:: *signal)(int) = &QComboBox::currentIndexChanged;
  QObject::connect(widget, signal, this, &DataConnectionQComboBox::widget_changed_notify);
  config_changed_notify(key);
}


//! Configuration item changed value.
void
DataConnectionQComboBox::widget_changed_notify()
{
  bool skip = false;

  if (intercept)
    {
      skip = intercept(key, true);
    }

  if (!skip)
    {
      int value = widget->currentIndex();
      config->set_value(key, value);
    }
}

//! Configuration item changed value.
void
DataConnectionQComboBox::config_changed_notify(const string &key)
{
  bool skip = false;
  if (intercept)
    {
      skip = intercept(key, false);
    }

  if (!skip)
    {
      int value;
      if (config->get_value(key, value))
        {
          widget->setCurrentIndex(value);
        }
    }
}

// /***********************************************************************
//  *                                                                     *
//  * Spin Button                                                         *
//  *                                                                     *
//  ***********************************************************************/

// //! Initialize connection.
// void
// DataConnectionGtkAdjustment::init()
// {
//   widget->signal_value_changed()
//     .connect(sigc::mem_fun(*this, &DataConnectionGtkAdjustment::widget_changed_notify));
//   config_changed_notify(key);
// }

// //! Widget changed value.
// void
// DataConnectionGtkAdjustment::widget_changed_notify()
// {
//   bool skip = false;

//   if (intercept)
//     {
//       skip = intercept(key, true);
//     }

//   if (!skip)
//     {
//       int value = (int)widget->get_value();

//       config->set_value(key, value);
//     }
// }

// //! Configuration item changed value.
// void
// DataConnectionGtkAdjustment::config_changed_notify(const string &key)
// {
//   bool skip = false;
//   if (intercept)
//     {
//       skip = intercept(key, false);
//     }

//   if (!skip)
//     {
//       int value;
//       if (config->get_value(key, value))
//         {
//           widget->set_value(value);
//         }
//     }
// }

// /***********************************************************************
//  *                                                                     *
//  * Spin Button                                                         *
//  *                                                                     *
//  ***********************************************************************/


//! Initialize connection.
void
DataConnectionTimeEntry::init()
{
  widget->signal_value_changed()
    .connect(boost::bind(&DataConnectionTimeEntry::widget_changed_notify, this));
  config_changed_notify(key);
}

//! Widget changed value.
void
DataConnectionTimeEntry::widget_changed_notify()
{
  bool skip = false;

  if (intercept)
    {
      skip = intercept(key, true);
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
  if (intercept)
    {
      skip = intercept(key, false);
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

// /***********************************************************************
//  *                                                                     *
//  * Spin Button                                                         *
//  *                                                                     *
//  ***********************************************************************/

// //! Initialize connection.
// void
// DataConnectionGtkEntryTwin::init()
// {
//   widget1->signal_changed()
//     .connect(sigc::mem_fun(*this, &DataConnectionGtkEntryTwin::widget_changed_notify));
//   widget2->signal_changed()
//     .connect(sigc::mem_fun(*this, &DataConnectionGtkEntryTwin::widget_changed_notify));
//   config_changed_notify(key);
// }


// //! Widget changed value.
// void
// DataConnectionGtkEntryTwin::widget_changed_notify()
// {
//   bool skip = false;

//   if (intercept)
//     {
//       skip = intercept(key, true);
//     }

//   if (!skip)
//     {
//       string value1 = widget1->get_text();
//       string value2 = widget2->get_text();
//       bool verified = true;

// #ifdef HAVE_GTK3
//       if (value1 == value2)
//         {
//           widget1->unset_background_color();
//           widget2->unset_background_color();
//         }
//       else
//         {
//           widget1->override_background_color(Gdk::RGBA("orange"));
//           widget2->override_background_color(Gdk::RGBA("orange"));
//           verified = false;
//         }
// #else
//       if (value1 == value2)
//         {
//           widget1->unset_base(Gtk::STATE_NORMAL);
//           widget2->unset_base(Gtk::STATE_NORMAL);
//         }
//       else
//         {
//           widget1->modify_base(Gtk::STATE_NORMAL, Gdk::Color("orange"));
//           widget2->modify_base(Gtk::STATE_NORMAL, Gdk::Color("orange"));
//           verified = false;
//         }
// #endif

//       if (verified)
//         {
//           config->set_value(key, value1);
//         }
//     }
// }

// //! Configuration item changed value.
// void
// DataConnectionGtkEntryTwin::config_changed_notify(const string &key)
// {
//   bool skip = false;
//   if (intercept)
//     {
//       skip = intercept(key, false);
//     }

//   if (!skip)
//     {
//       string value;
//       if (config->get_value(key, value))
//         {
//           widget1->set_text(value);
//           widget2->set_text(value);
//         }
//     }
// }


