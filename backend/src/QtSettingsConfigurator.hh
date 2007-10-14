// QtSettingsConfigurator.hh
//
// Copyright (C) 2006, 2007 Raymond Penners <raymond@dotsphinx.com>
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Id: IniConfigurator.hh 558 2006-02-23 19:42:12Z rcaelers $
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

  virtual bool save();

  virtual bool remove_key(const std::string &key);

  virtual bool get_value(const string &key, string &out) const;
  virtual bool get_value(const string &key, bool &out) const;
  virtual bool get_value(const string &key, int &out) const;
  virtual bool get_value(const string &key, long &out) const;
  virtual bool get_value(const string &key, double &out) const;

  virtual bool set_value(const string &key, string v);
  virtual bool set_value(const string &key, int v);
  virtual bool set_value(const string &key, long v);
  virtual bool set_value(const string &key, bool v);
  virtual bool set_value(const string &key, double v);

protected:
  QVariant qt_get(const string key, bool& exists) const;
  QString qt_key(const string key) const;
  void dispose();

  QSettings *settings;
};

#endif // QTSETTIGNSCONFIGURATOR_HH
