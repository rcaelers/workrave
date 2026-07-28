// Copyright (C) 2026 Rob Caelers <robc@krandor.nl>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include "rpc/dbus/Error.hh"

namespace workrave::rpc::dbus
{
  // These wrappers distinguish D-Bus basic types which have the same native
  // C++ representation. They are transport-neutral; QtCodec and GioCodec
  // provide the backend-specific wire representation.
  struct ObjectPath
  {
    std::string value;
  };

  struct Signature
  {
    std::string value;
  };

  struct UnixFd
  {
    int value{-1};
  };

  template<typename T>
  struct Variant
  {
    T value;
  };

  template<typename T>
  struct IsVariant : std::false_type
  {
  };

  template<typename T>
  struct IsVariant<Variant<T>> : std::true_type
  {
    using value_type = T;
  };

  template<typename T, bool = std::is_enum_v<std::remove_cv_t<T>>>
  struct NumericRepresentation
  {
    using type = std::remove_cv_t<T>;
  };

  template<typename T>
  struct NumericRepresentation<T, true>
  {
    using type = std::underlying_type_t<std::remove_cv_t<T>>;
  };

  template<typename T>
  using NumericRepresentationT = typename NumericRepresentation<T>::type;

  template<typename>
  inline constexpr bool wire_cast_unsupported = false;

  template<typename To, typename From>
  To checked_dbus_wire_cast(const From &value)
  {
    using Target = std::remove_cv_t<To>;
    using Source = std::remove_cv_t<From>;

    if constexpr (std::is_same_v<Target, Source>)
      {
        return value;
      }
    else if constexpr (std::is_same_v<Target, ObjectPath> && std::is_same_v<Source, std::string>)
      {
        return ObjectPath{value};
      }
    else if constexpr (std::is_same_v<Target, Signature> && std::is_same_v<Source, std::string>)
      {
        return Signature{value};
      }
    else if constexpr (std::is_same_v<Target, std::string> && std::is_same_v<Source, ObjectPath>)
      {
        return value.value;
      }
    else if constexpr (std::is_same_v<Target, std::string> && std::is_same_v<Source, Signature>)
      {
        return value.value;
      }
    else if constexpr (std::is_same_v<Target, UnixFd>
                       && (std::is_integral_v<Source> || std::is_enum_v<Source>))
      {
        return UnixFd{checked_dbus_wire_cast<int>(value)};
      }
    else if constexpr ((std::is_integral_v<Target> || std::is_enum_v<Target>)
                       && std::is_same_v<Source, UnixFd>)
      {
        return checked_dbus_wire_cast<Target>(value.value);
      }
    else if constexpr (IsVariant<Target>::value)
      {
        using Value = typename IsVariant<Target>::value_type;
        return Target{checked_dbus_wire_cast<Value>(value)};
      }
    else if constexpr (IsVariant<Source>::value)
      {
        return checked_dbus_wire_cast<Target>(value.value);
      }
    else if constexpr ((std::is_arithmetic_v<Target> || std::is_enum_v<Target>)
                       && (std::is_arithmetic_v<Source> || std::is_enum_v<Source>))
      {
        using TargetValue = NumericRepresentationT<Target>;
        using SourceValue = NumericRepresentationT<Source>;
        const SourceValue source = static_cast<SourceValue>(value);

        if constexpr (std::is_integral_v<TargetValue> && std::is_integral_v<SourceValue>)
          {
            if constexpr (std::is_same_v<TargetValue, bool>)
              {
                if (source != 0 && source != 1)
                  {
                    throw Error(std::string(error_names::invalid_args),
                                "D-Bus boolean conversion requires zero or one");
                  }
              }
            else if constexpr (!std::is_same_v<SourceValue, bool>)
              {
                if (!std::in_range<TargetValue>(source))
                  {
                    throw Error(std::string(error_names::invalid_args),
                                "D-Bus numeric conversion is out of range");
                  }
              }
          }
        else
          {
            const long double number = static_cast<long double>(source);
            if constexpr (std::is_integral_v<TargetValue>)
              {
                if (!std::isfinite(number) || std::trunc(number) != number
                    || number < static_cast<long double>(std::numeric_limits<TargetValue>::lowest())
                    || number > static_cast<long double>(std::numeric_limits<TargetValue>::max()))
                  {
                    throw Error(std::string(error_names::invalid_args),
                                "D-Bus numeric conversion is not an in-range integer");
                  }
              }
            else if (std::isfinite(number)
                     && (number < -static_cast<long double>(std::numeric_limits<TargetValue>::max())
                         || number > static_cast<long double>(std::numeric_limits<TargetValue>::max())))
              {
                throw Error(std::string(error_names::invalid_args),
                            "D-Bus floating-point conversion is out of range");
              }
          }

        const TargetValue converted = static_cast<TargetValue>(source);
        if constexpr (std::is_enum_v<Target>)
          return static_cast<Target>(converted);
        else
          return converted;
      }
    else
      {
        static_assert(wire_cast_unsupported<To>, "Unsupported native/D-Bus scalar conversion");
      }
  }
} // namespace workrave::rpc::dbus
