// Copyright (C) 2013 - 2021 Rob Caelers <robc@krandor.nl>
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

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#if SPDLOG_VERSION >= 10801
#  include <spdlog/cfg/env.h>
#endif

#include "SimulatedTime.hh"

#include "Configurator.hh"
#include "config/SettingCache.hh"
#include "utils/Logging.hh"
#include "utils/Enum.hh"

#include "IniConfigurator.hh"
#include "XmlConfigurator.hh"
#if defined(HAVE_GSETTINGS)
#  include "GSettingsConfigurator.hh"
#endif
#if defined(PLATFORM_OS_WINDOWS)
#  include "W32Configurator.hh"
#endif
#if defined(PLATFORM_OS_MACOS)
#  include "MacOSConfigurator.hh"
#  import <Foundation/NSUserDefaults.h>
#  import <Foundation/NSString.h>
#  import <Foundation/NSBundle.h>
#endif
#if defined(HAVE_QT)
#  include "QtSettingsConfigurator.hh"
#  include <QCoreApplication>
#endif

using namespace std;
using namespace workrave;
using namespace workrave::config;
using namespace workrave::utils;
using namespace std::literals::string_view_literals;

class GlobalFixture
{
public:
  GlobalFixture()
  {
  }
  ~GlobalFixture()
  {
  }

  void setup()
  {
    const auto *log_file = "workrave-config-test.log";

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, false);

    auto logger{std::make_shared<spdlog::logger>("workrave", file_sink)};
    spdlog::set_default_logger(logger);

    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%-5l%$] %v");

#if SPDLOG_VERSION >= 10801
    spdlog::cfg::load_env_levels();
#endif
  }

  void teardown()
  {
  }
};

enum class Mode
{
  Mode1,
  Mode2,
  Mode3
};

enum class ExtendedMode
{
  Invalid,
  Mode1,
  Mode2,
  Mode3
};

inline std::ostream &
operator<<(std::ostream &stream, Mode mode)
{
  switch (mode)
    {
    case Mode::Mode1:
      stream << "mode1";
      break;
    case Mode::Mode2:
      stream << "mode2";
      break;
    case Mode::Mode3:
      stream << "mode3";
      break;
    }
  return stream;
}

inline std::ostream &
operator<<(std::ostream &stream, ExtendedMode mode)
{
  switch (mode)
    {
    case ExtendedMode::Invalid:
      stream << "invalid";
      break;
    case ExtendedMode::Mode1:
      stream << "mode1";
      break;
    case ExtendedMode::Mode2:
      stream << "mode2";
      break;
    case ExtendedMode::Mode3:
      stream << "mode3";
      break;
    }
  return stream;
}

template<>
struct workrave::utils::enum_traits<ExtendedMode>
{
  static constexpr auto min = ExtendedMode::Mode1;
  static constexpr auto max = ExtendedMode::Mode3;
  static constexpr auto linear = true;
  static constexpr auto invalid = ExtendedMode::Invalid;

  static constexpr std::array<std::pair<std::string_view, ExtendedMode>, 5> names{{
    {"mode1", ExtendedMode::Mode1},
    {"mode2", ExtendedMode::Mode2},
    {"mode3", ExtendedMode::Mode3},
    {"invalid", ExtendedMode::Invalid},
  }};
};

class Fixture;
namespace helper
{
  template<typename T>
  void init(Fixture *fixture)
  {
  }
} // namespace helper

class Fixture
  : public IConfiguratorListener
  , public workrave::utils::Trackable
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

#if defined(HAVE_GSETTINGS)
    g_unsetenv("GSETTINGS_BACKEND");
    g_unsetenv("GSETTINGS_SCHEMA_DIR");
