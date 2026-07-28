// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>
#include <utility>

#include "rpc/dbus/Error.hh"
#include "rpc/dbus/Registration.hh"
#include "rpc/dbus/WireTypes.hh"

namespace
{
  TEST(DBusRegistrationTest, releasesExactlyOnce)
  {
    int releases = 0;
    {
      workrave::rpc::dbus::Registration registration([&releases] { ++releases; });
      EXPECT_TRUE(registration);
      registration.reset();
      EXPECT_FALSE(registration);
      EXPECT_EQ(releases, 1);
    }
    EXPECT_EQ(releases, 1);
  }

  TEST(DBusRegistrationTest, moveTransfersOwnership)
  {
    int releases = 0;
    {
      workrave::rpc::dbus::Registration first([&releases] { ++releases; });
      workrave::rpc::dbus::Registration second(std::move(first));
      EXPECT_FALSE(first);
      EXPECT_TRUE(second);
    }
    EXPECT_EQ(releases, 1);
  }

  TEST(DBusRegistrationTest, moveAssignmentReleasesPreviousRegistration)
  {
    int first_releases = 0;
    int second_releases = 0;
    {
      workrave::rpc::dbus::Registration first([&first_releases] { ++first_releases; });
      workrave::rpc::dbus::Registration second([&second_releases] { ++second_releases; });
      second = std::move(first);
      EXPECT_EQ(second_releases, 1);
      EXPECT_EQ(first_releases, 0);
    }
    EXPECT_EQ(first_releases, 1);
    EXPECT_EQ(second_releases, 1);
  }

  TEST(DBusRegistrationTest, teardownDoesNotPropagateExceptions)
  {
    workrave::rpc::dbus::Registration registration([] { throw std::runtime_error("release failed"); });
    EXPECT_NO_THROW(registration.reset());
  }

  TEST(DBusErrorTest, retainsWireNameAndDiagnostic)
  {
    const workrave::rpc::dbus::Error error(std::string(workrave::rpc::dbus::error_names::invalid_args),
                                           "value is out of range");
    EXPECT_EQ(error.name(), workrave::rpc::dbus::error_names::invalid_args);
    EXPECT_STREQ(error.what(), "value is out of range");
  }

  TEST(DBusWireTypesTest, convertsEveryNumericScalarWithRangeChecking)
  {
    using workrave::rpc::dbus::checked_dbus_wire_cast;

    EXPECT_EQ(checked_dbus_wire_cast<uint8_t>(int64_t{255}), 255);
    EXPECT_EQ(checked_dbus_wire_cast<bool>(uint32_t{1}), true);
    EXPECT_EQ(checked_dbus_wire_cast<int16_t>(int64_t{-32768}), -32768);
    EXPECT_EQ(checked_dbus_wire_cast<uint16_t>(uint64_t{65535}), 65535);
    EXPECT_EQ(checked_dbus_wire_cast<int32_t>(int64_t{-2147483648LL}),
              std::numeric_limits<int32_t>::lowest());
    EXPECT_EQ(checked_dbus_wire_cast<uint32_t>(uint64_t{4294967295ULL}),
              std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(checked_dbus_wire_cast<int64_t>(int32_t{-1}), -1);
    EXPECT_EQ(checked_dbus_wire_cast<uint64_t>(uint32_t{1}), 1);
    EXPECT_DOUBLE_EQ(checked_dbus_wire_cast<double>(float{1.5F}), 1.5);

    EXPECT_THROW(checked_dbus_wire_cast<uint8_t>(int32_t{256}),
                 workrave::rpc::dbus::Error);
    EXPECT_THROW(checked_dbus_wire_cast<bool>(int32_t{2}),
                 workrave::rpc::dbus::Error);
    EXPECT_THROW(checked_dbus_wire_cast<int32_t>(double{1.5}),
                 workrave::rpc::dbus::Error);
  }

  TEST(DBusWireTypesTest, convertsDistinctStringAndFileDescriptorTypes)
  {
    using namespace workrave::rpc::dbus;

    const auto object_path = checked_dbus_wire_cast<ObjectPath>(std::string{"/org/workrave"});
    const auto signature = checked_dbus_wire_cast<Signature>(std::string{"a{sv}"});
    const auto unix_fd = checked_dbus_wire_cast<UnixFd>(int{7});
    const auto variant = checked_dbus_wire_cast<Variant<int64_t>>(int64_t{42});

    EXPECT_EQ(checked_dbus_wire_cast<std::string>(object_path), "/org/workrave");
    EXPECT_EQ(checked_dbus_wire_cast<std::string>(signature), "a{sv}");
    EXPECT_EQ(checked_dbus_wire_cast<int>(unix_fd), 7);
    EXPECT_EQ(checked_dbus_wire_cast<int64_t>(variant), 42);
  }
}
