// Copyright (C) 2007, 2008, 2011, 2012 Rob Caelers <robc@krandor.nl>
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
#include <sigc++/sigc++.h>
#include <gdkmm/types.h>

#include <gtkmm.h>

#include "config/Setting.hh"
#include "ui/IApplicationContext.hh"

class TimeEntry;

namespace Gtk
{
  class Object;
  class Widget;
  class Entry;
  class SpinButton;
  class ComboBox;
  class Adjustment;
} // namespace Gtk

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
  explicit DataConnector(std::shared_ptr<IApplicationContext> app);
  ~DataConnector();

  void connect(const std::string &setting, DataConnection *connection, dc::Flags flags = dc::NONE);

  void connect(const std::string &setting,
               DataConnection *connection,
               sigc::slot<bool, const std::string &, bool> slot,
               dc::Flags flags = dc::NONE);

  template<class T, class R = T>
  void connect(workrave::config::Setting<T, R> &setting, DataConnection *connection, dc::Flags flags = dc::NONE)
  {
    connect(setting.key(), connection, flags);
  }

  template<class T, class R = T>
  void connect(workrave::config::Setting<T, R> &setting,
               DataConnection *connection,
               sigc::slot<bool, const std::string &, bool> slot,
               dc::Flags flags = dc::NONE)
  {
    connect(setting.key(), connection, slot, flags);
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

  //!
  Widgets connections;

  //!
  workrave::config::IConfigurator::Ptr config;
};

class DataConnection : public workrave::config::IConfiguratorListener
{
public:
  explicit DataConnection();
  ~DataConnection() override;

  void set(workrave::config::IConfigurator::Ptr config, dc::Flags flags, const std::string &key);
  virtual void init() = 0;

  sigc::signal<bool(const std::string &, bool)> intercept;

protected:
  workrave::config::IConfigurator::Ptr config;
  std::string key;
  dc::Flags flags;
};

#define DECLARE_DATA_TYPE(WidgetType, WrapperType, WidgetDataType /*, ConfigDataType */) \
  class WrapperType : public DataConnection                                              \
  {                                                                                      \
  public:                                                                                \
    WrapperType(WidgetType widget)                                                       \
      : widget(widget)                                                                   \
    {                                                                                    \
    }                                                                                    \
    virtual ~WrapperType()                                                               \
    {                                                                                    \
    }                                                                                    \
                                                                                         \
    void init();                                                                         \
    void widget_changed_notify();                                                        \
    void config_changed_notify(const std::string &key);                                  \
                                                                                         \
  private:                                                                               \
    WidgetType widget;                                                                   \
  };                                                                                     \
                                                                                         \
  namespace dc                                                                           \
  {                                                                                      \
    WrapperType *wrap(WidgetType t);                                                     \
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

DECLARE_DATA_TYPE(Gtk::Entry *, DataConnectionGtkEntry, std::string);
DECLARE_DATA_TYPE(Gtk::CheckButton *, DataConnectionGtkCheckButton, bool);
DECLARE_DATA_TYPE(Gtk::SpinButton *, DataConnectionGtkSpinButton, int);
DECLARE_DATA_TYPE(Gtk::ComboBox *, DataConnectionGtkComboBox, int);

DECLARE_DATA_TYPE(Glib::RefPtr<Gtk::Adjustment>, DataConnectionGtkAdjustment, int);

DECLARE_DATA_TYPE(TimeEntry *, DataConnectionTimeEntry, int);

class DataConnectionGtkEntryTwin : public DataConnection
{
public:
  DataConnectionGtkEntryTwin(Gtk::Entry *widget1, Gtk::Entry *widget2)
    : widget1(widget1)
    , widget2(widget2)
  {
  }
  ~DataConnectionGtkEntryTwin() override = default;

  void init() override;
  void widget_changed_notify();
  void config_changed_notify(const std::string &key) override;

private:
  Gtk::Entry *widget1;
  Gtk::Entry *widget2;
};

namespace dc
{
  DataConnectionGtkEntryTwin *wrap(Gtk::Entry *w1, Gtk::Entry *w2);
}

#endif
