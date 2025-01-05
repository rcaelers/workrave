// Copyright (C) 2021 Rob Caelers <robc@krandor.nl>
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

#include <vector>
#include <iostream>
#include <sstream>

#define BOOST_TEST_MODULE "workrave-utils-enum"
#include <boost/test/unit_test.hpp>

#include <boost/range/adaptor/reversed.hpp>

#include "utils/Enum.hh"
#include "utils/EnumIterator.hh"

struct Fixture
{
  Fixture() = default;
  ~Fixture() = default;

private:
};

enum class OperationMode
{
  Invalid = 1,
  Normal = 2,
  Suspended,
  Quiet,
  NotInRange
};

template<>
struct workrave::utils::enum_traits<OperationMode>
{
  static constexpr auto min = OperationMode::Normal;
  static constexpr auto max = OperationMode::Quiet;
  static constexpr auto linear = true;
};

inline std::ostream &
operator<<(std::ostream &stream, OperationMode mode)
{
  switch (mode)
    {
    case OperationMode::Normal:
      stream << "normal";
      break;
    case OperationMode::Suspended:
      stream << "suspended";
      break;
    case OperationMode::Quiet:
      stream << "quiet";
      break;
    case OperationMode::Invalid:
      stream << "invalid";
      break;
    case OperationMode::NotInRange:
      stream << "notinrange";
      break;
    }
  return stream;
}

enum OperationModeEnum
{
  Invalid = 1,
  Normal = 2,
  Suspended,
  Quiet,
  NotInRange
};

using namespace std::literals::string_view_literals;

template<>
struct workrave::utils::enum_traits<OperationModeEnum>
{
  static constexpr auto min = OperationModeEnum::Normal;
  static constexpr auto max = OperationModeEnum::Quiet;
  static constexpr auto linear = true;
  static constexpr auto invalid = OperationModeEnum::Invalid;

  static constexpr std::array<std::pair<std::string_view, OperationModeEnum>, 5> names{
    {{"normal", OperationModeEnum::Normal},
     {"suspended", OperationModeEnum::Suspended},
     {"quiet", OperationModeEnum::Quiet},
     {"invalid", OperationModeEnum::Invalid},
     {"notinrange", OperationModeEnum::NotInRange}}};
};

inline std::ostream &
operator<<(std::ostream &stream, OperationModeEnum mode)
{
  switch (mode)
    {
    case OperationModeEnum::Normal:
      stream << "normal";
      break;
    case OperationModeEnum::Suspended:
      stream << "suspended";
      break;
    case OperationModeEnum::Quiet:
      stream << "quiet";
      break;
    case OperationModeEnum::Invalid:
      stream << "invalid";
      break;
    case OperationModeEnum::NotInRange:
      stream << "notinrange";
      break;
    }
  return stream;
}

enum class OperationModeNoMinMax
{
  Normal = 2,
  Suspended,
  Quiet
};

inline std::ostream &
operator<<(std::ostream &stream, OperationModeNoMinMax mode)
{
  switch (mode)
    {
    case OperationModeNoMinMax::Normal:
      stream << "normal";
      break;
    case OperationModeNoMinMax::Suspended:
      stream << "suspended";
      break;
    case OperationModeNoMinMax::Quiet:
      stream << "quiet";
      break;
    }
  return stream;
}

enum class Kind
{
  None = 0,
  A = 1,
  B = 2,
  C = 4,
  D = 8,
};

template<>
struct workrave::utils::enum_traits<Kind>
{
  static constexpr auto flag = true;
  static constexpr auto bits = 4;

  static constexpr std::array<std::pair<std::string_view, Kind>, 5> names{
    {{"none", Kind::None}, {"A", Kind::A}, {"B", Kind::B}, {"C", Kind::C}, {"D", Kind::D}}};
};

BOOST_FIXTURE_TEST_SUITE(s, Fixture)

