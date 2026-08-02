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

#include <gtest/gtest.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/cfg/env.h>

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

class GlobalFixture : public ::testing::Environment
{
public:
  void SetUp() override
  {
    const auto *log_file = "workrave-config-test.log";

    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file, false);

    auto logger{std::make_shared<spdlog::logger>("workrave", file_sink)};
    spdlog::set_default_logger(logger);

    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%-5l%$] %v");
    spdlog::cfg::load_env_levels();
  }

  void TearDown() override
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
            {
              SCOPED_TRACE("Config");
              SCOPED_TRACE(::testing::Message() << "Count:" << i);
              check_func(i);
            }
            sim->current_time += 1000000;
          }
        catch (...)
          {
            std::cout << "error at:" << i << std::endl;
            throw;
          }
      }
  }

  void config_changed_notify(const std::string &key) override
  {
    EXPECT_EQ(key, expected_key);
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

::testing::Environment *const config_test_global_fixture = ::testing::AddGlobalTestEnvironment(new GlobalFixture);


using backend_types = ::testing::Types<IniConfigurator,
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

#if defined(HAVE_GSETTINGS)
using non_file_backend_types = ::testing::Types<GSettingsConfigurator>;
#endif

using file_backend_types = ::testing::Types<XmlConfigurator, IniConfigurator>;

template<typename T>
class ConfigTest : public Fixture, public ::testing::Test
{};
TYPED_TEST_SUITE(ConfigTest, backend_types);

template<typename T>
class ConfigFileTest : public Fixture, public ::testing::Test
{};
TYPED_TEST_SUITE(ConfigFileTest, file_backend_types);

#if defined(HAVE_GSETTINGS)
template<typename T>
class ConfigNonFileTest : public Fixture, public ::testing::Test
{};
TYPED_TEST_SUITE(ConfigNonFileTest, non_file_backend_types);
#endif

TYPED_TEST(ConfigTest, test_configurator_string)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  std::string value;
  ok = this->configurator->get_value("test/schema-defaults/string", value);
  EXPECT_EQ(ok, this->has_defaults);
  EXPECT_TRUE(!this->has_defaults || value == "default_string");
  EXPECT_EQ(this->configurator->has_user_value("test/schema-defaults/string"), false);

  this->configurator->set_value("test/schema-defaults/string", std::string{"string_value"});
  EXPECT_EQ(this->configurator->has_user_value("test/schema-defaults/string"), true);

  ok = this->configurator->get_value("test/schema-defaults/string", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, "string_value");

  this->configurator->set_value("test/schema-defaults/string", std::string{"other_string_value"});

  ok = this->configurator->get_value("test/schema-defaults/string", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, "other_string_value");
}

TYPED_TEST(ConfigTest, test_configurator_charstring)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  std::string value;
  ok = this->configurator->get_value("test/schema-defaults/charstring", value);
  EXPECT_EQ(ok, this->has_defaults);
  EXPECT_TRUE(!this->has_defaults || value == "default_charstring");
  EXPECT_EQ(this->configurator->has_user_value("test/schema-defaults/charstring"), false);

  this->configurator->set_value("test/schema-defaults/charstring", "charstring_value");
  EXPECT_EQ(this->configurator->has_user_value("test/schema-defaults/charstring"), true);

  ok = this->configurator->get_value("test/schema-defaults/charstring", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, "charstring_value");

  this->configurator->set_value("test/schema-defaults/charstring", "other_charstring_value");
  EXPECT_EQ(ok, true);

  ok = this->configurator->get_value("test/schema-defaults/charstring", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, "other_charstring_value");
}

TYPED_TEST(ConfigTest, test_configurator_int32)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t value;
  ok = this->configurator->get_value("test/schema-defaults/int32", value);
  EXPECT_EQ(ok, this->has_defaults);
  EXPECT_TRUE(!this->has_defaults || value == 1234);
  EXPECT_EQ(this->configurator->has_user_value("test/schema-defaults/int32"), false);

  this->configurator->set_value("test/schema-defaults/int32", 11);
  EXPECT_EQ(this->configurator->has_user_value("test/schema-defaults/int32"), true);

  ok = this->configurator->get_value("test/schema-defaults/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 11);

  this->configurator->set_value("test/schema-defaults/int32", 22);

  ok = this->configurator->get_value("test/schema-defaults/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 22);
}

TYPED_TEST(ConfigTest, test_configurator_int64)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int64_t value;
  ok = this->configurator->get_value("test/schema-defaults/int64", value);
  EXPECT_EQ(ok, this->has_defaults);
  EXPECT_TRUE(!this->has_defaults || value == INT64_C(1234));
  EXPECT_EQ(this->configurator->has_user_value("test/schema-defaults/int64"), false);

  this->configurator->set_value("test/schema-defaults/int64", INT64_C(11));
  EXPECT_EQ(this->configurator->has_user_value("test/schema-defaults/int64"), true);

  ok = this->configurator->get_value("test/schema-defaults/int64", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 11L);

  this->configurator->set_value("test/schema-defaults/int64", INT64_C(22));

  ok = this->configurator->get_value("test/schema-defaults/int64", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, INT64_C(22));
}

TYPED_TEST(ConfigTest, test_configurator_double)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  double value;
  ok = this->configurator->get_value("test/schema-defaults/double", value);
  EXPECT_EQ(ok, this->has_defaults);
  EXPECT_TRUE(!this->has_defaults || value == 12.34);
  EXPECT_EQ(this->configurator->has_user_value("test/schema-defaults/double"), false);

  this->configurator->set_value("test/schema-defaults/double", 11.11);
  EXPECT_EQ(this->configurator->has_user_value("test/schema-defaults/double"), true);

  ok = this->configurator->get_value("test/schema-defaults/double", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 11.11);

  this->configurator->set_value("test/schema-defaults/double", 22.22);

  ok = this->configurator->get_value("test/schema-defaults/double", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 22.22);
}