#endif
  };

  template<typename T>
  void init()
  {
    sim->reset();
    TimeSource::sync();

    helper::init<T>(this);
    configurator = std::make_shared<Configurator>(new T());
  }

  void tick() const
  {
    TimeSource::sync();
    configurator->heartbeat();
    sim->current_time += 1000000;
  }

  void tick(int seconds, const std::function<void(int count)> &check_func) const
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

  SettingGroup &group() const
  {
    return SettingCache::group(configurator, "test/settings");
  }

  Setting<int32_t> &setting_int32() const
  {
    return SettingCache::get<int32_t>(configurator, "test/settings/int32");
  }

  Setting<int64_t> &setting_int64() const
  {
    return SettingCache::get<int64_t>(configurator, "test/settings/int64");
  }

  Setting<int64_t, std::chrono::minutes> &setting_minutes() const
  {
    return SettingCache::get<int64_t, std::chrono::minutes>(configurator, "test/settings/minutes");
  }

  Setting<int32_t, std::chrono::hours> &setting_hours() const
  {
    return SettingCache::get<int32_t, std::chrono::hours>(configurator, "test/settings/hours");
  }

  Setting<int64_t, std::chrono::system_clock::time_point> &setting_time() const
  {
    return SettingCache::get<int64_t, std::chrono::system_clock::time_point>(configurator, "test/settings/time");
  }

  Setting<double> &setting_double() const
  {
    return SettingCache::get<double>(configurator, "test/settings/double");
  }

  Setting<std::string> &setting_string() const
  {
    return SettingCache::get<std::string>(configurator, "test/settings/string");
  }

  Setting<bool> &setting_bool() const
  {
    return SettingCache::get<bool>(configurator, "test/settings/bool");
  }

  Setting<std::vector<int32_t>> &setting_vector_int32() const
  {
    return SettingCache::get<std::vector<int32_t>>(configurator, "test/settings/vint32");
  }

  Setting<std::vector<std::string>> &setting_vector_string() const
  {
    return SettingCache::get<std::vector<std::string>>(configurator, "test/settings/vstring");
  }

  Setting<std::vector<int32_t>, std::vector<std::chrono::minutes>> &setting_vector_duration() const
  {
    return SettingCache::get<std::vector<int32_t>, std::vector<std::chrono::minutes>>(configurator, "test/settings/vduration");
  }

  Setting<int32_t, Mode> &setting_modedefault() const
  {
    return SettingCache::get<int32_t, Mode>(configurator, "test/settings/mode");
  }

  Setting<int32_t, Mode> &setting_mode() const
  {
    return SettingCache::get<int32_t, Mode>(configurator, "test/settings/mode");
  }

  Setting<int32_t, ExtendedMode> &setting_extendedmode_int() const
  {
    return SettingCache::get<int32_t, ExtendedMode>(configurator, "test/settings/extendedmode_int");
  }

  Setting<std::string, ExtendedMode> &setting_extendedmode_string() const
  {
    return SettingCache::get<std::string, ExtendedMode>(configurator, "test/settings/extendedmode_string");
  }

  Setting<int32_t> &setting_int32_default() const
  {
    return SettingCache::get<int32_t>(configurator, "test/settings/default/int32", 8888);
  }

  Setting<int64_t> &setting_int64_default() const
  {
    return SettingCache::get<int64_t>(configurator, "test/settings/default/int64", INT64_C(8888));
  }

  Setting<double> &setting_double_default() const
  {
    return SettingCache::get<double>(configurator, "test/settings/default/double", 88.88);
  }

  Setting<std::string> &setting_string_default() const
  {
    return SettingCache::get<std::string>(configurator, "test/settings/default/string", std::string("8888"));
  }

  Setting<bool> &setting_bool_default() const
  {
    return SettingCache::get<bool>(configurator, "test/settings/default/bool", true);
  }

  SimulatedTime::Ptr sim;
  Configurator::Ptr configurator;
  bool has_defaults{false};
  bool can_remove{true};
  std::string expected_key;
  int config_changed_count{0};
};

namespace helper
{
#if defined(HAVE_GSETTINGS)
  template<>
  void init<GSettingsConfigurator>(Fixture *fixture)
  {
    g_setenv("GSETTINGS_SCHEMA_DIR", BUILDDIR, true);
    g_setenv("GSETTINGS_BACKEND", "memory", 1);
    fixture->has_defaults = true;
    fixture->can_remove = false;
  }
#endif
#if defined(HAVE_QT)
  template<>
  void init<QtSettingsConfigurator>(Fixture *fixture)
  {
    QCoreApplication::setOrganizationName("Workrave");
    QCoreApplication::setOrganizationDomain("workrave.org");
    QCoreApplication::setApplicationName("WorkraveConfigTest");

    QSettings settings;
    settings.clear();
  }
#endif
#if defined(PLATFORM_OS_MACOS)
  template<>
  void init<MacOSConfigurator>(Fixture *fixture)
  {
    // NSString *appDomain = [[NSBundle mainBundle] bundleIdentifier];
    // [[NSUserDefaults standardUserDefaults] setPersistentDomain:[NSDictionary dictionary] forName:appDomain];

    NSDictionary *defaultsDictionary = [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];
    for (NSString *key in [defaultsDictionary allKeys])
      {
        [[NSUserDefaults standardUserDefaults] removeObjectForKey:key];
      }
  }
#endif
#if defined(PLATFORM_OS_WINDOWS)
  template<>
  void init<W32Configurator>(Fixture *fixture)
  {
    RegDeleteTree(HKEY_CURRENT_USER, TEXT("Software\\Workrave\\test"));
  }
#endif

} // namespace helper

BOOST_TEST_GLOBAL_FIXTURE(GlobalFixture);

BOOST_FIXTURE_TEST_SUITE(config, Fixture)

using backend_types = boost::mpl::list<IniConfigurator,
                                       XmlConfigurator
#if defined(HAVE_GSETTINGS)
                                       ,
                                       GSettingsConfigurator
#endif
#if defined(HAVE_QT)
                                       ,
                                       QtSettingsConfigurator
#endif
#if defined(PLATFORM_OS_MACOS)
                                       ,
                                       MacOSConfigurator
#endif
#if defined(PLATFORM_OS_WINDOWS)
                                       ,
                                       W32Configurator
#endif
                                       >;

using non_file_backend_types = boost::mpl::list<
#if defined(HAVE_GSETTINGS)
  GSettingsConfigurator
#endif
  >;

