// Configurator.hh
//
// Copyright (C) 2001, 2002, 2003, 2006, 2007, 2008 Rob Caelers <robc@krandor.nl>
// Copyright (C) 2007 Ray Satiro <raysatiro@yahoo.com>
//
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

#ifndef CONFIGURATOR_HH
#define CONFIGURATOR_HH

#include <string>
#include <list>
#include <map>

#include "Mutex.hh"
#include "IConfigurator.hh"
#include "IConfiguratorListener.hh"
#include "IConfigBackend.hh"

using namespace workrave;
using namespace std;

// Forward declarion of external interface.
namespace workrave {
  class IConfiguratorListener;
}
#include "Variant.hh"

class IConfigBackend;

class Configurator : public IConfigurator, public IConfiguratorListener
{
public:
  Configurator(IConfigBackend *backend);
  virtual ~Configurator();

  void heartbeat();

  // IConfigurator
  virtual void set_delay(const std::string &name, int delay);

  virtual bool load(std::string filename);
  virtual bool save(std::string filename);
  virtual bool save();

  virtual bool remove_key(const std::string &key) const;
  virtual bool rename_key(const std::string &key, const std::string &new_key);

  virtual bool get_value(const std::string &key, std::string &out) const;
  virtual bool get_value(const std::string &key, bool &out) const;
  virtual bool get_value(const std::string &key, int &out) const;
  virtual bool get_value(const std::string &key, double &out) const;

  virtual void get_value_with_default(const std::string & key, std::string &out, string s) const;
  virtual void get_value_with_default(const std::string & key, bool &out, const bool def) const;
  virtual void get_value_with_default(const std::string & key, int &out, const int def) const;
  virtual void get_value_with_default(const std::string & key, double &out, const double def) const;

  virtual bool set_value(const std::string &key, const std::string &v, ConfigFlags flags = CONFIG_FLAG_NONE);
  virtual bool set_value(const std::string &key, const char *v, ConfigFlags flags = CONFIG_FLAG_NONE);
  virtual bool set_value(const std::string &key, int v, ConfigFlags flags = CONFIG_FLAG_NONE);
  virtual bool set_value(const std::string &key, bool v, ConfigFlags flags = CONFIG_FLAG_NONE);
  virtual bool set_value(const std::string &key, double v, ConfigFlags flags = CONFIG_FLAG_NONE);

  virtual bool get_typed_value(const std::string &key, std::string &t) const;
  virtual bool set_typed_value(const std::string &key, const std::string &t);

  virtual bool add_listener(const std::string &key_prefix, IConfiguratorListener *listener);
  virtual bool remove_listener(IConfiguratorListener *listener);
  virtual bool remove_listener(const std::string &key_prefix, IConfiguratorListener *listener);
  virtual bool find_listener(IConfiguratorListener *listener, std::string &key) const;

private:
  typedef std::list<std::pair<std::string, IConfiguratorListener *> > Listeners;
  typedef std::list<std::pair<std::string, IConfiguratorListener *> >::iterator ListenerIter;
  typedef std::list<std::pair<std::string, IConfiguratorListener *> >::const_iterator ListenerCIter;

  //! Configuration change listeners.
  Listeners listeners;

private:
  struct DelayedConfig
  {
    std::string key;
    Variant value;
    time_t until;
  };

  struct Setting
  {
    std::string key;
    int delay;
  };

  typedef std::map<std::string, DelayedConfig> DelayedList;
  typedef DelayedList::iterator DelayedListIter;
  typedef DelayedList::const_iterator DelayedListCIter;

  typedef std::map<std::string, Setting> Settings;
  typedef std::map<std::string, Setting>::iterator SettingIter;
  typedef std::map<std::string, Setting>::const_iterator SettingCIter;


private:
  bool find_setting(const string &name, Setting &setting) const;
  
  bool set_value(const std::string &key, Variant &value, ConfigFlags flags = CONFIG_FLAG_NONE);
  bool get_value(const std::string &key, VariantType type, Variant &value) const;

  void fire_configurator_event(const std::string &key);
  void strip_leading_slash(std::string &key) const;
  void strip_trailing_slash(std::string &key) const;
  void add_trailing_slash(std::string &key) const;

  void config_changed_notify(const std::string &key);
  
private:
  //! Registered settings.
  Settings settings;

  //! Delayed settings
  DelayedList delayed_config;

  //! The backend in use.
  IConfigBackend *backend;

  //! Next auto save time.
  time_t auto_save_time;
};


#endif // CONFIGURATOR_HH
