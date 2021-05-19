// Copyright (C) 2013 Rob Caelers
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#define BOOST_TEST_MODULE workrave_config
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test;
#include <boost/mpl/list.hpp>

#include "SimulatedTime.hh"

#include "Configurator.hh"
#include "config/SettingCache.hh"

#include "IniConfigurator.hh"
#include "XmlConfigurator.hh"
#ifdef HAVE_GSETTINGS
#  include "GSettingsConfigurator.hh"
#endif
#ifdef PLATFORM_OS_WINDOWS
#  include "W32Configurator.hh"
#endif
#ifdef PLATFORM_OS_MACOS
#  include "MacOSConfigurator.hh"
#endif
#ifdef HAVE_QT5
#  include "QtSettingsConfigurator.hh"
#endif

using namespace std;
using namespace workrave;
using namespace workrave::config;
using namespace workrave::utils;

class Fixture : public IConfiguratorListener
{
  public:
    Fixture()
    {
      sim = SimulatedTime::create();
      SettingCache::reset();
    }

    ~Fixture()
    {
      configurator->remove_listener(this);

#ifdef HAVE_GSETTINGS
	    g_unsetenv ("GSETTINGS_BACKEND");
	    g_unsetenv ("GSETTINGS_SCHEMA_DIR");
#endif
    };

    template<typename T>
    void init()
    {
      sim->reset();
      TimeSource::sync();
      configurator = std::make_shared<Configurator>(new T());
    }

    template<>
    void init<GSettingsConfigurator>()
    {
      sim->reset();
      TimeSource::sync();

      g_setenv("GSETTINGS_SCHEMA_DIR", BUILDDIR, true);
      g_setenv("GSETTINGS_BACKEND", "memory", 1);

      configurator = std::make_shared<Configurator>(new GSettingsConfigurator());
      has_defaults = true;
    }

    void tick()
    {
      TimeSource::sync();
      configurator->heartbeat();
      sim->current_time += 1000000;
    }

    void tick(int seconds, const std::function<void(int count)> &check_func)
    {
      for (int i = 0; i < seconds; i++)
      {
        try
        {

          TimeSource::sync();
          configurator->heartbeat();
          BOOST_TEST_CONTEXT("Config")
          {
            BOOST_TEST_INFO_SCOPE("Count:" << i);
            check_func(i);
          }
          sim->current_time += 1000000;
        }
        catch (...)
        {
          BOOST_TEST_MESSAGE(string("error at:") + boost::lexical_cast<string>(i));
          std::cout << "error at:" << i << std::endl;
          throw;
        }
      }
    }

    void config_changed_notify(const std::string &key) override
    {
      BOOST_CHECK_EQUAL(key, expected_key);
      config_changed_count++;
    }

    enum class Mode
    {
        Mode1, Mode2, Mode3
    };

    SettingGroup &
    group()
    {
      return SettingCache::group(configurator, "test/settings");
    }

    Setting<int> &
    setting_int()
    {
      return SettingCache::get<int>(configurator, "test/settings/int");
    }

    Setting<double> &
    setting_double()
    {
      return SettingCache::get<double>(configurator, "test/settings/double");
    }

    Setting<std::string> &
    setting_string()
    {
      return SettingCache::get<std::string>(configurator, "test/settings/string");
    }

    Setting<bool> &
    setting_bool()
    {
      return SettingCache::get<bool>(configurator, "test/settings/bool");
    }

    Setting<int, Mode> &
    setting_modedefault()
    {
      return SettingCache::get<int, Mode>(configurator, "test/settings/mode");
    }

    Setting<int, Mode> &
    setting_mode()
    {
      return SettingCache::get<int, Mode>(configurator, "test/settings/mode");
    }

    Setting<int> &
    setting_int_default()
    {
      return SettingCache::get<int>(configurator, "test/settings/default/int", 8888);
    }

    Setting<double> &
    setting_double_default()
    {
      return SettingCache::get<double>(configurator, "test/settings/default/double", 88.88);
    }

    Setting<std::string> &
    setting_string_default()
    {
      return SettingCache::get<std::string>(configurator, "test/settings/default/string", std::string("8888"));
    }

    Setting<bool> &
    setting_bool_default()
    {
      return SettingCache::get<bool>(configurator, "test/settings/default/bool", true);
    }