BOOST_AUTO_TEST_CASE(test_enum_class_mix_max)
{
  BOOST_CHECK_EQUAL(workrave::utils::enum_has_min_v<OperationMode>, true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_has_max_v<OperationMode>, true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_has_invalid_v<OperationMode>, false);
  BOOST_CHECK_EQUAL(workrave::utils::enum_has_names_v<OperationMode>, false);
  BOOST_CHECK_EQUAL(workrave::utils::enum_has_min_v<OperationModeNoMinMax>, false);
  BOOST_CHECK_EQUAL(workrave::utils::enum_has_max_v<OperationModeNoMinMax>, false);
  BOOST_CHECK_EQUAL(workrave::utils::enum_min_value<OperationMode>(), 2);
  BOOST_CHECK_EQUAL(workrave::utils::enum_max_value<OperationMode>(), 4);
  BOOST_CHECK_EQUAL(workrave::utils::enum_count<OperationMode>(), 3);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationMode>(0), false);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationMode>(1), false);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationMode>(2), true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationMode>(3), true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationMode>(4), true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationMode>(5), false);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationMode>(OperationMode::Invalid), false);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationMode>(OperationMode::Normal), true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationMode>(OperationMode::Suspended), true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationMode>(OperationMode::Quiet), true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationMode>(OperationMode::NotInRange), false);
}

BOOST_AUTO_TEST_CASE(test_enum_mix_max)
{
  BOOST_CHECK_EQUAL(workrave::utils::enum_has_min_v<OperationModeEnum>, true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_has_max_v<OperationModeEnum>, true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_has_invalid_v<OperationModeEnum>, true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_has_names_v<OperationModeEnum>, true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_min_value<OperationModeEnum>(), 2);
  BOOST_CHECK_EQUAL(workrave::utils::enum_max_value<OperationModeEnum>(), 4);
  BOOST_CHECK_EQUAL(workrave::utils::enum_count<OperationModeEnum>(), 3);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationModeEnum>(1), false);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationModeEnum>(2), true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationModeEnum>(3), true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationModeEnum>(4), true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationModeEnum>(5), false);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationModeEnum>(OperationModeEnum::Invalid), false);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationModeEnum>(OperationModeEnum::Normal), true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationModeEnum>(OperationModeEnum::Suspended), true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationModeEnum>(OperationModeEnum::Quiet), true);
  BOOST_CHECK_EQUAL(workrave::utils::enum_in_range<OperationModeEnum>(OperationModeEnum::NotInRange), false);
}

BOOST_AUTO_TEST_CASE(test_enum_class_from_string)
{
  BOOST_CHECK_EQUAL(workrave::utils::enum_from_string<OperationModeEnum>("invalid"), OperationModeEnum::Invalid);
  BOOST_CHECK_EQUAL(workrave::utils::enum_from_string<OperationModeEnum>("normal"), OperationModeEnum::Normal);
  BOOST_CHECK_EQUAL(workrave::utils::enum_from_string<OperationModeEnum>("suspended"), OperationModeEnum::Suspended);
  BOOST_CHECK_EQUAL(workrave::utils::enum_from_string<OperationModeEnum>("quiet"), OperationModeEnum::Quiet);
  BOOST_CHECK_EQUAL(workrave::utils::enum_from_string<OperationModeEnum>("notinrange"), OperationModeEnum::NotInRange);
  BOOST_CHECK_EQUAL(workrave::utils::enum_from_string<OperationModeEnum>("foo"), OperationModeEnum::Invalid);
}

BOOST_AUTO_TEST_CASE(test_enum_class_to_string)
{
  BOOST_CHECK_EQUAL(workrave::utils::enum_to_string<OperationModeEnum>(OperationModeEnum::Invalid), "invalid");
  BOOST_CHECK_EQUAL(workrave::utils::enum_to_string<OperationModeEnum>(OperationModeEnum::Normal), "normal");
  BOOST_CHECK_EQUAL(workrave::utils::enum_to_string<OperationModeEnum>(OperationModeEnum::Suspended), "suspended");
  BOOST_CHECK_EQUAL(workrave::utils::enum_to_string<OperationModeEnum>(OperationModeEnum::Quiet), "quiet");
  BOOST_CHECK_EQUAL(workrave::utils::enum_to_string<OperationModeEnum>(OperationModeEnum::NotInRange), "notinrange");
  BOOST_CHECK_EQUAL(workrave::utils::enum_to_string<OperationModeEnum>(static_cast<OperationModeEnum>(200)), "");
}