TYPED_TEST(ConfigTest, test_configurator_bool)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  bool value;
  ok = this->configurator->get_value("test/schema-defaults/bool", value);
  EXPECT_EQ(ok, this->has_defaults);
  EXPECT_TRUE(!this->has_defaults || value);
  EXPECT_EQ(this->configurator->has_user_value("test/schema-defaults/bool"), false);

  this->configurator->set_value("test/schema-defaults/bool", true);
  EXPECT_EQ(this->configurator->has_user_value("test/schema-defaults/bool"), true);

  ok = this->configurator->get_value("test/schema-defaults/bool", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, true);

  this->configurator->set_value("test/schema-defaults/bool", false);

  ok = this->configurator->get_value("test/schema-defaults/bool", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, false);
}

TYPED_TEST(ConfigTest, test_configurator_string_default)
{
  using T = TypeParam;
  this->template init<T>();
  std::string value;
  this->configurator->get_value_with_default("test/code-defaults/string", value, "11");
  EXPECT_EQ(value, this->has_defaults ? "default_string" : "11");

  this->configurator->set_value("test/code-defaults/string", std::string{"22"});

  this->configurator->get_value_with_default("test/code-defaults/string", value, "11");
  EXPECT_EQ(value, "22");
}

TYPED_TEST(ConfigTest, test_configurator_int32_default)
{
  using T = TypeParam;
  this->template init<T>();

  int32_t value;
  this->configurator->get_value_with_default("test/code-defaults/int32", value, 33);
  EXPECT_EQ(value, this->has_defaults ? 1234 : 33);

  this->configurator->set_value("test/code-defaults/int32", 44);

  this->configurator->get_value_with_default("test/code-defaults/int32", value, 33);
  EXPECT_EQ(value, 44);
}

TYPED_TEST(ConfigTest, test_configurator_int64_default)
{
  using T = TypeParam;
  this->template init<T>();

  int64_t value;
  this->configurator->get_value_with_default("test/code-defaults/int64", value, INT64_C(33));
  EXPECT_EQ(value, this->has_defaults ? INT64_C(1234) : INT64_C(33));

  this->configurator->set_value("test/code-defaults/int64", INT64_C(44));

  this->configurator->get_value_with_default("test/code-defaults/int64", value, INT64_C(33));
  EXPECT_EQ(value, INT64_C(44));
}

TYPED_TEST(ConfigTest, test_configurator_double_default)
{
  using T = TypeParam;
  this->template init<T>();

  double value;
  this->configurator->get_value_with_default("test/code-defaults/double", value, 33.33);
  EXPECT_EQ(value, this->has_defaults ? 12.34 : 33.33);

  this->configurator->set_value("test/code-defaults/double", 44.44);

  this->configurator->get_value_with_default("test/code-defaults/double", value, 33.33);
  EXPECT_EQ(value, 44.44);
}

TYPED_TEST(ConfigTest, test_configurator_bool_default)
{
  using T = TypeParam;
  this->template init<T>();

  bool value;
  this->configurator->get_value_with_default("test/code-defaults/bool", value, true);
  EXPECT_EQ(value, true);

  this->configurator->get_value_with_default("test/code-defaults/bool", value, false);
  EXPECT_EQ(value, this->has_defaults ? true : false);

  this->configurator->set_value("test/code-defaults/bool", true);

  this->configurator->get_value_with_default("test/code-defaults/bool", value, false);
  EXPECT_EQ(value, true);
}

TYPED_TEST(ConfigTest, test_configurator_string_wrong_type)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t ivalue;
  ok = this->configurator->get_value("test/other/string", ivalue);
  EXPECT_EQ(ok, false);

  int64_t i64value;
  ok = this->configurator->get_value("test/other/string", i64value);
  EXPECT_EQ(ok, false);

  double dvalue;
  ok = this->configurator->get_value("test/other/string", dvalue);
  EXPECT_EQ(ok, false);

  bool bvalue;
  ok = this->configurator->get_value("test/other/string", bvalue);
  EXPECT_EQ(ok, false);
}

TYPED_TEST(ConfigTest, test_configurator_int32_wrong_type)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  std::string svalue;
  ok = this->configurator->get_value("test/other/int32", svalue);
  EXPECT_EQ(ok, false);

  int64_t i64value;
  ok = this->configurator->get_value("test/other/int32", i64value);
  EXPECT_EQ(ok, false);

  double dvalue;
  ok = this->configurator->get_value("test/other/int32", dvalue);
  EXPECT_EQ(ok, false);

  bool bvalue;
  ok = this->configurator->get_value("test/other/int32", bvalue);
  EXPECT_EQ(ok, false);
}

TYPED_TEST(ConfigTest, test_configurator_int64_wrong_type)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  std::string svalue;
  ok = this->configurator->get_value("test/other/int64", svalue);
  EXPECT_EQ(ok, false);

  int32_t ivalue;
  ok = this->configurator->get_value("test/other/int64", ivalue);
  EXPECT_EQ(ok, false);

  double dvalue;
  ok = this->configurator->get_value("test/other/int64", dvalue);
  EXPECT_EQ(ok, false);

  bool bvalue;
  ok = this->configurator->get_value("test/other/int64", bvalue);
  EXPECT_EQ(ok, false);
}

TYPED_TEST(ConfigTest, test_configurator_double_wrong_type)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  std::string svalue;
  ok = this->configurator->get_value("test/other/double", svalue);
  EXPECT_EQ(ok, false);

  int32_t ivalue;
  ok = this->configurator->get_value("test/other/double", ivalue);
  EXPECT_EQ(ok, false);

  int64_t i64value;
  ok = this->configurator->get_value("test/other/double", i64value);
  EXPECT_EQ(ok, false);

  bool bvalue;
  ok = this->configurator->get_value("test/other/double", bvalue);
  EXPECT_EQ(ok, false);
}

