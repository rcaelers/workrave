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
  ~QtSettingsConfigurator() override;

  bool load(std::string filename) override;
  bool save(std::string filename) override;
  bool save() override;

  bool remove_key(const std::string &key) override;

  bool get_config_value(const std::string &key, std::string &out) const override;
  bool get_config_value(const std::string &key, bool &out) const override;
  bool get_config_value(const std::string &key, int &out) const override;
  bool get_config_value(const std::string &key, long &out) const override;
  bool get_config_value(const std::string &key, double &out) const override;

  bool set_config_value(const std::string &key, std::string v) override;
  bool set_config_value(const std::string &key, int v) override;
  bool set_config_value(const std::string &key, long v) override;
  bool set_config_value(const std::string &key, bool v) override;
  bool set_config_value(const std::string &key, double v) override;

protected:
  QVariant qt_get_value(const std::string &key, bool& exists) const;
  QString qt_key(const std::string &key) const;
  void dispose();

  QSettings *settings;
};

#endif // QTSETTIGNSCONFIGURATOR_HH