BOOST_AUTO_TEST_CASE(test_enum_class_linear)
{
  auto e = OperationMode::Normal;
  auto ee = e++;
  BOOST_CHECK_EQUAL(e, OperationMode::Suspended);
  BOOST_CHECK_EQUAL(ee, OperationMode::Normal);
  ee = ++e;
  BOOST_CHECK_EQUAL(ee, OperationMode::Quiet);
  BOOST_CHECK_EQUAL(e, OperationMode::Quiet);
}

BOOST_AUTO_TEST_CASE(test_enum_linear)
{
  auto e = OperationModeEnum::Normal;
  auto ee = e++;
  BOOST_CHECK_EQUAL(e, OperationModeEnum::Suspended);
  BOOST_CHECK_EQUAL(ee, OperationModeEnum::Normal);
  ee = ++e;
  BOOST_CHECK_EQUAL(ee, OperationModeEnum::Quiet);
  BOOST_CHECK_EQUAL(e, OperationModeEnum::Quiet);
}

BOOST_AUTO_TEST_CASE(test_enum_class_range)
{
  std::vector<OperationMode> modes;

  for (auto v: workrave::utils::enum_range<OperationMode>())
    {
      modes.push_back(v);
    }

  BOOST_CHECK_EQUAL(modes.size(), 3);
  BOOST_CHECK_EQUAL(modes[0], OperationMode::Normal);
  BOOST_CHECK_EQUAL(modes[1], OperationMode::Suspended);
  BOOST_CHECK_EQUAL(modes[2], OperationMode::Quiet);
}

BOOST_AUTO_TEST_CASE(test_enum_range)
{
  std::vector<OperationModeEnum> modes;

  for (auto v: workrave::utils::enum_range<OperationModeEnum>())
    {
      modes.push_back(v);
    }

  BOOST_CHECK_EQUAL(modes.size(), 3);
  BOOST_CHECK_EQUAL(modes[0], OperationModeEnum::Normal);
  BOOST_CHECK_EQUAL(modes[1], OperationModeEnum::Suspended);
  BOOST_CHECK_EQUAL(modes[2], OperationModeEnum::Quiet);
}

BOOST_AUTO_TEST_CASE(test_enum_reverse_range)
{
  std::vector<OperationMode> modes;

  for (auto v: boost::adaptors::reverse(workrave::utils::enum_range<OperationMode>()))
    {
      modes.push_back(v);
    }

  BOOST_CHECK_EQUAL(modes.size(), 3);
  BOOST_CHECK_EQUAL(modes[0], OperationMode::Quiet);
  BOOST_CHECK_EQUAL(modes[1], OperationMode::Suspended);
  BOOST_CHECK_EQUAL(modes[2], OperationMode::Normal);
}

BOOST_AUTO_TEST_CASE(test_enum_value_range)
{
  std::vector<int> modes;

  for (auto v: workrave::utils::enum_value_range<OperationMode>())
    {
      modes.push_back(v);
    }

  BOOST_CHECK_EQUAL(modes.size(), 3);
  BOOST_CHECK_EQUAL(modes[0], 2);
  BOOST_CHECK_EQUAL(modes[1], 3);
  BOOST_CHECK_EQUAL(modes[2], 4);
}

BOOST_AUTO_TEST_CASE(test_enum_array)
{
  workrave::utils::array<OperationMode, std::string> arr{"Normal", "Suspended", "Quiet"};

  BOOST_CHECK_EQUAL(arr.size(), 3);
  BOOST_CHECK_EQUAL(arr[0], "Normal");
  BOOST_CHECK_EQUAL(arr[1], "Suspended");
  BOOST_CHECK_EQUAL(arr[2], "Quiet");

  const workrave::utils::array<OperationMode, std::string> arrc{"Normal", "Suspended", "Quiet"};

  BOOST_CHECK_EQUAL(arrc.size(), 3);
  BOOST_CHECK_EQUAL(arrc[0], "Normal");
  BOOST_CHECK_EQUAL(arrc[1], "Suspended");
  BOOST_CHECK_EQUAL(arrc[2], "Quiet");

  BOOST_CHECK_EQUAL(arr[OperationMode::Normal], "Normal");
  BOOST_CHECK_EQUAL(arr[OperationMode::Suspended], "Suspended");
  BOOST_CHECK_EQUAL(arr[OperationMode::Quiet], "Quiet");
}