TYPED_TEST(ConfigTest, test_configurator_bool_wrong_type)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  std::string svalue;
  ok = this->configurator->get_value("test/other/bool", svalue);
  EXPECT_EQ(ok, false);

  int32_t ivalue;
  ok = this->configurator->get_value("test/other/bool", ivalue);
  EXPECT_EQ(ok, false);

  int64_t i64value;
  ok = this->configurator->get_value("test/other/bool", i64value);
  EXPECT_EQ(ok, false);

  double dvalue;
  ok = this->configurator->get_value("test/other/bool", dvalue);
  EXPECT_EQ(ok, false);
}

TYPED_TEST(ConfigTest, test_configurator_bad_key)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  std::string value;
  ok = this->configurator->get_value("", value);
  // EXPECT_EQ(ok, false); TODO: check for IniConfigurator

  ok = this->configurator->get_value("/", value);
  // EXPECT_EQ(ok, false); TODO: check for IniConfigurator

  ok = this->configurator->get_value(" ", value);
  EXPECT_EQ(ok, false);

  ok = this->configurator->get_value("lskjflskd", value);
  EXPECT_EQ(ok, false);
}

TYPED_TEST(ConfigTest, test_configurator_listener_one)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  ok = this->configurator->add_listener("test/other/int32", this);
  EXPECT_EQ(ok, true);

  this->expected_key = "test/other/int32";
  this->configurator->set_value("test/other/int32", 1001);
  EXPECT_EQ(this->config_changed_count, 1);

  ok = this->configurator->remove_listener("test", this);
  EXPECT_EQ(ok, false);

  this->configurator->set_value("test/other/int32", 1002);
  EXPECT_EQ(this->config_changed_count, 2);

  this->configurator->set_value("test/other/int32", 1002);
  EXPECT_EQ(this->config_changed_count, 2);

  this->configurator->set_value("test/other/double", 1002.1002);
  EXPECT_EQ(this->config_changed_count, 2);

  ok = this->configurator->remove_listener("test/other/int32", this);
  EXPECT_EQ(ok, true);

  this->configurator->set_value("test/other/int32", 1003);
  EXPECT_EQ(this->config_changed_count, 2);

  ok = this->configurator->add_listener("test/other/int32/", this);

  this->configurator->set_value("test/other/int32", 1001);
  EXPECT_EQ(this->config_changed_count, 3);

  ok = this->configurator->remove_listener("test/other/int32", this);
  EXPECT_EQ(ok, true);

  this->configurator->set_value("test/other/int32", 1004);
  EXPECT_EQ(this->config_changed_count, 3);
}

TYPED_TEST(ConfigTest, test_configurator_listener_section)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  ok = this->configurator->add_listener("test/other/", this);
  EXPECT_EQ(ok, true);

  this->expected_key = "test/other/int32";
  this->configurator->set_value("test/other/int32", 1005);
  EXPECT_EQ(this->config_changed_count, 1);

  this->expected_key = "test/other/double";
  this->configurator->set_value("test/other/double", 1005.55);
  EXPECT_EQ(this->config_changed_count, 2);
}

TYPED_TEST(ConfigTest, test_configurator_listener_multiple)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  ok = this->configurator->add_listener("test/other/int32", this);
  EXPECT_EQ(ok, true);
  ok = this->configurator->add_listener("test/other/double/", this);
  EXPECT_EQ(ok, true);

  this->expected_key = "test/other/int32";
  this->configurator->set_value("test/other/int32", 1006);
  EXPECT_EQ(this->config_changed_count, 1);

  this->expected_key = "test/other/double";
  this->configurator->set_value("test/other/double", 1006.1006);
  EXPECT_EQ(this->config_changed_count, 2);

  ok = this->configurator->remove_listener("test/other/double", this);
  ok = this->configurator->remove_listener("test/other/int32", this);
}

TYPED_TEST(ConfigTest, test_configurator_listener_add_remove)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  ok = this->configurator->add_listener("test/other/int32", this);
  EXPECT_EQ(ok, true);
  ok = this->configurator->add_listener("test/other/int64", this);
  EXPECT_EQ(ok, true);
  ok = this->configurator->add_listener("test/other/double/", this);
  EXPECT_EQ(ok, true);
  ok = this->configurator->add_listener("test/other/double/", (IConfiguratorListener *)0xdeadbeef);
  EXPECT_EQ(ok, true);
  ok = this->configurator->add_listener("test/other/double/", (IConfiguratorListener *)0xdeadbeef);
  EXPECT_EQ(ok, false);
  ok = this->configurator->add_listener("test/other/string", (IConfiguratorListener *)0xbaaaaaad);
  EXPECT_EQ(ok, true);

  ok = this->configurator->remove_listener((IConfiguratorListener *)0xbaaaaaad);
  EXPECT_EQ(ok, true);
  ok = this->configurator->remove_listener((IConfiguratorListener *)0xbaadf00d);
  EXPECT_EQ(ok, false);

  ok = this->configurator->remove_listener("test/other/double", this);
  EXPECT_EQ(ok, true);
  ok = this->configurator->remove_listener("test/other/double", (IConfiguratorListener *)0xdeadbeef);
  EXPECT_EQ(ok, true);
  ok = this->configurator->remove_listener("test/other/double", (IConfiguratorListener *)0xdeadbeef);
  EXPECT_EQ(ok, false);
  ok = this->configurator->remove_listener("test/other/int32", (IConfiguratorListener *)0xdeadbeef);
  EXPECT_EQ(ok, false);
}