    SimulatedTime::Ptr sim;
    Configurator::Ptr configurator;
    bool has_defaults { false };
    std::string expected_key;
    int config_changed_count { 0 };
};

inline std::ostream &operator<<(std::ostream &stream, Fixture::Mode mode)
{
  switch (mode)
  {
    case Fixture::Mode::Mode1:
      stream << "mode1";
      break;
    case Fixture::Mode::Mode2:
      stream << "mode2";
      break;
    case Fixture::Mode::Mode3:
      stream << "mode3";
      break;
  }
  return stream;
}


BOOST_FIXTURE_TEST_SUITE(config, Fixture)

typedef boost::mpl::list<IniConfigurator, XmlConfigurator
#ifdef HAVE_GSETTINGS
                         , GSettingsConfigurator
#endif
                         > backend_types;

typedef boost::mpl::list<
#ifdef HAVE_GSETTINGS
                         GSettingsConfigurator
#endif
                         > non_file_backend_types;

typedef boost::mpl::list<XmlConfigurator, IniConfigurator> file_backend_types;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_string, T, backend_types)
{
  init<T>();

  bool ok { false };

  std::string value;
  ok = configurator->get_value("test/schema-defaults/string", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK(!has_defaults || value == "default_string");
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/string"), false);

  ok = configurator->set_value("test/schema-defaults/string", std::string{"string_value"});
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/string"), true);

  ok = configurator->get_value("test/schema-defaults/string", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "string_value");

  ok = configurator->set_value("test/schema-defaults/string", std::string{"other_string_value"});
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("test/schema-defaults/string", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "other_string_value");
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_charstring, T, backend_types)
{
  init<T>();

  bool ok { false };

  std::string value;
  ok = configurator->get_value("test/schema-defaults/charstring", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK(!has_defaults || value == "default_charstring");
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/charstring"), false);

  ok = configurator->set_value("test/schema-defaults/charstring", "charstring_value");
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/charstring"), true);

  ok = configurator->get_value("test/schema-defaults/charstring", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "charstring_value");

  ok = configurator->set_value("test/schema-defaults/charstring", "other_charstring_value");
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("test/schema-defaults/charstring", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "other_charstring_value");
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_int, T, backend_types)
{
  init<T>();

  bool ok { false };

  int value;
  ok = configurator->get_value("test/schema-defaults/int", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK(!has_defaults || value == 1234);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/int"), false);

  ok = configurator->set_value("test/schema-defaults/int", 11);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/int"), true);

  ok = configurator->get_value("test/schema-defaults/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 11);

  ok = configurator->set_value("test/schema-defaults/int", 22);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("test/schema-defaults/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 22);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_double, T, backend_types)
{
  init<T>();

  bool ok { false };

  double value;
  ok = configurator->get_value("test/schema-defaults/double", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK(!has_defaults || value == 12.34);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/double"), false);

  ok = configurator->set_value("test/schema-defaults/double", 11.11);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/double"), true);

  ok = configurator->get_value("test/schema-defaults/double", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 11.11);

  ok = configurator->set_value("test/schema-defaults/double", 22.22);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("test/schema-defaults/double", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 22.22);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_bool, T, backend_types)
{
  init<T>();

  bool ok { false };

  bool value;
  ok = configurator->get_value("test/schema-defaults/bool", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK(!has_defaults || value);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/bool"), false);

  ok = configurator->set_value("test/schema-defaults/bool", true);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/bool"), true);

  ok = configurator->get_value("test/schema-defaults/bool", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, true);

  ok = configurator->set_value("test/schema-defaults/bool", false);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("test/schema-defaults/bool", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_string_default, T, backend_types)
{
  init<T>();
  std::string value;
  configurator->get_value_with_default("test/code-defaults/string", value, "11");
  BOOST_CHECK_EQUAL(value, has_defaults ? "default_string": "11");

  bool ok = configurator->set_value("test/code-defaults/string", std::string{"22"});
  BOOST_CHECK_EQUAL(ok, true);

  configurator->get_value_with_default("test/code-defaults/string", value, "11");
  BOOST_CHECK_EQUAL(value, "22");
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_int_default, T, backend_types)
{
  init<T>();

  int value;
  configurator->get_value_with_default("test/code-defaults/int", value, 33);
  BOOST_CHECK_EQUAL(value, has_defaults ? 1234 : 33);

  bool ok = configurator->set_value("test/code-defaults/int", 44);
  BOOST_CHECK_EQUAL(ok, true);

  configurator->get_value_with_default("test/code-defaults/int", value, 33);
  BOOST_CHECK_EQUAL(value, 44);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_double_default, T, backend_types)
{
  init<T>();

  double value;
  configurator->get_value_with_default("test/code-defaults/double", value, 33.33);
  BOOST_CHECK_EQUAL(value, has_defaults ? 12.34 : 33.33);

  bool ok = configurator->set_value("test/code-defaults/double", 44.44);
  BOOST_CHECK_EQUAL(ok, true);

  configurator->get_value_with_default("test/code-defaults/double", value, 33.33);
  BOOST_CHECK_EQUAL(value, 44.44);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_bool_default, T, backend_types)
{
  init<T>();

  bool value;
  configurator->get_value_with_default("test/code-defaults/bool", value, true);
  BOOST_CHECK_EQUAL(value, true);

  configurator->get_value_with_default("test/code-defaults/bool", value, false);
  BOOST_CHECK_EQUAL(value, has_defaults ? true : false);

  bool ok = configurator->set_value("test/code-defaults/bool", true);
  BOOST_CHECK_EQUAL(ok, true);

  configurator->get_value_with_default("test/code-defaults/bool", value, false);
  BOOST_CHECK_EQUAL(value, true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_string_wrong_type, T, backend_types)
{
  init<T>();

  bool ok { false };

  int ivalue;
  ok = configurator->get_value("test/other/string", ivalue);
  BOOST_CHECK_EQUAL(ok, false);

  double dvalue;
  ok = configurator->get_value("test/other/string", dvalue);
  BOOST_CHECK_EQUAL(ok, false);

  bool bvalue;
  ok = configurator->get_value("test/other/string", bvalue);
  BOOST_CHECK_EQUAL(ok, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_int_wrong_type, T, backend_types)
{
  init<T>();

  bool ok { false };

  std::string svalue;
  ok = configurator->get_value("test/other/int", svalue);
  BOOST_CHECK_EQUAL(ok, false);

  double dvalue;
  ok = configurator->get_value("test/other/int", dvalue);
  BOOST_CHECK_EQUAL(ok, false);

  bool bvalue;
  ok = configurator->get_value("test/other/int", bvalue);
  BOOST_CHECK_EQUAL(ok, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_double_wrong_type, T, backend_types)
{
  init<T>();

  bool ok { false };

  std::string svalue;
  ok = configurator->get_value("test/other/double", svalue);
  BOOST_CHECK_EQUAL(ok, false);

  int ivalue;
  ok = configurator->get_value("test/other/double", ivalue);
  BOOST_CHECK_EQUAL(ok, false);

  bool bvalue;
  ok = configurator->get_value("test/other/double", bvalue);
  BOOST_CHECK_EQUAL(ok, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_bool_wrong_type, T, backend_types)
{
  init<T>();

  bool ok { false };

  std::string svalue;
  ok = configurator->get_value("test/other/bool", svalue);
  BOOST_CHECK_EQUAL(ok, false);

  int ivalue;
  ok = configurator->get_value("test/other/bool", ivalue);
  BOOST_CHECK_EQUAL(ok, false);

  double dvalue;
  ok = configurator->get_value("test/other/bool", dvalue);
  BOOST_CHECK_EQUAL(ok, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_bad_key, T, backend_types)
{
  init<T>();

  bool ok { false };

  std::string value;
  ok = configurator->get_value("", value);
  //BOOST_CHECK_EQUAL(ok, false); TODO: check for IniConfigurator

  ok = configurator->get_value("/", value);
  //BOOST_CHECK_EQUAL(ok, false); TODO: check for IniConfigurator

  ok = configurator->get_value(" ", value);
  BOOST_CHECK_EQUAL(ok, false);

  ok = configurator->get_value("lskjflskd", value);
  BOOST_CHECK_EQUAL(ok, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_listener_one, T, backend_types)
{
  init<T>();

  bool ok { false };

  ok = configurator->add_listener("test/other/int", this);
  BOOST_CHECK_EQUAL(ok, true);

  expected_key = "test/other/int";
  ok = configurator->set_value("test/other/int", 1001);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 1);

  ok = configurator->remove_listener("test", this);
  BOOST_CHECK_EQUAL(ok, false);

  ok = configurator->set_value("test/other/int", 1002);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 2);

  ok = configurator->set_value("test/other/int", 1002);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 2);

  ok = configurator->set_value("test/other/double", 1002.1002);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 2);

  ok = configurator->remove_listener("test/other/int", this);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->set_value("test/other/int", 1003);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 2);

  ok = configurator->add_listener("test/other/int/", this);

  std::string key;
  configurator->find_listener(this, key);
  BOOST_CHECK_EQUAL(key, "test/other/int");

  ok = configurator->set_value("test/other/int", 1001);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 3);

  ok = configurator->remove_listener("test/other/int", this);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->set_value("test/other/int", 1004);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 3);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_listener_section, T, backend_types)
{
  init<T>();

  bool ok { false };

  ok = configurator->add_listener("test/other/", this);
  BOOST_CHECK_EQUAL(ok, true);

  expected_key = "test/other/int";
  ok = configurator->set_value("test/other/int", 1005);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 1);

  expected_key = "test/other/double";
  ok = configurator->set_value("test/other/double", 1005.55);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 2);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_listener_multiple, T, backend_types)
{
  init<T>();

  bool ok { false };

  ok = configurator->add_listener("test/other/int", this);
  ok = configurator->add_listener("test/other/double/", this);

  expected_key = "test/other/int";
  ok = configurator->set_value("test/other/int", 1006);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 1);

  expected_key = "test/other/double";
  ok = configurator->set_value("test/other/double", 1006.1006);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 2);

  ok = configurator->remove_listener("test/other/double", this);
  ok = configurator->remove_listener("test/other/int", this);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_listener_add_remove, T, backend_types)
{
  init<T>();

  bool ok { false };

  ok = configurator->add_listener("test/other/int", this);
  BOOST_CHECK_EQUAL(ok, true);
  ok = configurator->add_listener("test/other/double/", this);
  BOOST_CHECK_EQUAL(ok, true);
  ok = configurator->add_listener("test/other/double/", (IConfiguratorListener *)0xdeadbeef);
  BOOST_CHECK_EQUAL(ok, true);
  ok = configurator->add_listener("test/other/double/", (IConfiguratorListener *)0xdeadbeef);
  BOOST_CHECK_EQUAL(ok, false);
  ok = configurator->add_listener("test/other/string", (IConfiguratorListener *)0xbaaaaaad);
  BOOST_CHECK_EQUAL(ok, true);

  std::string key;
  ok = configurator->find_listener(this, key);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(key, "test/other/int");
  ok = configurator->find_listener((IConfiguratorListener *)0xdeadbeef, key);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(key, "test/other/double");
  ok = configurator->find_listener((IConfiguratorListener *)0xbaadf00d, key);
  BOOST_CHECK_EQUAL(ok, false);

  ok = configurator->remove_listener((IConfiguratorListener *)0xbaaaaaad);
  BOOST_CHECK_EQUAL(ok, true);
  ok = configurator->remove_listener((IConfiguratorListener *)0xbaadf00d);
  BOOST_CHECK_EQUAL(ok, false);

  ok = configurator->remove_listener("test/other/double", this);
  BOOST_CHECK_EQUAL(ok, true);
  ok = configurator->remove_listener("test/other/double", (IConfiguratorListener *)0xdeadbeef);
  BOOST_CHECK_EQUAL(ok, true);
  ok = configurator->remove_listener("test/other/double", (IConfiguratorListener *)0xdeadbeef);
  BOOST_CHECK_EQUAL(ok, false);
  ok = configurator->remove_listener("test/other/int", (IConfiguratorListener *)0xdeadbeef);
  BOOST_CHECK_EQUAL(ok, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_leading_slash, T, backend_types)
{
  init<T>();

  bool ok { false };

  int value;
  ok = configurator->set_value("/test/other/int", 1007);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1007);

  ok = configurator->set_value("/test/other/int", 1008);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1008);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_trailing_slash, T, backend_types)
{
  init<T>();

  bool ok { false };

  int value;
  ok = configurator->set_value("test/other/int/", 1009);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("test/other/int/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1009);

  ok = configurator->set_value("test/other/int/", 1010);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("test/other/int/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1010);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_leading_trailing_slash, T, backend_types)
{
  init<T>();

  bool ok { false };

  int value;
  ok = configurator->set_value("/test/other/int/", 1011);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/int/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1011);

  ok = configurator->set_value("/test/other/int/", 1012);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/int/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1012);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay, T, backend_types)
{
  init<T>();

  bool ok { false };

  int value;
  ok = configurator->set_value("test/other/int", 1013);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("test/other/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1013);

  configurator->set_delay("test/other/int", 5);

  ok = configurator->add_listener("test/other/int", this);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->set_value("/test/other/int/", 1014);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 0);

  expected_key = "test/other/int";
  ok = configurator->get_value("test/other/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1014);

  tick(6, [](int c) {});
  BOOST_CHECK_EQUAL(config_changed_count, 1);

  ok = configurator->remove_listener(this);
  BOOST_CHECK_EQUAL(ok, true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay_repeat, T, backend_types)
{
  init<T>();

  bool ok { false };

  int value;
  ok = configurator->set_value("test/other/int", 1015);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("test/other/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1015);

  configurator->set_delay("test/other/int", 5);
  configurator->set_delay("test/other/int", 10);

  ok = configurator->add_listener("test/other/int", this);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->set_value("/test/other/int/", 1016);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 0);

  expected_key = "test/other/int";
  ok = configurator->get_value("test/other/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1016);

  tick(9, [](int c) {});
  BOOST_CHECK_EQUAL(config_changed_count, 0);
  tick(2, [](int c) {});
  BOOST_CHECK_EQUAL(config_changed_count, 1);

  ok = configurator->remove_listener(this);
  BOOST_CHECK_EQUAL(ok, true);
}


BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay_save_load, T, file_backend_types)
{
  init<T>();

  bool ok { false };

  int value;
  ok = configurator->set_value("test/other/int", 1017);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("test/other/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1017);

  configurator->set_delay("test/other/int", 5);

  ok = configurator->add_listener("test/other/int", this);
  BOOST_CHECK_EQUAL(ok, true);

  configurator->save("temp-save");

  ok = configurator->set_value("/test/other/int/", 1018);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(config_changed_count, 0);

  expected_key = "test/other/int";
  ok = configurator->get_value("test/other/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1018);

  tick(6, [](int c) {});
  BOOST_CHECK_EQUAL(config_changed_count, 1);

  configurator->load("temp-save");

  ok = configurator->get_value("test/other/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1017);

  ok = configurator->remove_listener(this);
  BOOST_CHECK_EQUAL(ok, true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay_immediate, T, backend_types)
{
  init<T>();

  bool ok { false };

  int value;

  configurator->set_delay("test/other/int", 5);

  ok = configurator->set_value("test/other/int", 1019, CONFIG_FLAG_IMMEDIATE);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/int/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1019);

  ok = configurator->set_value("test/other/double", 1019.1019, CONFIG_FLAG_IMMEDIATE);
  BOOST_CHECK_EQUAL(ok, true);

  double dv;
  ok = configurator->get_value("/test/other/double/", dv);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(dv, 1019.1019);
}

// BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay_invalid, T, backend_types)
// {
//   init<T>();

//   bool ok { false };

//   int value;

//   configurator->set_delay("test/other/invalid", 5);

//   ok = configurator->set_value("test/other/invalid", 89);
//   BOOST_CHECK_EQUAL(ok, true);

//   ok = configurator->set_value("test/other/invalid", 88.1, CONFIG_FLAG_IMMEDIATE);
//   BOOST_CHECK_EQUAL(ok, true);
// }


BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay_same_value, T, backend_types)
{
  init<T>();

  bool ok { false };

  int value;
  ok = configurator->set_value("test/other/int", 1020);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("test/other/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1020);

  configurator->set_delay("test/other/int", 5);

  ok = configurator->set_value("/test/other/int/", 1021);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->set_value("/test/other/int/", 1020);
  BOOST_CHECK_EQUAL(ok, true);

  tick(6, [](int c) {});

  ok = configurator->get_value("test/other/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1020);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay_initial_value, T, backend_types)
{
  init<T>();

  bool ok { false };

  int value;
  ok = configurator->get_value("test/other/delay-initial", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  configurator->set_delay("test/other/delay-initial", 5);

  ok = configurator->set_value("/test/other/delay-initial/", 1022);
  BOOST_CHECK_EQUAL(ok, true);

  tick(6, [](int c) {});

  ok = configurator->get_value("test/other/delay-initial", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1022);
}


BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_remove, T, backend_types)
{
  init<T>();

  bool ok { false };

  int value;
  ok = configurator->get_value("/test/other/int/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->set_value("/test/other/int/", 1023);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/int/", value);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->remove_key("/test/other/int/");
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/int/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  if (has_defaults)
  {
    BOOST_CHECK_EQUAL(value, 1234);
  }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_rename_int, T, backend_types)
{
  init<T>();

  bool ok { false };

  int value;
  ok = configurator->get_value("/test/other/int/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->get_value("/test/other/int2/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->set_value("/test/other/int/", 1024);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/int/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1024);

  ok = configurator->rename_key("/test/other/int/", "/test/other/int2/");
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/int2/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1024);

  ok = configurator->get_value("/test/other/int/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_rename_bool, T, backend_types)
{
  init<T>();

  bool ok { false };

  bool value;
  ok = configurator->get_value("/test/other/bool/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->get_value("/test/other/bool2/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->set_value("/test/other/bool/", true);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/bool/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, true);

  ok = configurator->rename_key("/test/other/bool/", "/test/other/bool2/");
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/bool2/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, true);

  ok = configurator->get_value("/test/other/bool/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_rename_double, T, backend_types)
{
  init<T>();

  bool ok { false };

  double value;
  ok = configurator->get_value("/test/other/double/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->get_value("/test/other/double2/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->set_value("/test/other/double/", 1025.1025);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/double/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1025.1025);

  ok = configurator->rename_key("/test/other/double/", "/test/other/double2/");
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/double2/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1025.1025);

  ok = configurator->get_value("/test/other/double/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_rename_string, T, backend_types)
{
  init<T>();

  bool ok { false };

  std::string value;
  ok = configurator->get_value("/test/other/string/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->get_value("/test/other/string2/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->set_value("/test/other/string/", "27");
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/string/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "27");

  ok = configurator->rename_key("/test/other/string/", "/test/other/string2/");
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/string2/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "27");

  ok = configurator->get_value("/test/other/string/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_rename_exists, T, backend_types)
{
  init<T>();

  bool ok { false };

  int value;
  ok = configurator->get_value("/test/other/int/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->get_value("/test/other/int2/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->set_value("/test/other/int/", 1026);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->set_value("/test/other/int2/", 1027);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->rename_key("/test/other/int/", "/test/other/int2/");
  BOOST_CHECK_EQUAL(ok, false);

  ok = configurator->get_value("/test/other/int2/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1027);

  ok = configurator->get_value("/test/other/int/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1026);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_initial, T, backend_types)
{
  init<T>();

  bool ok { false };

  int value;
  ok = configurator->get_value("/test/other/initial", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->set_value("/test/other/initial", 1028, CONFIG_FLAG_INITIAL);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/initial", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, has_defaults ? 1234 : 1028);

  ok = configurator->set_value("/test/other/initial", 1029, CONFIG_FLAG_INITIAL);
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("/test/other/initial", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, has_defaults ? 1234: 1028);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_save_load, T, file_backend_types)
{
  init<T>();

  configurator->set_value("/test/other/string", "1030");
  configurator->set_value("/test/other/int", 1030);
  configurator->set_value("/test/other/double", 1030.1030);
  configurator->set_value("/test/other/bool", true);

  configurator->save("temp-save");

  configurator->set_value("/test/other/string", "1031");
  configurator->set_value("/test/other/int", 1031);
  configurator->set_value("/test/other/double", 1031.1031);
  configurator->set_value("/test/other/bool", false);

  configurator->load("temp-save");

  std::string svalue;
  bool ok = configurator->get_value("test/other/string", svalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(svalue, "1030");

  int ivalue;
  ok = configurator->get_value("test/other/int", ivalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(ivalue, 1030);

  double dvalue;
  ok = configurator->get_value("test/other/double", dvalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(dvalue, 1030.1030);

  bool bvalue;
  ok = configurator->get_value("test/other/bool", bvalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(bvalue, true);

  configurator->set_value("/test/other/string", "1031");
  configurator->set_value("/test/other/int", 1031);
  configurator->set_value("/test/other/double", 1031.1031);
  configurator->set_value("/test/other/bool", false);

  configurator->save();

  configurator->set_value("/test/other/string", "1032");
  configurator->set_value("/test/other/int", 1032);
  configurator->set_value("/test/other/double", 1032.1032);
  configurator->set_value("/test/other/bool", true);

  configurator->load("temp-save");

  ok = configurator->get_value("test/other/string", svalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(svalue, "1031");

  ok = configurator->get_value("test/other/int", ivalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(ivalue, 1031);

  ok = configurator->get_value("test/other/double", dvalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(dvalue, 1031.1031);

  ok = configurator->get_value("test/other/bool", bvalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(bvalue, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_dummy_save_load, T, non_file_backend_types)
{
  init<T>();

  configurator->set_value("/test/other/string", "1033");
  configurator->set_value("/test/other/int", 1033);
  configurator->set_value("/test/other/double", 1033.1033);
  configurator->set_value("/test/other/bool", true);

  configurator->save("temp-save");

  configurator->set_value("/test/other/string", "1034");
  configurator->set_value("/test/other/int", 1034);
  configurator->set_value("/test/other/double", 1034.1034);
  configurator->set_value("/test/other/bool", false);

  configurator->load("temp-save");

  std::string svalue;
  bool ok = configurator->get_value("test/other/string", svalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(svalue, "1034");

  int ivalue;
  ok = configurator->get_value("test/other/int", ivalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(ivalue, 1034);

  double dvalue;
  ok = configurator->get_value("test/other/double", dvalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(dvalue, 1034.1034);

  bool bvalue;
  ok = configurator->get_value("test/other/bool", bvalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(bvalue, false);

  configurator->set_value("/test/other/string", "1033");
  configurator->set_value("/test/other/int", 1033);
  configurator->set_value("/test/other/double", 1033.1033);
  configurator->set_value("/test/other/bool", true);

  configurator->save();

  configurator->set_value("/test/other/string", "1034");
  configurator->set_value("/test/other/int", 1034);
  configurator->set_value("/test/other/double", 1034.1034);
  configurator->set_value("/test/other/bool", false);

  configurator->load("temp-save");

  ok = configurator->get_value("test/other/string", svalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(svalue, "1034");

  ok = configurator->get_value("test/other/int", ivalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(ivalue, 1034);

  ok = configurator->get_value("test/other/double", dvalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(dvalue, 1034.1034);

  ok = configurator->get_value("test/other/bool", bvalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(bvalue, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_int, T, backend_types)
{
  init<T>();

  setting_int().set(1035);

  int value;
  bool ok = configurator->get_value("test/settings/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1035);
  BOOST_CHECK_EQUAL(setting_int()(), 1035);
  BOOST_CHECK_EQUAL(setting_int().get(), 1035);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_double, T, backend_types)
{
  init<T>();

  setting_double().set(1036.46);

  double value;
  bool ok = configurator->get_value("test/settings/double", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1036.46);
  BOOST_CHECK_EQUAL(setting_double()(), 1036.46);
  BOOST_CHECK_EQUAL(setting_double().get(), 1036.46);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_bool, T, backend_types)
{
  init<T>();

  setting_bool().set(true);

  bool value;
  bool ok = configurator->get_value("test/settings/bool", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, true);
  BOOST_CHECK_EQUAL(setting_bool()(), true);
  BOOST_CHECK_EQUAL(setting_bool().get(), true);

  setting_bool().set(false);

  ok = configurator->get_value("test/settings/bool", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, false);
  BOOST_CHECK_EQUAL(setting_bool()(), false);
  BOOST_CHECK_EQUAL(setting_bool().get(), false);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_enum, T, backend_types)
{
  init<T>();

  setting_mode().set(Mode::Mode1);

  int ivalue;
  bool ok = configurator->get_value("test/settings/mode", ivalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(ivalue, (int)Mode::Mode1);
  BOOST_CHECK_EQUAL(setting_mode()(), Mode::Mode1);
  BOOST_CHECK_EQUAL(setting_mode().get(), Mode::Mode1);

  setting_mode().set(Mode::Mode2);

  ok = configurator->get_value("test/settings/mode", ivalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(ivalue, (int)Mode::Mode2);
  BOOST_CHECK_EQUAL(setting_mode()(), Mode::Mode2);
  BOOST_CHECK_EQUAL(setting_mode().get(), Mode::Mode2);
};


BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_int_default, T, backend_types)
{
  init<T>();

  int value;
  bool ok = configurator->get_value("test/settings/default/int", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK_EQUAL(setting_int_default()(), has_defaults ? 1234 : 8888);
  BOOST_CHECK_EQUAL(setting_int_default().get(), has_defaults ? 1234 : 8888);

  setting_int_default().set(1035);

  ok = configurator->get_value("test/settings/default/int", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1035);
  BOOST_CHECK_EQUAL(setting_int_default()(), 1035);
  BOOST_CHECK_EQUAL(setting_int_default().get(), 1035);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_double_default, T, backend_types)
{
  init<T>();

  double value;
  bool ok = configurator->get_value("test/settings/default/double", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK_EQUAL(setting_double_default()(), has_defaults ? 12.34 : 88.88);
  BOOST_CHECK_EQUAL(setting_double_default().get(), has_defaults ? 12.34 : 88.88);

  setting_double_default().set(1036.46);

  ok = configurator->get_value("test/settings/default/double", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1036.46);
  BOOST_CHECK_EQUAL(setting_double_default()(), 1036.46);
  BOOST_CHECK_EQUAL(setting_double_default().get(), 1036.46);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_bool_default, T, backend_types)
{
  init<T>();

  bool value;
  bool ok = configurator->get_value("test/settings/default/bool", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK_EQUAL(setting_bool_default()(), true);
  BOOST_CHECK_EQUAL(setting_bool_default().get(), true);

  setting_bool_default().set(false);

  ok = configurator->get_value("test/settings/default/bool", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, false);
  BOOST_CHECK_EQUAL(setting_bool_default()(), false);
  BOOST_CHECK_EQUAL(setting_bool_default().get(), false);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_string_default, T, backend_types)
{
  init<T>();

  std::string value;
  bool ok = configurator->get_value("test/settings/default/string", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK_EQUAL(setting_string_default()(), has_defaults ? "default_string" : "8888");
  BOOST_CHECK_EQUAL(setting_string_default().get(), has_defaults ? "default_string" : "8888");

  setting_string_default().set("1037");

  ok = configurator->get_value("test/settings/default/string", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "1037");
  BOOST_CHECK_EQUAL(setting_string_default()(), "1037");
  BOOST_CHECK_EQUAL(setting_string_default().get(), "1037");
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_connect, T, backend_types)
{
  init<T>();

  setting_int().set(1038);

  int fired = 0;
  auto connection = setting_int().connect([&fired] (int value) {
    BOOST_CHECK_EQUAL(value, 1039);
    fired++;
  });

  BOOST_CHECK_EQUAL(fired, 0);
  setting_int().set(1039);
  BOOST_CHECK_EQUAL(fired, 1);
  connection.disconnect();
  setting_int().set(1040);
  BOOST_CHECK_EQUAL(fired, 1);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_attached, T, backend_types)
{
  init<T>();

  setting_int().set(1041);

  int fired = 0;
  auto connection = setting_int().attach([&fired] (int value) {
    BOOST_CHECK_EQUAL(value, fired == 0 ? 1041 : 1042);
    fired++;
  });

  BOOST_CHECK_EQUAL(fired, 1);
  setting_int().set(1042);
  BOOST_CHECK_EQUAL(fired, 2);
  connection.disconnect();
  setting_int().set(1043);
  BOOST_CHECK_EQUAL(fired, 2);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_group_connect, T, backend_types)
{
  init<T>();

  setting_int().set(1044);

  int fired = 0;
  auto connection = group().connect([&fired] () {
    fired++;
  });

  BOOST_CHECK_EQUAL(fired, 0);
  setting_int().set(1045);
  BOOST_CHECK_EQUAL(fired, 1);
  setting_double().set(1046.1);
  BOOST_CHECK_EQUAL(fired, 2);
  connection.disconnect();
  setting_int().set(1047);
  BOOST_CHECK_EQUAL(fired, 2);
};

BOOST_AUTO_TEST_SUITE_END()
