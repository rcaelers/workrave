// Copyright (C) 2006, 2007 Raymond Penners <raymond@dotsphinx.com>
// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#include "QtSettingsConfigurator.hh"

using namespace std;

QtSettingsConfigurator::QtSettingsConfigurator(QSettings *s)
{
  settings = s;
}

QtSettingsConfigurator::~QtSettingsConfigurator()
{
  dispose();
}

void
QtSettingsConfigurator::dispose()
{
  if (settings)
    {
      delete settings;
      settings = nullptr;
    }
}

bool
QtSettingsConfigurator::load(string filename)
{
  (void)filename;
  return true;
}

bool
QtSettingsConfigurator::save(string filename)
{
  (void)filename;
  return true;
}

bool
QtSettingsConfigurator::save()
{
  if (settings)
    {
      settings->sync();
      return true;
    }
  return false;
}

QString
QtSettingsConfigurator::qt_key(const string &key) const
{
  QString qkey(key.c_str());
  return qkey.prepend('/');
}

QVariant
QtSettingsConfigurator::qt_get_value(const string &key, bool &exists) const
{
  exists = false;
  QVariant var;
  if (settings)
    {
      const QString qkey = qt_key(key);
      if (settings->contains(qkey))
        {
          var = settings->value(qkey);
          exists = true;
        }
    }
  return var;
}

bool
QtSettingsConfigurator::has_user_value(const std::string &key)
{
  bool exists;
  QVariant var = qt_get_value(key, exists);
  return exists;
}

bool
QtSettingsConfigurator::remove_key(const std::string &key)
{
  (void)key;
  return true;
}

bool
QtSettingsConfigurator::get_config_value(const string &key, string &out) const
{
  bool exists;
  QVariant var = qt_get_value(key, exists);
  if (exists)
    {
      QString qout = var.toString();
      out = qout.toStdString();
    }
  return exists;
}

bool
QtSettingsConfigurator::get_config_value(const string &key, bool &out) const
{
  bool exists;
  QVariant var = qt_get_value(key, exists);
  if (exists)
    {
      out = var.toBool();
    }
  return exists;
}

bool
QtSettingsConfigurator::get_config_value(const string &key, int &out) const
{
  bool exists;
  QVariant var = qt_get_value(key, exists);
  if (exists)
    {
      out = var.toInt();
    }
  return exists;
}

bool
QtSettingsConfigurator::get_config_value(const string &key, long &out) const
{
  bool exists;
  QVariant var = qt_get_value(key, exists);
  if (exists)
    {
      out = static_cast<long>(var.toInt()); // Why doesn't Qt have toLong?
    }
  return exists;
}

bool
QtSettingsConfigurator::get_config_value(const string &key, double &out) const
{
  bool exists;
  QVariant var = qt_get_value(key, exists);
  if (exists)
    {
      out = var.toDouble();
    }
  return exists;
}

bool
QtSettingsConfigurator::set_config_value(const string &key, string v)
{
  bool ok = false;
  if (settings)
    {
      const QString qkey = qt_key(key);
      QVariant qval = v.c_str();
      settings->setValue(qkey, qval);
      ok = true;
    }
  return ok;
}

bool
QtSettingsConfigurator::set_config_value(const string &key, int v)
{
  bool ok = false;
  if (settings)
    {
      const QString qkey = qt_key(key);
      QVariant qval = v;
      settings->setValue(qkey, qval);
      ok = true;
    }
  return ok;
}

bool
QtSettingsConfigurator::set_config_value(const string &key, long v)
{
  bool ok = false;
  if (settings)
    {
      const QString qkey = qt_key(key);
      QVariant qval = static_cast<int>(v); // QT no long?
      settings->setValue(qkey, qval);
      ok = true;
    }
  return ok;
}

bool
QtSettingsConfigurator::set_config_value(const string &key, bool v)
{
  bool ok = false;
  if (settings)
    {
      const QString qkey = qt_key(key);
      QVariant qval = v;
      settings->setValue(qkey, qval);
      ok = true;
    }
  return ok;
}

bool
QtSettingsConfigurator::set_config_value(const string &key, double v)
{
  bool ok = false;
  if (settings)
    {
      const QString qkey = qt_key(key);
      QVariant qval = v;
      settings->setValue(qkey, qval);
      ok = true;
    }
  return ok;
}
