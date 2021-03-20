// GConfConfigurator.hh
//
// Copyright (C) 2001, 2002, 2003, 2006, 2007 Rob Caelers <robc@krandor.nl>
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

#ifndef GCONFCONFIGURATOR_HH
#define GCONFCONFIGURATOR_HH

#include <string>
#include <map>

#include "glib.h"

#ifndef GCONF_HACK
class GConfClient;
class GConfEntry;
class GConfValue;
#endif

#include "IConfigBackend.hh"

class GConfConfigurator
  : public IConfigBackend
  , public IConfigBackendMonitoring
{
public:
  GConfConfigurator();
  virtual ~GConfConfigurator();

  virtual bool load(std::string filename);
  virtual bool save(std::string filename);
  virtual bool save();

  virtual bool remove_key(const std::string &key);
  virtual bool get_value(const std::string &key, VariantType type, Variant &value) const;
  virtual bool set_value(const std::string &key, Variant &value);

  virtual void set_listener(IConfiguratorListener *listener);
  virtual bool add_listener(const std::string &key_prefix);
  virtual bool remove_listener(const std::string &key_prefix);

private:
  bool get_value(const std::string &key, GConfValue **value) const;

  typedef std::map<guint, std::string> IDMap;
  typedef IDMap::iterator IDMapIter;

  //! id -> key maps
  IDMap ids;

  //! GConf thingy
  GConfClient *gconf_client;

  //! notify connection ID
  guint connection_id;

  //! GConf Root of workrave
  std::string gconf_root;

  //! Send changes to.
  IConfiguratorListener *listener;

  //! Callback.
  static void static_key_changed(GConfClient *client, guint cnxn_id, GConfEntry *entry, gpointer user_data);

  //!
  void key_changed(guint cnxn_id, GConfEntry *entry);
};

#endif // GCONFCONFIGURATOR_HH
