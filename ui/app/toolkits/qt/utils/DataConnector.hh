// DataConnector.hh --- Connect widget with the configurator
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

#ifndef DATACONNECTOR_HH
#define DATACONNECTOR_HH

#include <string>
#include <list>

#include "config/Config.hh"
#include "core/ICore.hh"
#include "config/Setting.hh"
#include "ui/IApplicationContext.hh"

#include <QtGui>
#include <QtWidgets>

class TimeEntry;

class DataConnection;

namespace dc
{
  enum Flags
  {
    NONE = 0,
    NO_CONFIG = 1,
    NO_WIDGET = 2,
  };
} // namespace dc

class DataConnector
{
public:
  using Ptr = std::shared_ptr<DataConnector>;

  explicit DataConnector(std::shared_ptr<IApplicationContext> app);
  ~DataConnector();

  void connect(const std::string &setting, DataConnection *connection, dc::Flags flags = dc::NONE);

  void connect(const std::string &setting,
               DataConnection *connection,
               std::function<bool(const std::string &, bool)> cb,
               dc::Flags flags = dc::NONE);

  template<class T, class R = T>
  void connect(workrave::config::Setting<T, R> &setting, DataConnection *connection, dc::Flags flags = dc::NONE)
  {
    connect(setting.key(), connection, flags);
  }

  template<class T, class R = T>
  void connect(workrave::config::Setting<T, R> &setting,
               DataConnection *connection,
               std::function<bool(const std::string &, bool)> cb,
               dc::Flags flags = dc::NONE)
  {
    connect(setting.key(), connection, cb, flags);
  }

private:
  struct MonitoredWidget
  {
    MonitoredWidget()
    {
      connection = nullptr;
    }

    DataConnection *connection;
  };

  using Widgets = std::list<MonitoredWidget>;
  using WidgetIter = Widgets::iterator;

  Widgets connections;
  workrave::config::IConfigurator::Ptr config;
};

class DataConnection
  : public QObject
  , public workrave::config::IConfiguratorListener
{
public:
  DataConnection();
  ~DataConnection() override;

  void set(workrave::config::IConfigurator::Ptr config, dc::Flags flags, const std::string &key);
  virtual void init() = 0;

  std::function<bool(const std::string &, bool)> intercept;

protected:
  workrave::config::IConfigurator::Ptr config;
  std::string key;
  dc::Flags flags;
};

#define DECLARE_DATA_TYPE(WidgetType, WrapperType)      \
  class WrapperType : public DataConnection             \
  {                                                     \
  public:                                               \
    explicit WrapperType(WidgetType widget)             \
      : widget(widget)                                  \
    {                                                   \
    }                                                   \
    virtual ~WrapperType()                              \
    {                                                   \
    }                                                   \
                                                        \
    void init();                                        \
    void widget_changed_notify();                       \
    void config_changed_notify(const std::string &key); \
                                                        \
  private:                                              \
    WidgetType widget;                                  \
  };                                                    \
                                                        \
  namespace dc                                          \
  {                                                     \
    WrapperType *wrap(WidgetType t);                    \
  }

#define DEFINE_DATA_TYPE(WidgetType, WrapperType) \
  namespace dc                                    \
  {                                               \
    WrapperType *wrap(WidgetType t)               \
    {                                             \
      return new WrapperType(t);                  \
    }                                             \
  }

#define DEFINE_DATA_TYPE_PTR(WidgetType, WrapperType) \
  namespace dc                                        \
  {                                                   \
    WrapperType *wrap(WidgetType *t)                  \
    {                                                 \
      if (t != NULL)                                  \
        {                                             \
          return new WrapperType(t);                  \
        }                                             \
      else                                            \
        {                                             \
          return NULL;                                \
        }                                             \
    }                                                 \
  }

DECLARE_DATA_TYPE(QCheckBox *, DataConnectionQCheckBox);
DECLARE_DATA_TYPE(QSpinBox *, DataConnectionQSpinBox);
DECLARE_DATA_TYPE(QComboBox *, DataConnectionQComboBox);
DECLARE_DATA_TYPE(QAbstractSlider *, DataConnectionQAbstractSlider);
DECLARE_DATA_TYPE(TimeEntry *, DataConnectionTimeEntry);

#endif
