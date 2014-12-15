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
DEFINE_DATA_TYPE_PTR(QAbstractSlider, DataConnectionQAbstractSlider);
DEFINE_DATA_TYPE_PTR(TimeEntry, DataConnectionTimeEntry);

DataConnector::Ptr
DataConnector::create()
{
  return Ptr(new DataConnector);
}

DataConnector::DataConnector()
{
  config = CoreFactory::get_configurator();
}

DataConnector::~DataConnector()
{
  for (auto &connection : connections)
    {
      delete connection.connection;
    }
}

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


/***********************************************************************
 *                                                                     *
 * Spin Button                                                         *
 *                                                                     *
 ***********************************************************************/

void
DataConnectionQSpinBox::init()
{
  void (QSpinBox:: *signal)(int) = &QSpinBox::valueChanged;
  QObject::connect(widget, signal, this, &DataConnectionQSpinBox::widget_changed_notify);
  config_changed_notify(key);
}

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

void
DataConnectionQCheckBox::init()
{
  QObject::connect(widget, &QCheckBox::stateChanged, this, &DataConnectionQCheckBox::widget_changed_notify);
  config_changed_notify(key);
}

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

/***********************************************************************
 *                                                                     *
 * ComboBox Button                                                         *
 *                                                                     *
 ***********************************************************************/

void
DataConnectionQComboBox::init()
{
  void (QComboBox:: *signal)(int) = &QComboBox::currentIndexChanged;
  QObject::connect(widget, signal, this, &DataConnectionQComboBox::widget_changed_notify);
  config_changed_notify(key);
}


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

/***********************************************************************
 *                                                                     *
 * Slider                                                              *
 *                                                                     *
 ***********************************************************************/

void
DataConnectionQAbstractSlider::init()
{
  void (QAbstractSlider:: *signal)(int) = &QAbstractSlider::valueChanged;
  QObject::connect(widget, signal, this, &DataConnectionQAbstractSlider::widget_changed_notify);
  config_changed_notify(key);
}


void
DataConnectionQAbstractSlider::widget_changed_notify()
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

void
DataConnectionQAbstractSlider::config_changed_notify(const string &key)
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
          widget->setValue(value);
        }
    }
}

/***********************************************************************
 *                                                                     *
 * TimeEntry                                                           *
 *                                                                     *
 ***********************************************************************/

void
DataConnectionTimeEntry::init()
{
  widget->signal_value_changed()
    .connect(boost::bind(&DataConnectionTimeEntry::widget_changed_notify, this));
  config_changed_notify(key);
}

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