using file_backend_types = boost::mpl::list<XmlConfigurator, IniConfigurator>;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_string, T, backend_types)
{
  init<T>();

  bool ok{false};

  std::string value;
  ok = configurator->get_value("test/schema-defaults/string", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK(!has_defaults || value == "default_string");
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/string"), false);

  configurator->set_value("test/schema-defaults/string", std::string{"string_value"});
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/string"), true);

  ok = configurator->get_value("test/schema-defaults/string", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "string_value");

  configurator->set_value("test/schema-defaults/string", std::string{"other_string_value"});

  ok = configurator->get_value("test/schema-defaults/string", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "other_string_value");
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_charstring, T, backend_types)
{
  init<T>();

  bool ok{false};

  std::string value;
  ok = configurator->get_value("test/schema-defaults/charstring", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK(!has_defaults || value == "default_charstring");
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/charstring"), false);

  configurator->set_value("test/schema-defaults/charstring", "charstring_value");
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/charstring"), true);

  ok = configurator->get_value("test/schema-defaults/charstring", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "charstring_value");

  configurator->set_value("test/schema-defaults/charstring", "other_charstring_value");
  BOOST_CHECK_EQUAL(ok, true);

  ok = configurator->get_value("test/schema-defaults/charstring", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "other_charstring_value");
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_int32, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t value;
  ok = configurator->get_value("test/schema-defaults/int32", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK(!has_defaults || value == 1234);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/int32"), false);

  configurator->set_value("test/schema-defaults/int32", 11);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/int32"), true);

  ok = configurator->get_value("test/schema-defaults/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 11);

  configurator->set_value("test/schema-defaults/int32", 22);

  ok = configurator->get_value("test/schema-defaults/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 22);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_int64, T, backend_types)
{
  init<T>();

  bool ok{false};

  int64_t value;
  ok = configurator->get_value("test/schema-defaults/int64", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK(!has_defaults || value == INT64_C(1234));
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/int64"), false);

  configurator->set_value("test/schema-defaults/int64", INT64_C(11));
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/int64"), true);

  ok = configurator->get_value("test/schema-defaults/int64", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 11L);

  configurator->set_value("test/schema-defaults/int64", INT64_C(22));

  ok = configurator->get_value("test/schema-defaults/int64", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, INT64_C(22));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_double, T, backend_types)
{
  init<T>();

  bool ok{false};

  double value;
  ok = configurator->get_value("test/schema-defaults/double", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK(!has_defaults || value == 12.34);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/double"), false);

  configurator->set_value("test/schema-defaults/double", 11.11);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/double"), true);

  ok = configurator->get_value("test/schema-defaults/double", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 11.11);

  configurator->set_value("test/schema-defaults/double", 22.22);

  ok = configurator->get_value("test/schema-defaults/double", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 22.22);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_bool, T, backend_types)
{
  init<T>();

  bool ok{false};

  bool value;
  ok = configurator->get_value("test/schema-defaults/bool", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK(!has_defaults || value);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/bool"), false);

  configurator->set_value("test/schema-defaults/bool", true);
  BOOST_CHECK_EQUAL(configurator->has_user_value("test/schema-defaults/bool"), true);

  ok = configurator->get_value("test/schema-defaults/bool", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, true);

  configurator->set_value("test/schema-defaults/bool", false);

  ok = configurator->get_value("test/schema-defaults/bool", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_string_default, T, backend_types)
{
  init<T>();
  std::string value;
  configurator->get_value_with_default("test/code-defaults/string", value, "11");
  BOOST_CHECK_EQUAL(value, has_defaults ? "default_string" : "11");

  configurator->set_value("test/code-defaults/string", std::string{"22"});

  configurator->get_value_with_default("test/code-defaults/string", value, "11");
  BOOST_CHECK_EQUAL(value, "22");
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_int32_default, T, backend_types)
{
  init<T>();

  int32_t value;
  configurator->get_value_with_default("test/code-defaults/int32", value, 33);
  BOOST_CHECK_EQUAL(value, has_defaults ? 1234 : 33);

  configurator->set_value("test/code-defaults/int32", 44);

  configurator->get_value_with_default("test/code-defaults/int32", value, 33);
  BOOST_CHECK_EQUAL(value, 44);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_int64_default, T, backend_types)
{
  init<T>();

  int64_t value;
  configurator->get_value_with_default("test/code-defaults/int64", value, INT64_C(33));
  BOOST_CHECK_EQUAL(value, has_defaults ? INT64_C(1234) : INT64_C(33));

  configurator->set_value("test/code-defaults/int64", INT64_C(44));

  configurator->get_value_with_default("test/code-defaults/int64", value, INT64_C(33));
  BOOST_CHECK_EQUAL(value, INT64_C(44));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_double_default, T, backend_types)
{
  init<T>();

  double value;
  configurator->get_value_with_default("test/code-defaults/double", value, 33.33);
  BOOST_CHECK_EQUAL(value, has_defaults ? 12.34 : 33.33);

  configurator->set_value("test/code-defaults/double", 44.44);

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

  configurator->set_value("test/code-defaults/bool", true);

  configurator->get_value_with_default("test/code-defaults/bool", value, false);
  BOOST_CHECK_EQUAL(value, true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_string_wrong_type, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t ivalue;
  ok = configurator->get_value("test/other/string", ivalue);
  BOOST_CHECK_EQUAL(ok, false);

  int64_t i64value;
  ok = configurator->get_value("test/other/string", i64value);
  BOOST_CHECK_EQUAL(ok, false);

  double dvalue;
  ok = configurator->get_value("test/other/string", dvalue);
  BOOST_CHECK_EQUAL(ok, false);

  bool bvalue;
  ok = configurator->get_value("test/other/string", bvalue);
  BOOST_CHECK_EQUAL(ok, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_int32_wrong_type, T, backend_types)
{
  init<T>();

  bool ok{false};

  std::string svalue;
  ok = configurator->get_value("test/other/int32", svalue);
  BOOST_CHECK_EQUAL(ok, false);

  int64_t i64value;
  ok = configurator->get_value("test/other/int32", i64value);
  BOOST_CHECK_EQUAL(ok, false);

  double dvalue;
  ok = configurator->get_value("test/other/int32", dvalue);
  BOOST_CHECK_EQUAL(ok, false);

  bool bvalue;
  ok = configurator->get_value("test/other/int32", bvalue);
  BOOST_CHECK_EQUAL(ok, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_int64_wrong_type, T, backend_types)
{
  init<T>();

  bool ok{false};

  std::string svalue;
  ok = configurator->get_value("test/other/int64", svalue);
  BOOST_CHECK_EQUAL(ok, false);

  int32_t ivalue;
  ok = configurator->get_value("test/other/int64", ivalue);
  BOOST_CHECK_EQUAL(ok, false);

  double dvalue;
  ok = configurator->get_value("test/other/int64", dvalue);
  BOOST_CHECK_EQUAL(ok, false);

  bool bvalue;
  ok = configurator->get_value("test/other/int64", bvalue);
  BOOST_CHECK_EQUAL(ok, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_double_wrong_type, T, backend_types)
{
  init<T>();

  bool ok{false};

  std::string svalue;
  ok = configurator->get_value("test/other/double", svalue);
  BOOST_CHECK_EQUAL(ok, false);

  int32_t ivalue;
  ok = configurator->get_value("test/other/double", ivalue);
  BOOST_CHECK_EQUAL(ok, false);

  int64_t i64value;
  ok = configurator->get_value("test/other/double", i64value);
  BOOST_CHECK_EQUAL(ok, false);

  bool bvalue;
  ok = configurator->get_value("test/other/double", bvalue);
  BOOST_CHECK_EQUAL(ok, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_bool_wrong_type, T, backend_types)
{
  init<T>();

  bool ok{false};

  std::string svalue;
  ok = configurator->get_value("test/other/bool", svalue);
  BOOST_CHECK_EQUAL(ok, false);

  int32_t ivalue;
  ok = configurator->get_value("test/other/bool", ivalue);
  BOOST_CHECK_EQUAL(ok, false);

  int64_t i64value;
  ok = configurator->get_value("test/other/bool", i64value);
  BOOST_CHECK_EQUAL(ok, false);

  double dvalue;
  ok = configurator->get_value("test/other/bool", dvalue);
  BOOST_CHECK_EQUAL(ok, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_bad_key, T, backend_types)
{
  init<T>();

  bool ok{false};

  std::string value;
  ok = configurator->get_value("", value);
  // BOOST_CHECK_EQUAL(ok, false); TODO: check for IniConfigurator

  ok = configurator->get_value("/", value);
  // BOOST_CHECK_EQUAL(ok, false); TODO: check for IniConfigurator

  ok = configurator->get_value(" ", value);
  BOOST_CHECK_EQUAL(ok, false);

  ok = configurator->get_value("lskjflskd", value);
  BOOST_CHECK_EQUAL(ok, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_listener_one, T, backend_types)
{
  init<T>();

  bool ok{false};

  ok = configurator->add_listener("test/other/int32", this);
  BOOST_CHECK_EQUAL(ok, true);

  expected_key = "test/other/int32";
  configurator->set_value("test/other/int32", 1001);
  BOOST_CHECK_EQUAL(config_changed_count, 1);

  ok = configurator->remove_listener("test", this);
  BOOST_CHECK_EQUAL(ok, false);

  configurator->set_value("test/other/int32", 1002);
  BOOST_CHECK_EQUAL(config_changed_count, 2);

  configurator->set_value("test/other/int32", 1002);
  BOOST_CHECK_EQUAL(config_changed_count, 2);

  configurator->set_value("test/other/double", 1002.1002);
  BOOST_CHECK_EQUAL(config_changed_count, 2);

  ok = configurator->remove_listener("test/other/int32", this);
  BOOST_CHECK_EQUAL(ok, true);

  configurator->set_value("test/other/int32", 1003);
  BOOST_CHECK_EQUAL(config_changed_count, 2);

  ok = configurator->add_listener("test/other/int32/", this);

  configurator->set_value("test/other/int32", 1001);
  BOOST_CHECK_EQUAL(config_changed_count, 3);

  ok = configurator->remove_listener("test/other/int32", this);
  BOOST_CHECK_EQUAL(ok, true);

  configurator->set_value("test/other/int32", 1004);
  BOOST_CHECK_EQUAL(config_changed_count, 3);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_listener_section, T, backend_types)
{
  init<T>();

  bool ok{false};

  ok = configurator->add_listener("test/other/", this);
  BOOST_CHECK_EQUAL(ok, true);

  expected_key = "test/other/int32";
  configurator->set_value("test/other/int32", 1005);
  BOOST_CHECK_EQUAL(config_changed_count, 1);

  expected_key = "test/other/double";
  configurator->set_value("test/other/double", 1005.55);
  BOOST_CHECK_EQUAL(config_changed_count, 2);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_listener_multiple, T, backend_types)
{
  init<T>();

  bool ok{false};

  ok = configurator->add_listener("test/other/int32", this);
  BOOST_CHECK_EQUAL(ok, true);
  ok = configurator->add_listener("test/other/double/", this);
  BOOST_CHECK_EQUAL(ok, true);

  expected_key = "test/other/int32";
  configurator->set_value("test/other/int32", 1006);
  BOOST_CHECK_EQUAL(config_changed_count, 1);

  expected_key = "test/other/double";
  configurator->set_value("test/other/double", 1006.1006);
  BOOST_CHECK_EQUAL(config_changed_count, 2);

  ok = configurator->remove_listener("test/other/double", this);
  ok = configurator->remove_listener("test/other/int32", this);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_listener_add_remove, T, backend_types)
{
  init<T>();

  bool ok{false};

  ok = configurator->add_listener("test/other/int32", this);
  BOOST_CHECK_EQUAL(ok, true);
  ok = configurator->add_listener("test/other/int64", this);
  BOOST_CHECK_EQUAL(ok, true);
  ok = configurator->add_listener("test/other/double/", this);
  BOOST_CHECK_EQUAL(ok, true);
  ok = configurator->add_listener("test/other/double/", (IConfiguratorListener *)0xdeadbeef);
  BOOST_CHECK_EQUAL(ok, true);
  ok = configurator->add_listener("test/other/double/", (IConfiguratorListener *)0xdeadbeef);
  BOOST_CHECK_EQUAL(ok, false);
  ok = configurator->add_listener("test/other/string", (IConfiguratorListener *)0xbaaaaaad);
  BOOST_CHECK_EQUAL(ok, true);

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
  ok = configurator->remove_listener("test/other/int32", (IConfiguratorListener *)0xdeadbeef);
  BOOST_CHECK_EQUAL(ok, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_leading_slash, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t value;
  configurator->set_value("/test/other/int32", 1007);

  ok = configurator->get_value("/test/other/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1007);

  configurator->set_value("/test/other/int32", 1008);

  ok = configurator->get_value("/test/other/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1008);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_trailing_slash, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t value;
  configurator->set_value("test/other/int32/", 1009);

  ok = configurator->get_value("test/other/int32/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1009);

  configurator->set_value("test/other/int32/", 1010);

  ok = configurator->get_value("test/other/int32/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1010);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_leading_trailing_slash, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t value;
  configurator->set_value("/test/other/int32/", 1011);

  ok = configurator->get_value("/test/other/int32/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1011);

  configurator->set_value("/test/other/int32/", 1012);

  ok = configurator->get_value("/test/other/int32/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1012);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t value;
  configurator->set_value("test/other/int32", 1013);

  ok = configurator->get_value("test/other/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1013);

  configurator->set_delay("test/other/int32", 5);

  ok = configurator->add_listener("test/other/int32", this);
  BOOST_CHECK_EQUAL(ok, true);

  configurator->set_value("/test/other/int32/", 1014);
  BOOST_CHECK_EQUAL(config_changed_count, 0);

  expected_key = "test/other/int32";
  ok = configurator->get_value("test/other/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1014);

  tick(6, [](int32_t c) {});
  BOOST_CHECK_EQUAL(config_changed_count, 1);

  ok = configurator->remove_listener(this);
  BOOST_CHECK_EQUAL(ok, true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay_repeat, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t value;
  configurator->set_value("test/other/int32", 1015);

  ok = configurator->get_value("test/other/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1015);

  configurator->set_delay("test/other/int32", 5);
  configurator->set_delay("test/other/int32", 10);

  ok = configurator->add_listener("test/other/int32", this);
  BOOST_CHECK_EQUAL(ok, true);

  configurator->set_value("/test/other/int32/", 1016);
  BOOST_CHECK_EQUAL(config_changed_count, 0);

  expected_key = "test/other/int32";
  ok = configurator->get_value("test/other/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1016);

  tick(9, [](int32_t c) {});
  BOOST_CHECK_EQUAL(config_changed_count, 0);
  tick(2, [](int32_t c) {});
  BOOST_CHECK_EQUAL(config_changed_count, 1);

  ok = configurator->remove_listener(this);
  BOOST_CHECK_EQUAL(ok, true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay_save_load, T, file_backend_types)
{
  init<T>();

  bool ok{false};

  configurator->load("temp-save");

  int32_t value;
  configurator->set_value("test/other/int32", 1017);

  ok = configurator->get_value("test/other/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1017);

  configurator->set_delay("test/other/int32", 5);

  ok = configurator->add_listener("test/other/int32", this);
  BOOST_CHECK_EQUAL(ok, true);

  configurator->save();

  configurator->set_value("/test/other/int32/", 1018);
  BOOST_CHECK_EQUAL(config_changed_count, 0);

  expected_key = "test/other/int32";
  ok = configurator->get_value("test/other/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1018);

  tick(6, [](int32_t c) {});
  BOOST_CHECK_EQUAL(config_changed_count, 1);

  configurator->load("temp-save");

  ok = configurator->get_value("test/other/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1017);

  ok = configurator->remove_listener(this);
  BOOST_CHECK_EQUAL(ok, true);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay_immediate, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t value;

  configurator->set_delay("test/other/int32", 5);

  configurator->set_value("test/other/int32", 1019, CONFIG_FLAG_IMMEDIATE);

  ok = configurator->get_value("/test/other/int32/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1019);

  configurator->set_value("test/other/double", 1019.1019, CONFIG_FLAG_IMMEDIATE);

  double dv;
  ok = configurator->get_value("/test/other/double/", dv);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(dv, 1019.1019);
}

// BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay_invalid, T, backend_types)
// {
//   init<T>();

//   bool ok { false };

//   int32_t value;

//   configurator->set_delay("test/other/invalid", 5);

//   ok = configurator->set_value("test/other/invalid", 89);
//   BOOST_CHECK_EQUAL(ok, true);

//   ok = configurator->set_value("test/other/invalid", 88.1, CONFIG_FLAG_IMMEDIATE);
//   BOOST_CHECK_EQUAL(ok, true);
// }

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay_same_value, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t value;
  configurator->set_value("test/other/int32", 1020);

  ok = configurator->get_value("test/other/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1020);

  configurator->set_delay("test/other/int32", 5);

  configurator->set_value("/test/other/int32/", 1021);

  configurator->set_value("/test/other/int32/", 1020);

  tick(6, [](int32_t c) {});

  ok = configurator->get_value("test/other/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1020);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay_initial_value, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t value;
  ok = configurator->get_value("test/other/delay-initial", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  configurator->set_delay("test/other/delay-initial", 5);

  configurator->set_value("/test/other/delay-initial/", 1022);

  tick(6, [](int32_t c) {});

  ok = configurator->get_value("test/other/delay-initial", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1022);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_remove, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t value;
  ok = configurator->get_value("/test/other/int32/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  configurator->set_value("/test/other/int32/", 1023);

  ok = configurator->get_value("/test/other/int32/", value);
  BOOST_CHECK_EQUAL(ok, true);

  configurator->remove_key("/test/other/int32/");

  ok = configurator->get_value("/test/other/int32/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  if (has_defaults)
    {
      BOOST_CHECK_EQUAL(value, 1234);
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_rename_int32_t, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t value;
  ok = configurator->get_value("/test/other/int32/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->get_value("/test/other/int32_2/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  configurator->set_value("/test/other/int32/", 1024);

  ok = configurator->get_value("/test/other/int32/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1024);

  configurator->rename_key("/test/other/int32/", "/test/other/int32_2/");

  ok = configurator->get_value("/test/other/int32_2/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1024);

  ok = configurator->get_value("/test/other/int32/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_rename_int64_t, T, backend_types)
{
  init<T>();

  bool ok{false};

  int64_t value;
  ok = configurator->get_value("/test/other/int64/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->get_value("/test/other/int64-2/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  configurator->set_value("/test/other/int64/", INT64_C(1024));

  ok = configurator->get_value("/test/other/int64/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, INT64_C(1024));

  configurator->rename_key("/test/other/int64/", "/test/other/int64-2/");

  ok = configurator->get_value("/test/other/int64-2/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, INT64_C(1024));

  ok = configurator->get_value("/test/other/int64/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_rename_bool, T, backend_types)
{
  init<T>();

  bool ok{false};

  bool value;
  ok = configurator->get_value("/test/other/bool/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->get_value("/test/other/bool2/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  configurator->set_value("/test/other/bool/", true);

  ok = configurator->get_value("/test/other/bool/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, true);

  configurator->rename_key("/test/other/bool/", "/test/other/bool2/");

  ok = configurator->get_value("/test/other/bool2/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, true);

  ok = configurator->get_value("/test/other/bool/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_rename_double, T, backend_types)
{
  init<T>();

  bool ok{false};

  double value;
  ok = configurator->get_value("/test/other/double/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->get_value("/test/other/double2/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  configurator->set_value("/test/other/double/", 1025.1025);

  ok = configurator->get_value("/test/other/double/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1025.1025);

  configurator->rename_key("/test/other/double/", "/test/other/double2/");

  ok = configurator->get_value("/test/other/double2/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1025.1025);

  ok = configurator->get_value("/test/other/double/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_rename_string, T, backend_types)
{
  init<T>();

  bool ok{false};

  std::string value;
  ok = configurator->get_value("/test/other/string/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->get_value("/test/other/string2/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  configurator->set_value("/test/other/string/", "27");

  ok = configurator->get_value("/test/other/string/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "27");

  configurator->rename_key("/test/other/string/", "/test/other/string2/");

  ok = configurator->get_value("/test/other/string2/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "27");

  ok = configurator->get_value("/test/other/string/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_rename_exists, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t value;
  ok = configurator->get_value("/test/other/int32/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  ok = configurator->get_value("/test/other/int32-2/", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  configurator->set_value("/test/other/int32/", 1026);

  configurator->set_value("/test/other/int32-2/", 1027);

  configurator->rename_key("/test/other/int32/", "/test/other/int32-2/");
  // FIXME: check logging BOOST_CHECK_EQUAL(ok, false);

  ok = configurator->get_value("/test/other/int32-2/", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1027);

  ok = configurator->get_value("/test/other/int32/", value);
  BOOST_CHECK_EQUAL(ok, !can_remove);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_initial, T, backend_types)
{
  init<T>();

  bool ok{false};

  int32_t value;
  ok = configurator->get_value("/test/other/initial", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);

  configurator->set_value("/test/other/initial", 1028, CONFIG_FLAG_INITIAL);

  ok = configurator->get_value("/test/other/initial", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, has_defaults ? 1234 : 1028);

  configurator->set_value("/test/other/initial", 1029, CONFIG_FLAG_INITIAL);

  ok = configurator->get_value("/test/other/initial", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, has_defaults ? 1234 : 1028);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_save_load, T, file_backend_types)
{
  init<T>();

  configurator->load("temp-save");

  configurator->set_value("/test/other/string", "1030");
  configurator->set_value("/test/other/int32", 1030);
  configurator->set_value("/test/other/double", 1030.1030);
  configurator->set_value("/test/other/bool", true);

  configurator->save();

  configurator->set_value("/test/other/string", "1031");
  configurator->set_value("/test/other/int32", 1031);
  configurator->set_value("/test/other/double", 1031.1031);
  configurator->set_value("/test/other/bool", false);

  configurator->load("temp-save");

  std::string svalue;
  bool ok = configurator->get_value("test/other/string", svalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(svalue, "1030");

  int32_t ivalue;
  ok = configurator->get_value("test/other/int32", ivalue);
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
  configurator->set_value("/test/other/int32", 1031);
  configurator->set_value("/test/other/double", 1031.1031);
  configurator->set_value("/test/other/bool", false);

  configurator->save();

  configurator->set_value("/test/other/string", "1032");
  configurator->set_value("/test/other/int32", 1032);
  configurator->set_value("/test/other/double", 1032.1032);
  configurator->set_value("/test/other/bool", true);

  configurator->load("temp-save");

  ok = configurator->get_value("test/other/string", svalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(svalue, "1031");

  ok = configurator->get_value("test/other/int32", ivalue);
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

  configurator->load("temp-save");

  configurator->set_value("/test/other/string", "1033");
  configurator->set_value("/test/other/int32", 1033);
  configurator->set_value("/test/other/double", 1033.1033);
  configurator->set_value("/test/other/bool", true);

  configurator->save();

  configurator->set_value("/test/other/string", "1034");
  configurator->set_value("/test/other/int32", 1034);
  configurator->set_value("/test/other/double", 1034.1034);
  configurator->set_value("/test/other/bool", false);

  configurator->load("temp-save");

  std::string svalue;
  bool ok = configurator->get_value("test/other/string", svalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(svalue, "1034");

  int32_t ivalue;
  ok = configurator->get_value("test/other/int32", ivalue);
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
  configurator->set_value("/test/other/int32", 1033);
  configurator->set_value("/test/other/double", 1033.1033);
  configurator->set_value("/test/other/bool", true);

  configurator->save();

  configurator->set_value("/test/other/string", "1034");
  configurator->set_value("/test/other/int32", 1034);
  configurator->set_value("/test/other/double", 1034.1034);
  configurator->set_value("/test/other/bool", false);

  configurator->load("temp-save");

  ok = configurator->get_value("test/other/string", svalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(svalue, "1034");

  ok = configurator->get_value("test/other/int32", ivalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(ivalue, 1034);

  ok = configurator->get_value("test/other/double", dvalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(dvalue, 1034.1034);

  ok = configurator->get_value("test/other/bool", bvalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(bvalue, false);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_int32, T, backend_types)
{
  init<T>();

  setting_int32().set(1035);

  int32_t value;
  bool ok = configurator->get_value("test/settings/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1035);
  BOOST_CHECK_EQUAL(setting_int32()(), 1035);
  BOOST_CHECK_EQUAL(setting_int32().get(), 1035);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_int64, T, backend_types)
{
  init<T>();

  setting_int64().set(INT64_C(1035));

  int64_t value;
  bool ok = configurator->get_value("test/settings/int64", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, INT64_C(1035));
  BOOST_CHECK_EQUAL(setting_int64()(), INT64_C(1035));
  BOOST_CHECK_EQUAL(setting_int64().get(), INT64_C(1035));
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

  int32_t ivalue;
  bool ok = configurator->get_value("test/settings/mode", ivalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(ivalue, (int32_t)Mode::Mode1);
  BOOST_CHECK_EQUAL(setting_mode()(), Mode::Mode1);
  BOOST_CHECK_EQUAL(setting_mode().get(), Mode::Mode1);

  setting_mode().set(Mode::Mode2);

  ok = configurator->get_value("test/settings/mode", ivalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(ivalue, (int32_t)Mode::Mode2);
  BOOST_CHECK_EQUAL(setting_mode()(), Mode::Mode2);
  BOOST_CHECK_EQUAL(setting_mode().get(), Mode::Mode2);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_enum_trait_int, T, backend_types)
{
  init<T>();

  setting_extendedmode_int().set(ExtendedMode::Mode1);

  int32_t ivalue;
  bool ok = configurator->get_value("test/settings/extendedmode_int", ivalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(ivalue, (int32_t)ExtendedMode::Mode1);
  BOOST_CHECK_EQUAL(setting_extendedmode_int()(), ExtendedMode::Mode1);
  BOOST_CHECK_EQUAL(setting_extendedmode_int().get(), ExtendedMode::Mode1);

  setting_extendedmode_int().set(ExtendedMode::Mode2);

  ok = configurator->get_value("test/settings/extendedmode_int", ivalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(ivalue, (int32_t)ExtendedMode::Mode2);
  BOOST_CHECK_EQUAL(setting_extendedmode_int()(), ExtendedMode::Mode2);
  BOOST_CHECK_EQUAL(setting_extendedmode_int().get(), ExtendedMode::Mode2);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_enum_trait_string, T, backend_types)
{
  init<T>();

  setting_extendedmode_string().set(ExtendedMode::Mode1);

  std::string svalue;
  bool ok = configurator->get_value("test/settings/extendedmode_string", svalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(svalue, "mode1");
  BOOST_CHECK_EQUAL(setting_extendedmode_string()(), ExtendedMode::Mode1);
  BOOST_CHECK_EQUAL(setting_extendedmode_string().get(), ExtendedMode::Mode1);

  setting_extendedmode_string().set(ExtendedMode::Mode2);

  ok = configurator->get_value("test/settings/extendedmode_string", svalue);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(svalue, "mode2");
  BOOST_CHECK_EQUAL(setting_extendedmode_string()(), ExtendedMode::Mode2);
  BOOST_CHECK_EQUAL(setting_extendedmode_string().get(), ExtendedMode::Mode2);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_minutes, T, backend_types)
{
  init<T>();

  std::chrono::minutes m{28};

  setting_minutes().set(m);

  int64_t value;
  bool ok = configurator->get_value("test/settings/minutes", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, INT64_C(28));
  BOOST_CHECK(setting_minutes()() == m);
  BOOST_CHECK(setting_minutes().get() == m);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_hours, T, backend_types)
{
  init<T>();

  std::chrono::hours h{54};

  setting_hours().set(h);

  int32_t value;
  bool ok = configurator->get_value("test/settings/hours", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 54);
  BOOST_CHECK(setting_hours()() == h);
  BOOST_CHECK(setting_hours().get() == h);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_time, T, backend_types)
{
  init<T>();

  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

  setting_time().set(now);
  int64_t now64 = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

  int64_t value;
  bool ok = configurator->get_value("test/settings/time", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, now64);
  BOOST_CHECK(std::chrono::duration_cast<std::chrono::seconds>(setting_time()().time_since_epoch()).count() == now64);
  BOOST_CHECK(std::chrono::duration_cast<std::chrono::seconds>(setting_time().get().time_since_epoch()).count() == now64);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_vector_int32, T, backend_types)
{
  init<T>();

  std::vector<int> values{4, 8, 15, 16, 23, 42};

  setting_vector_int32().set(values);

  std::string value;
  bool ok = configurator->get_value("test/settings/vint32", value);

  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "4;8;15;16;23;42");
  BOOST_TEST(setting_vector_int32()() == values);
  BOOST_TEST(setting_vector_int32().get() == values);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_vector_duration, T, backend_types)
{
  init<T>();

  std::vector<std::chrono::minutes> values{std::chrono::minutes(0),
                                           std::chrono::minutes(4),
                                           std::chrono::minutes(8),
                                           std::chrono::minutes(15),
                                           std::chrono::minutes(16),
                                           std::chrono::minutes(23),
                                           std::chrono::minutes(42)};

  setting_vector_duration().set(values);

  std::string value;
  bool ok = configurator->get_value("test/settings/vduration", value);

  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "0;4;8;15;16;23;42");
  BOOST_TEST(setting_vector_duration()() == values);
  BOOST_TEST(setting_vector_duration().get() == values);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_vector_string, T, backend_types)
{
  init<T>();

  std::vector<std::string> values{"hydra", "arrow", "swan", "flame", "pearl"};

  setting_vector_string().set(values);

  std::string value;
  bool ok = configurator->get_value("test/settings/vstring", value);

  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, "hydra;arrow;swan;flame;pearl");
  BOOST_TEST(setting_vector_string()() == values);
  BOOST_TEST(setting_vector_string().get() == values);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_int32_default, T, backend_types)
{
  init<T>();

  int32_t value;
  bool ok = configurator->get_value("test/settings/default/int32", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK_EQUAL(setting_int32_default()(), has_defaults ? 1234 : 8888);
  BOOST_CHECK_EQUAL(setting_int32_default().get(), has_defaults ? 1234 : 8888);

  setting_int32_default().set(1035);

  ok = configurator->get_value("test/settings/default/int32", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, 1035);
  BOOST_CHECK_EQUAL(setting_int32_default()(), 1035);
  BOOST_CHECK_EQUAL(setting_int32_default().get(), 1035);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_int64_default, T, backend_types)
{
  init<T>();

  int64_t value;
  bool ok = configurator->get_value("test/settings/default/int64", value);
  BOOST_CHECK_EQUAL(ok, has_defaults);
  BOOST_CHECK_EQUAL(setting_int64_default()(), has_defaults ? INT64_C(1234) : INT64_C(8888));
  BOOST_CHECK_EQUAL(setting_int64_default().get(), has_defaults ? INT64_C(1234) : INT64_C(8888));

  setting_int64_default().set(INT64_C(1035));

  ok = configurator->get_value("test/settings/default/int64", value);
  BOOST_CHECK_EQUAL(ok, true);
  BOOST_CHECK_EQUAL(value, INT64_C(1035));
  BOOST_CHECK_EQUAL(setting_int64_default()(), INT64_C(1035));
  BOOST_CHECK_EQUAL(setting_int64_default().get(), INT64_C(1035));
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

  setting_int32().set(1038);

  int fired = 0;
  auto connection = setting_int32().connect(this, [&fired](int32_t value) {
    BOOST_CHECK_EQUAL(value, 1039);
    fired++;
  });

  BOOST_CHECK_EQUAL(fired, 0);
  setting_int32().set(1039);
  BOOST_CHECK_EQUAL(fired, 1);
  connection.disconnect();
  setting_int32().set(1040);
  BOOST_CHECK_EQUAL(fired, 1);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_attached, T, backend_types)
{
  init<T>();

  setting_int32().set(1041);

  int fired = 0;
  auto connection = setting_int32().attach(this, [&fired](int32_t value) {
    BOOST_CHECK_EQUAL(value, fired == 0 ? 1041 : 1042);
    fired++;
  });

  BOOST_CHECK_EQUAL(fired, 1);
  setting_int32().set(1042);
  BOOST_CHECK_EQUAL(fired, 2);
  connection.disconnect();
  setting_int32().set(1043);
  BOOST_CHECK_EQUAL(fired, 2);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_group_connect, T, backend_types)
{
  init<T>();

  setting_int32().set(1044);

  int fired = 0;
  auto connection = group().connect(this, [&fired]() { fired++; });

  BOOST_CHECK_EQUAL(fired, 0);
  setting_int32().set(1045);
  BOOST_CHECK_EQUAL(fired, 1);
  setting_double().set(1046.1);
  BOOST_CHECK_EQUAL(fired, 2);
  connection.disconnect();
  setting_int32().set(1047);
  BOOST_CHECK_EQUAL(fired, 2);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_connect_tracked, T, backend_types)
{
  init<T>();

  setting_int32().set(1048);

  int fired = 0;
  {
    workrave::utils::Trackable tracker;
    setting_int32().connect(tracker, [&fired](int32_t x) { fired++; });

    BOOST_CHECK_EQUAL(fired, 0);
    setting_int32().set(1049);
    BOOST_CHECK_EQUAL(fired, 1);
  }

  setting_int32().set(1050);
  BOOST_CHECK_EQUAL(fired, 1);
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_settings_group_connect_tracked, T, backend_types)
{
  init<T>();

  setting_int32().set(1051);

  int fired = 0;
  {
    workrave::utils::Trackable tracker;
    group().connect(&tracker, [&fired]() { fired++; });

    BOOST_CHECK_EQUAL(fired, 0);
    setting_int32().set(1052);
    BOOST_CHECK_EQUAL(fired, 1);
    setting_double().set(1053.1);
    BOOST_CHECK_EQUAL(fired, 2);
  }

  setting_int32().set(1054);
  BOOST_CHECK_EQUAL(fired, 2);
};

BOOST_AUTO_TEST_SUITE_END()
