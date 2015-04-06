// QtSettingsConfigurator.hh
//
// Copyright (C) 2006, 2007, 2013 Raymond Penners <raymond@dotsphinx.com>
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

#ifndef QTSETTINGSCONFIGURATOR_HH
#define QTSETTINGSCONFIGURATOR_HH

#include <QSettings>
#include "IConfigBackend.hh"
#include "ConfigBackendAdapter.hh"

class QtSettingsConfigurator : public virtual IConfigBackend, public virtual ConfigBackendAdapter
{
public:
  QtSettingsConfigurator(QSettings *settings = 0);
  virtual ~QtSettingsConfigurator();

  virtual bool load(std::string filename);
  virtual bool save(std::string filename);
  virtual bool save();

  virtual bool remove_key(const std::string &key);

  virtual bool get_config_value(const std::string &key, std::string &out) const;
  virtual bool get_config_value(const std::string &key, bool &out) const;
  virtual bool get_config_value(const std::string &key, int &out) const;
  virtual bool get_config_value(const std::string &key, long &out) const;
  virtual bool get_config_value(const std::string &key, double &out) const;

  virtual bool set_config_value(const std::string &key, std::string v);
  virtual bool set_config_value(const std::string &key, int v);
  virtual bool set_config_value(const std::string &key, long v);
  virtual bool set_config_value(const std::string &key, bool v);
  virtual bool set_config_value(const std::string &key, double v);

protected:
  QVariant qt_get_value(const std::string &key, bool& exists) const;
  QString qt_key(const std::string &key) const;
  void dispose();

  QSettings *settings;
};

#endif // QTSETTIGNSCONFIGURATOR_HH