BOOST_AUTO_TEST_CASE(test_flags)
{
  auto k{Kind::A | Kind::B};
  BOOST_CHECK_EQUAL(k.get(), 3);

  k |= Kind::C;
  BOOST_CHECK_EQUAL(k.get(), 7);

  k &= Kind::C;
  BOOST_CHECK_EQUAL(k.get(), 4);

  k ^= Kind::A;
  BOOST_CHECK_EQUAL(k.get(), 5);

  k ^= Kind::A;
  BOOST_CHECK_EQUAL(k.get(), 4);

  k |= workrave::utils::Flags<Kind>{Kind::D};
  BOOST_CHECK_EQUAL(k.get(), 12);

  k &= workrave::utils::Flags<Kind>{Kind::D};
  BOOST_CHECK_EQUAL(k.get(), 8);

  k ^= workrave::utils::Flags<Kind>{Kind::A};
  BOOST_CHECK_EQUAL(k.get(), 9);

  k ^= workrave::utils::Flags<Kind>{Kind::A};
  BOOST_CHECK_EQUAL(k.get(), 8);

  workrave::utils::Flags<Kind> b{Kind::B};
  k = k | b;
  BOOST_CHECK_EQUAL(k.get(), 10);

  workrave::utils::Flags<Kind> a{Kind::A};
  k = k ^ a;
  BOOST_CHECK_EQUAL(k.get(), 11);

  k = k ^ a;
  BOOST_CHECK_EQUAL(k.get(), 10);

  workrave::utils::Flags<Kind> d{Kind::D};
  k = k & d;
  BOOST_CHECK_EQUAL(k.get(), 8);

  k = Kind::A | Kind::B | Kind::C | Kind::D;
  k &= (~(Kind::B | Kind::D));
  BOOST_CHECK_EQUAL(k.get(), 5);
  BOOST_CHECK_EQUAL(k.is_set(Kind::A), true);
  BOOST_CHECK_EQUAL(k.is_set(Kind::B), false);
  BOOST_CHECK_EQUAL(k.is_set(Kind::C), true);
  BOOST_CHECK_EQUAL(k.is_set(Kind::D), false);

  k = Kind::A & Kind::B;
  BOOST_CHECK_EQUAL(k.get(), 0);

  k = Kind::A ^ Kind::B;
  BOOST_CHECK_EQUAL(k.get(), 3);

  k = Kind::B ^ Kind::B;
  BOOST_CHECK_EQUAL(k.get(), 0);

  k.clear();
  BOOST_CHECK_EQUAL(k.get(), 0);

  k.set(9);
  BOOST_CHECK_EQUAL(k.is_set(Kind::A), true);
  BOOST_CHECK_EQUAL(k.is_set(Kind::B), false);
  BOOST_CHECK_EQUAL(k.is_set(Kind::C), false);
  BOOST_CHECK_EQUAL(k.is_set(Kind::D), true);
  BOOST_CHECK_EQUAL(k.is_set(Kind::A | Kind::D), true);
  BOOST_CHECK_EQUAL(k.is_set(Kind::B | Kind::D), false);

  k = Kind::A;
  BOOST_CHECK_EQUAL(k.get(), 1);
  BOOST_CHECK_EQUAL(k == a, true);
  BOOST_CHECK_EQUAL(k == b, false);
  BOOST_CHECK_EQUAL(k != a, false);
  BOOST_CHECK_EQUAL(k != b, true);

  k = ~Kind::A;
  BOOST_CHECK_EQUAL(k.get(), ~1);

  k = Kind::A | Kind::B | Kind::D;
  BOOST_CHECK_EQUAL(static_cast<bool>(k & (Kind::A | Kind::B)), true);
  BOOST_CHECK_EQUAL(static_cast<bool>(k & (Kind::C)), false);

  std::ostringstream oss;
  oss << k;
  BOOST_CHECK_EQUAL(oss.str(), "A,B,D");
}

BOOST_AUTO_TEST_SUITE_END()