TYPED_TEST(ConfigTest, test_configurator_leading_slash)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t value;
  this->configurator->set_value("/test/other/int32", 1007);

  ok = this->configurator->get_value("/test/other/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1007);

  this->configurator->set_value("/test/other/int32", 1008);

  ok = this->configurator->get_value("/test/other/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1008);
}

TYPED_TEST(ConfigTest, test_configurator_trailing_slash)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t value;
  this->configurator->set_value("test/other/int32/", 1009);

  ok = this->configurator->get_value("test/other/int32/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1009);

  this->configurator->set_value("test/other/int32/", 1010);

  ok = this->configurator->get_value("test/other/int32/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1010);
}

TYPED_TEST(ConfigTest, test_configurator_leading_trailing_slash)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t value;
  this->configurator->set_value("/test/other/int32/", 1011);

  ok = this->configurator->get_value("/test/other/int32/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1011);

  this->configurator->set_value("/test/other/int32/", 1012);

  ok = this->configurator->get_value("/test/other/int32/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1012);
}

TYPED_TEST(ConfigTest, test_configurator_delay)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t value;
  this->configurator->set_value("test/other/int32", 1013);

  ok = this->configurator->get_value("test/other/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1013);

  this->configurator->set_delay("test/other/int32", 5);

  ok = this->configurator->add_listener("test/other/int32", this);
  EXPECT_EQ(ok, true);

  this->configurator->set_value("/test/other/int32/", 1014);
  EXPECT_EQ(this->config_changed_count, 0);

  this->expected_key = "test/other/int32";
  ok = this->configurator->get_value("test/other/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1014);

  this->tick(6, [](int32_t c) {});
  EXPECT_EQ(this->config_changed_count, 1);

  ok = this->configurator->remove_listener(this);
  EXPECT_EQ(ok, true);
}

TYPED_TEST(ConfigTest, test_configurator_delay_repeat)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t value;
  this->configurator->set_value("test/other/int32", 1015);

  ok = this->configurator->get_value("test/other/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1015);

  this->configurator->set_delay("test/other/int32", 5);
  this->configurator->set_delay("test/other/int32", 10);

  ok = this->configurator->add_listener("test/other/int32", this);
  EXPECT_EQ(ok, true);

  this->configurator->set_value("/test/other/int32/", 1016);
  EXPECT_EQ(this->config_changed_count, 0);

  this->expected_key = "test/other/int32";
  ok = this->configurator->get_value("test/other/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1016);

  this->tick(9, [](int32_t c) {});
  EXPECT_EQ(this->config_changed_count, 0);
  this->tick(2, [](int32_t c) {});
  EXPECT_EQ(this->config_changed_count, 1);

  ok = this->configurator->remove_listener(this);
  EXPECT_EQ(ok, true);
}

TYPED_TEST(ConfigFileTest, test_configurator_delay_save_load)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  this->configurator->load("temp-save");

  int32_t value;
  this->configurator->set_value("test/other/int32", 1017);

  ok = this->configurator->get_value("test/other/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1017);

  this->configurator->set_delay("test/other/int32", 5);

  ok = this->configurator->add_listener("test/other/int32", this);
  EXPECT_EQ(ok, true);

  this->configurator->save();

  this->configurator->set_value("/test/other/int32/", 1018);
  EXPECT_EQ(this->config_changed_count, 0);

  this->expected_key = "test/other/int32";
  ok = this->configurator->get_value("test/other/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1018);

  this->tick(6, [](int32_t c) {});
  EXPECT_EQ(this->config_changed_count, 1);

  this->configurator->load("temp-save");

  ok = this->configurator->get_value("test/other/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1017);

  ok = this->configurator->remove_listener(this);
  EXPECT_EQ(ok, true);
}

TYPED_TEST(ConfigTest, test_configurator_delay_immediate)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t value;

  this->configurator->set_delay("test/other/int32", 5);

  this->configurator->set_value("test/other/int32", 1019, CONFIG_FLAG_IMMEDIATE);

  ok = this->configurator->get_value("/test/other/int32/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1019);

  this->configurator->set_value("test/other/double", 1019.1019, CONFIG_FLAG_IMMEDIATE);

  double dv;
  ok = this->configurator->get_value("/test/other/double/", dv);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(dv, 1019.1019);
}

// BOOST_AUTO_TEST_CASE_TEMPLATE(test_configurator_delay_invalid, T, backend_types)
// {
//   init<T>();

//   bool ok { false };

//   int32_t value;

//   configurator->set_delay("test/other/invalid", 5);

//   ok = configurator->set_value("test/other/invalid", 89);
//   EXPECT_EQ(ok, true);

//   ok = configurator->set_value("test/other/invalid", 88.1, CONFIG_FLAG_IMMEDIATE);
//   EXPECT_EQ(ok, true);
// }

TYPED_TEST(ConfigTest, test_configurator_delay_same_value)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t value;
  this->configurator->set_value("test/other/int32", 1020);

  ok = this->configurator->get_value("test/other/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1020);

  this->configurator->set_delay("test/other/int32", 5);

  this->configurator->set_value("/test/other/int32/", 1021);

  this->configurator->set_value("/test/other/int32/", 1020);

  this->tick(6, [](int32_t c) {});

  ok = this->configurator->get_value("test/other/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1020);
}

TYPED_TEST(ConfigTest, test_configurator_delay_initial_value)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t value;
  ok = this->configurator->get_value("test/other/delay-initial", value);
  EXPECT_EQ(ok, this->has_defaults);

  this->configurator->set_delay("test/other/delay-initial", 5);

  this->configurator->set_value("/test/other/delay-initial/", 1022);

  this->tick(6, [](int32_t c) {});

  ok = this->configurator->get_value("test/other/delay-initial", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1022);
}

TYPED_TEST(ConfigTest, test_configurator_remove)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t value;
  ok = this->configurator->get_value("/test/other/int32/", value);
  EXPECT_EQ(ok, this->has_defaults);

  this->configurator->set_value("/test/other/int32/", 1023);

  ok = this->configurator->get_value("/test/other/int32/", value);
  EXPECT_EQ(ok, true);

  this->configurator->remove_key("/test/other/int32/");

  ok = this->configurator->get_value("/test/other/int32/", value);
  EXPECT_EQ(ok, this->has_defaults);
  if (this->has_defaults)
    {
      EXPECT_EQ(value, 1234);
    }
}

TYPED_TEST(ConfigTest, test_configurator_rename_int32_t)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t value;
  ok = this->configurator->get_value("/test/other/int32/", value);
  EXPECT_EQ(ok, this->has_defaults);

  ok = this->configurator->get_value("/test/other/int32_2/", value);
  EXPECT_EQ(ok, this->has_defaults);

  this->configurator->set_value("/test/other/int32/", 1024);

  ok = this->configurator->get_value("/test/other/int32/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1024);

  this->configurator->rename_key("/test/other/int32/", "/test/other/int32_2/");

  ok = this->configurator->get_value("/test/other/int32_2/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1024);

  ok = this->configurator->get_value("/test/other/int32/", value);
  EXPECT_EQ(ok, this->has_defaults);
}

TYPED_TEST(ConfigTest, test_configurator_rename_int64_t)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int64_t value;
  ok = this->configurator->get_value("/test/other/int64/", value);
  EXPECT_EQ(ok, this->has_defaults);

  ok = this->configurator->get_value("/test/other/int64-2/", value);
  EXPECT_EQ(ok, this->has_defaults);

  this->configurator->set_value("/test/other/int64/", INT64_C(1024));

  ok = this->configurator->get_value("/test/other/int64/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, INT64_C(1024));

  this->configurator->rename_key("/test/other/int64/", "/test/other/int64-2/");

  ok = this->configurator->get_value("/test/other/int64-2/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, INT64_C(1024));

  ok = this->configurator->get_value("/test/other/int64/", value);
  EXPECT_EQ(ok, this->has_defaults);
}

TYPED_TEST(ConfigTest, test_configurator_rename_bool)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  bool value;
  ok = this->configurator->get_value("/test/other/bool/", value);
  EXPECT_EQ(ok, this->has_defaults);

  ok = this->configurator->get_value("/test/other/bool2/", value);
  EXPECT_EQ(ok, this->has_defaults);

  this->configurator->set_value("/test/other/bool/", true);

  ok = this->configurator->get_value("/test/other/bool/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, true);

  this->configurator->rename_key("/test/other/bool/", "/test/other/bool2/");

  ok = this->configurator->get_value("/test/other/bool2/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, true);

  ok = this->configurator->get_value("/test/other/bool/", value);
  EXPECT_EQ(ok, this->has_defaults);
}

TYPED_TEST(ConfigTest, test_configurator_rename_double)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  double value;
  ok = this->configurator->get_value("/test/other/double/", value);
  EXPECT_EQ(ok, this->has_defaults);

  ok = this->configurator->get_value("/test/other/double2/", value);
  EXPECT_EQ(ok, this->has_defaults);

  this->configurator->set_value("/test/other/double/", 1025.1025);

  ok = this->configurator->get_value("/test/other/double/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1025.1025);

  this->configurator->rename_key("/test/other/double/", "/test/other/double2/");

  ok = this->configurator->get_value("/test/other/double2/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1025.1025);

  ok = this->configurator->get_value("/test/other/double/", value);
  EXPECT_EQ(ok, this->has_defaults);
}

TYPED_TEST(ConfigTest, test_configurator_rename_string)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  std::string value;
  ok = this->configurator->get_value("/test/other/string/", value);
  EXPECT_EQ(ok, this->has_defaults);

  ok = this->configurator->get_value("/test/other/string2/", value);
  EXPECT_EQ(ok, this->has_defaults);

  this->configurator->set_value("/test/other/string/", "27");

  ok = this->configurator->get_value("/test/other/string/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, "27");

  this->configurator->rename_key("/test/other/string/", "/test/other/string2/");

  ok = this->configurator->get_value("/test/other/string2/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, "27");

  ok = this->configurator->get_value("/test/other/string/", value);
  EXPECT_EQ(ok, this->has_defaults);
}

TYPED_TEST(ConfigTest, test_configurator_rename_exists)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t value;
  ok = this->configurator->get_value("/test/other/int32/", value);
  EXPECT_EQ(ok, this->has_defaults);

  ok = this->configurator->get_value("/test/other/int32-2/", value);
  EXPECT_EQ(ok, this->has_defaults);

  this->configurator->set_value("/test/other/int32/", 1026);

  this->configurator->set_value("/test/other/int32-2/", 1027);

  this->configurator->rename_key("/test/other/int32/", "/test/other/int32-2/");
  // FIXME: check logging EXPECT_EQ(ok, false);

  ok = this->configurator->get_value("/test/other/int32-2/", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1027);

  ok = this->configurator->get_value("/test/other/int32/", value);
  EXPECT_EQ(ok, !this->can_remove);
}

TYPED_TEST(ConfigTest, test_configurator_initial)
{
  using T = TypeParam;
  this->template init<T>();

  bool ok{false};

  int32_t value;
  ok = this->configurator->get_value("/test/other/initial", value);
  EXPECT_EQ(ok, this->has_defaults);

  this->configurator->set_value("/test/other/initial", 1028, CONFIG_FLAG_INITIAL);

  ok = this->configurator->get_value("/test/other/initial", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, this->has_defaults ? 1234 : 1028);

  this->configurator->set_value("/test/other/initial", 1029, CONFIG_FLAG_INITIAL);

  ok = this->configurator->get_value("/test/other/initial", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, this->has_defaults ? 1234 : 1028);
}

TYPED_TEST(ConfigFileTest, test_configurator_save_load)
{
  using T = TypeParam;
  this->template init<T>();

  this->configurator->load("temp-save");

  this->configurator->set_value("/test/other/string", "1030");
  this->configurator->set_value("/test/other/int32", 1030);
  this->configurator->set_value("/test/other/double", 1030.1030);
  this->configurator->set_value("/test/other/bool", true);

  this->configurator->save();

  this->configurator->set_value("/test/other/string", "1031");
  this->configurator->set_value("/test/other/int32", 1031);
  this->configurator->set_value("/test/other/double", 1031.1031);
  this->configurator->set_value("/test/other/bool", false);

  this->configurator->load("temp-save");

  std::string svalue;
  bool ok = this->configurator->get_value("test/other/string", svalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(svalue, "1030");

  int32_t ivalue;
  ok = this->configurator->get_value("test/other/int32", ivalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(ivalue, 1030);

  double dvalue;
  ok = this->configurator->get_value("test/other/double", dvalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(dvalue, 1030.1030);

  bool bvalue;
  ok = this->configurator->get_value("test/other/bool", bvalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(bvalue, true);

  this->configurator->set_value("/test/other/string", "1031");
  this->configurator->set_value("/test/other/int32", 1031);
  this->configurator->set_value("/test/other/double", 1031.1031);
  this->configurator->set_value("/test/other/bool", false);

  this->configurator->save();

  this->configurator->set_value("/test/other/string", "1032");
  this->configurator->set_value("/test/other/int32", 1032);
  this->configurator->set_value("/test/other/double", 1032.1032);
  this->configurator->set_value("/test/other/bool", true);

  this->configurator->load("temp-save");

  ok = this->configurator->get_value("test/other/string", svalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(svalue, "1031");

  ok = this->configurator->get_value("test/other/int32", ivalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(ivalue, 1031);

  ok = this->configurator->get_value("test/other/double", dvalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(dvalue, 1031.1031);

  ok = this->configurator->get_value("test/other/bool", bvalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(bvalue, false);
}

#if defined(HAVE_GSETTINGS)
TYPED_TEST(ConfigNonFileTest, test_configurator_dummy_save_load)
{
  using T = TypeParam;
  this->template init<T>();

  this->configurator->load("temp-save");

  this->configurator->set_value("/test/other/string", "1033");
  this->configurator->set_value("/test/other/int32", 1033);
  this->configurator->set_value("/test/other/double", 1033.1033);
  this->configurator->set_value("/test/other/bool", true);

  this->configurator->save();

  this->configurator->set_value("/test/other/string", "1034");
  this->configurator->set_value("/test/other/int32", 1034);
  this->configurator->set_value("/test/other/double", 1034.1034);
  this->configurator->set_value("/test/other/bool", false);

  this->configurator->load("temp-save");

  std::string svalue;
  bool ok = this->configurator->get_value("test/other/string", svalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(svalue, "1034");

  int32_t ivalue;
  ok = this->configurator->get_value("test/other/int32", ivalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(ivalue, 1034);

  double dvalue;
  ok = this->configurator->get_value("test/other/double", dvalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(dvalue, 1034.1034);

  bool bvalue;
  ok = this->configurator->get_value("test/other/bool", bvalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(bvalue, false);

  this->configurator->set_value("/test/other/string", "1033");
  this->configurator->set_value("/test/other/int32", 1033);
  this->configurator->set_value("/test/other/double", 1033.1033);
  this->configurator->set_value("/test/other/bool", true);

  this->configurator->save();

  this->configurator->set_value("/test/other/string", "1034");
  this->configurator->set_value("/test/other/int32", 1034);
  this->configurator->set_value("/test/other/double", 1034.1034);
  this->configurator->set_value("/test/other/bool", false);

  this->configurator->load("temp-save");

  ok = this->configurator->get_value("test/other/string", svalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(svalue, "1034");

  ok = this->configurator->get_value("test/other/int32", ivalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(ivalue, 1034);

  ok = this->configurator->get_value("test/other/double", dvalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(dvalue, 1034.1034);

  ok = this->configurator->get_value("test/other/bool", bvalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(bvalue, false);
}
#endif

TYPED_TEST(ConfigTest, test_settings_int32)
{
  using T = TypeParam;
  this->template init<T>();

  this->setting_int32().set(1035);

  int32_t value;
  bool ok = this->configurator->get_value("test/settings/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1035);
  EXPECT_EQ(this->setting_int32()(), 1035);
  EXPECT_EQ(this->setting_int32().get(), 1035);
};

TYPED_TEST(ConfigTest, test_settings_int64)
{
  using T = TypeParam;
  this->template init<T>();

  this->setting_int64().set(INT64_C(1035));

  int64_t value;
  bool ok = this->configurator->get_value("test/settings/int64", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, INT64_C(1035));
  EXPECT_EQ(this->setting_int64()(), INT64_C(1035));
  EXPECT_EQ(this->setting_int64().get(), INT64_C(1035));
};

TYPED_TEST(ConfigTest, test_settings_double)
{
  using T = TypeParam;
  this->template init<T>();

  this->setting_double().set(1036.46);

  double value;
  bool ok = this->configurator->get_value("test/settings/double", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1036.46);
  EXPECT_EQ(this->setting_double()(), 1036.46);
  EXPECT_EQ(this->setting_double().get(), 1036.46);
};

TYPED_TEST(ConfigTest, test_settings_bool)
{
  using T = TypeParam;
  this->template init<T>();

  this->setting_bool().set(true);

  bool value;
  bool ok = this->configurator->get_value("test/settings/bool", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, true);
  EXPECT_EQ(this->setting_bool()(), true);
  EXPECT_EQ(this->setting_bool().get(), true);

  this->setting_bool().set(false);

  ok = this->configurator->get_value("test/settings/bool", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, false);
  EXPECT_EQ(this->setting_bool()(), false);
  EXPECT_EQ(this->setting_bool().get(), false);
};

TYPED_TEST(ConfigTest, test_settings_enum)
{
  using T = TypeParam;
  this->template init<T>();

  this->setting_mode().set(Mode::Mode1);

  int32_t ivalue;
  bool ok = this->configurator->get_value("test/settings/mode", ivalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(ivalue, (int32_t)Mode::Mode1);
  EXPECT_EQ(this->setting_mode()(), Mode::Mode1);
  EXPECT_EQ(this->setting_mode().get(), Mode::Mode1);

  this->setting_mode().set(Mode::Mode2);

  ok = this->configurator->get_value("test/settings/mode", ivalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(ivalue, (int32_t)Mode::Mode2);
  EXPECT_EQ(this->setting_mode()(), Mode::Mode2);
  EXPECT_EQ(this->setting_mode().get(), Mode::Mode2);
};

TYPED_TEST(ConfigTest, test_settings_enum_trait_int)
{
  using T = TypeParam;
  this->template init<T>();

  this->setting_extendedmode_int().set(ExtendedMode::Mode1);

  int32_t ivalue;
  bool ok = this->configurator->get_value("test/settings/extendedmode_int", ivalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(ivalue, (int32_t)ExtendedMode::Mode1);
  EXPECT_EQ(this->setting_extendedmode_int()(), ExtendedMode::Mode1);
  EXPECT_EQ(this->setting_extendedmode_int().get(), ExtendedMode::Mode1);

  this->setting_extendedmode_int().set(ExtendedMode::Mode2);

  ok = this->configurator->get_value("test/settings/extendedmode_int", ivalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(ivalue, (int32_t)ExtendedMode::Mode2);
  EXPECT_EQ(this->setting_extendedmode_int()(), ExtendedMode::Mode2);
  EXPECT_EQ(this->setting_extendedmode_int().get(), ExtendedMode::Mode2);
};

TYPED_TEST(ConfigTest, test_settings_enum_trait_string)
{
  using T = TypeParam;
  this->template init<T>();

  this->setting_extendedmode_string().set(ExtendedMode::Mode1);

  std::string svalue;
  bool ok = this->configurator->get_value("test/settings/extendedmode_string", svalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(svalue, "mode1");
  EXPECT_EQ(this->setting_extendedmode_string()(), ExtendedMode::Mode1);
  EXPECT_EQ(this->setting_extendedmode_string().get(), ExtendedMode::Mode1);

  this->setting_extendedmode_string().set(ExtendedMode::Mode2);

  ok = this->configurator->get_value("test/settings/extendedmode_string", svalue);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(svalue, "mode2");
  EXPECT_EQ(this->setting_extendedmode_string()(), ExtendedMode::Mode2);
  EXPECT_EQ(this->setting_extendedmode_string().get(), ExtendedMode::Mode2);
};

TYPED_TEST(ConfigTest, test_settings_minutes)
{
  using T = TypeParam;
  this->template init<T>();

  std::chrono::minutes m{28};

  this->setting_minutes().set(m);

  int64_t value;
  bool ok = this->configurator->get_value("test/settings/minutes", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, INT64_C(28));
  EXPECT_TRUE(this->setting_minutes()() == m);
  EXPECT_TRUE(this->setting_minutes().get() == m);
};

TYPED_TEST(ConfigTest, test_settings_hours)
{
  using T = TypeParam;
  this->template init<T>();

  std::chrono::hours h{54};

  this->setting_hours().set(h);

  int32_t value;
  bool ok = this->configurator->get_value("test/settings/hours", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 54);
  EXPECT_TRUE(this->setting_hours()() == h);
  EXPECT_TRUE(this->setting_hours().get() == h);
};

TYPED_TEST(ConfigTest, test_settings_time)
{
  using T = TypeParam;
  this->template init<T>();

  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

  this->setting_time().set(now);
  int64_t now64 = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

  int64_t value;
  bool ok = this->configurator->get_value("test/settings/time", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, now64);
  EXPECT_TRUE(std::chrono::duration_cast<std::chrono::seconds>(this->setting_time()().time_since_epoch()).count() == now64);
  EXPECT_TRUE(std::chrono::duration_cast<std::chrono::seconds>(this->setting_time().get().time_since_epoch()).count() == now64);
};

TYPED_TEST(ConfigTest, test_settings_vector_int32)
{
  using T = TypeParam;
  this->template init<T>();

  std::vector<int> values{4, 8, 15, 16, 23, 42};

  this->setting_vector_int32().set(values);

  std::string value;
  bool ok = this->configurator->get_value("test/settings/vint32", value);

  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, "4;8;15;16;23;42");
  EXPECT_TRUE(this->setting_vector_int32()() == values);
  EXPECT_TRUE(this->setting_vector_int32().get() == values);
};

TYPED_TEST(ConfigTest, test_settings_vector_duration)
{
  using T = TypeParam;
  this->template init<T>();

  std::vector<std::chrono::minutes> values{std::chrono::minutes(0),
                                           std::chrono::minutes(4),
                                           std::chrono::minutes(8),
                                           std::chrono::minutes(15),
                                           std::chrono::minutes(16),
                                           std::chrono::minutes(23),
                                           std::chrono::minutes(42)};

  this->setting_vector_duration().set(values);

  std::string value;
  bool ok = this->configurator->get_value("test/settings/vduration", value);

  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, "0;4;8;15;16;23;42");
  EXPECT_TRUE(this->setting_vector_duration()() == values);
  EXPECT_TRUE(this->setting_vector_duration().get() == values);
};

TYPED_TEST(ConfigTest, test_settings_vector_string)
{
  using T = TypeParam;
  this->template init<T>();

  std::vector<std::string> values{"hydra", "arrow", "swan", "flame", "pearl"};

  this->setting_vector_string().set(values);

  std::string value;
  bool ok = this->configurator->get_value("test/settings/vstring", value);

  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, "hydra;arrow;swan;flame;pearl");
  EXPECT_TRUE(this->setting_vector_string()() == values);
  EXPECT_TRUE(this->setting_vector_string().get() == values);
};

TYPED_TEST(ConfigTest, test_settings_int32_default)
{
  using T = TypeParam;
  this->template init<T>();

  int32_t value;
  bool ok = this->configurator->get_value("test/settings/default/int32", value);
  EXPECT_EQ(ok, this->has_defaults);
  EXPECT_EQ(this->setting_int32_default()(), this->has_defaults ? 1234 : 8888);
  EXPECT_EQ(this->setting_int32_default().get(), this->has_defaults ? 1234 : 8888);

  this->setting_int32_default().set(1035);

  ok = this->configurator->get_value("test/settings/default/int32", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1035);
  EXPECT_EQ(this->setting_int32_default()(), 1035);
  EXPECT_EQ(this->setting_int32_default().get(), 1035);
};

TYPED_TEST(ConfigTest, test_settings_int64_default)
{
  using T = TypeParam;
  this->template init<T>();

  int64_t value;
  bool ok = this->configurator->get_value("test/settings/default/int64", value);
  EXPECT_EQ(ok, this->has_defaults);
  EXPECT_EQ(this->setting_int64_default()(), this->has_defaults ? INT64_C(1234) : INT64_C(8888));
  EXPECT_EQ(this->setting_int64_default().get(), this->has_defaults ? INT64_C(1234) : INT64_C(8888));

  this->setting_int64_default().set(INT64_C(1035));

  ok = this->configurator->get_value("test/settings/default/int64", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, INT64_C(1035));
  EXPECT_EQ(this->setting_int64_default()(), INT64_C(1035));
  EXPECT_EQ(this->setting_int64_default().get(), INT64_C(1035));
};

TYPED_TEST(ConfigTest, test_settings_double_default)
{
  using T = TypeParam;
  this->template init<T>();

  double value;
  bool ok = this->configurator->get_value("test/settings/default/double", value);
  EXPECT_EQ(ok, this->has_defaults);
  EXPECT_EQ(this->setting_double_default()(), this->has_defaults ? 12.34 : 88.88);
  EXPECT_EQ(this->setting_double_default().get(), this->has_defaults ? 12.34 : 88.88);

  this->setting_double_default().set(1036.46);

  ok = this->configurator->get_value("test/settings/default/double", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, 1036.46);
  EXPECT_EQ(this->setting_double_default()(), 1036.46);
  EXPECT_EQ(this->setting_double_default().get(), 1036.46);
};

TYPED_TEST(ConfigTest, test_settings_bool_default)
{
  using T = TypeParam;
  this->template init<T>();

  bool value;
  bool ok = this->configurator->get_value("test/settings/default/bool", value);
  EXPECT_EQ(ok, this->has_defaults);
  EXPECT_EQ(this->setting_bool_default()(), true);
  EXPECT_EQ(this->setting_bool_default().get(), true);

  this->setting_bool_default().set(false);

  ok = this->configurator->get_value("test/settings/default/bool", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, false);
  EXPECT_EQ(this->setting_bool_default()(), false);
  EXPECT_EQ(this->setting_bool_default().get(), false);
};

TYPED_TEST(ConfigTest, test_settings_string_default)
{
  using T = TypeParam;
  this->template init<T>();

  std::string value;
  bool ok = this->configurator->get_value("test/settings/default/string", value);
  EXPECT_EQ(ok, this->has_defaults);
  EXPECT_EQ(this->setting_string_default()(), this->has_defaults ? "default_string" : "8888");
  EXPECT_EQ(this->setting_string_default().get(), this->has_defaults ? "default_string" : "8888");

  this->setting_string_default().set("1037");

  ok = this->configurator->get_value("test/settings/default/string", value);
  EXPECT_EQ(ok, true);
  EXPECT_EQ(value, "1037");
  EXPECT_EQ(this->setting_string_default()(), "1037");
  EXPECT_EQ(this->setting_string_default().get(), "1037");
};

TYPED_TEST(ConfigTest, test_settings_connect)
{
  using T = TypeParam;
  this->template init<T>();

  this->setting_int32().set(1038);

  int fired = 0;
  auto connection = this->setting_int32().connect(this, [&fired](int32_t value) {
    EXPECT_EQ(value, 1039);
    fired++;
  });

  EXPECT_EQ(fired, 0);
  this->setting_int32().set(1039);
  EXPECT_EQ(fired, 1);
  connection.disconnect();
  this->setting_int32().set(1040);
  EXPECT_EQ(fired, 1);
};

TYPED_TEST(ConfigTest, test_settings_attached)
{
  using T = TypeParam;
  this->template init<T>();

  this->setting_int32().set(1041);

  int fired = 0;
  auto connection = this->setting_int32().attach(this, [&fired](int32_t value) {
    EXPECT_EQ(value, fired == 0 ? 1041 : 1042);
    fired++;
  });

  EXPECT_EQ(fired, 1);
  this->setting_int32().set(1042);
  EXPECT_EQ(fired, 2);
  connection.disconnect();
  this->setting_int32().set(1043);
  EXPECT_EQ(fired, 2);
};

TYPED_TEST(ConfigTest, test_settings_group_connect)
{
  using T = TypeParam;
  this->template init<T>();

  this->setting_int32().set(1044);

  int fired = 0;
  auto connection = this->group().connect(this, [&fired]() { fired++; });

  EXPECT_EQ(fired, 0);
  this->setting_int32().set(1045);
  EXPECT_EQ(fired, 1);
  this->setting_double().set(1046.1);
  EXPECT_EQ(fired, 2);
  connection.disconnect();
  this->setting_int32().set(1047);
  EXPECT_EQ(fired, 2);
};

TYPED_TEST(ConfigTest, test_settings_connect_tracked)
{
  using T = TypeParam;
  this->template init<T>();

  this->setting_int32().set(1048);

  int fired = 0;
  {
    workrave::utils::Trackable tracker;
    this->setting_int32().connect(tracker, [&fired](int32_t x) { fired++; });

    EXPECT_EQ(fired, 0);
    this->setting_int32().set(1049);
    EXPECT_EQ(fired, 1);
  }

  this->setting_int32().set(1050);
  EXPECT_EQ(fired, 1);
};

TYPED_TEST(ConfigTest, test_settings_group_connect_tracked)
{
  using T = TypeParam;
  this->template init<T>();

  this->setting_int32().set(1051);

  int fired = 0;
  {
    workrave::utils::Trackable tracker;
    this->group().connect(&tracker, [&fired]() { fired++; });

    EXPECT_EQ(fired, 0);
    this->setting_int32().set(1052);
    EXPECT_EQ(fired, 1);
    this->setting_double().set(1053.1);
    EXPECT_EQ(fired, 2);
  }

  this->setting_int32().set(1054);
  EXPECT_EQ(fired, 2);
};

