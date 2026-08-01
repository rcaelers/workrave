// Copyright (C) 2026 Rob Caelers
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

#ifndef WORKRAVE_LIBS_STATS_STATVALUE_HH
#define WORKRAVE_LIBS_STATS_STATVALUE_HH

#include <chrono>
#include <cstdint>
#include <functional>
#include <utility>

namespace workrave::stats
{
  //! A single named statistic: a strongly typed value that persists itself, a bit
  //! like Setting<T> in libs/config persists to the configuration backend.
  /*!
   *  T is either int64_t, for a count, or std::chrono::seconds, for a duration. Both
   *  convert to and from the plain int64_t that the store keeps its rows in, so the
   *  store itself stays agnostic of what a counter means.
   *
   *  A value bound with on_change() writes through to the store on every set()/add(),
   *  for values that change rarely enough that the extra write is negligible. Left
   *  unbound, set()/add() only update the in-memory value, for values recomputed
   *  continuously, where writing to the store on every change would be excessive; the
   *  owner then persists it periodically by reading get().
   *
   *  Copying drops the binding: a copy is inert data, e.g. a historical snapshot
   *  returned by value, never a second live handle onto the same store row.
   */
  template<typename T>
  class StatValue
  {
  public:
    using OnChange = std::function<void(T)>;

    StatValue() = default;
    ~StatValue() = default;

    StatValue(const StatValue &other)
      : value_(other.value_)
    {
    }

    StatValue &operator=(const StatValue &other)
    {
      value_ = other.value_;
      on_change_ = nullptr;
      return *this;
    }

    StatValue(StatValue &&) = default;
    StatValue &operator=(StatValue &&) = default;

    [[nodiscard]] T get() const
    {
      return value_;
    }

    void set(T value)
    {
      value_ = value;
      if (on_change_)
        {
          on_change_(value_);
        }
    }

    void add(T delta)
    {
      set(value_ + delta);
    }

    //! Binds this value to the store, so every set()/add() writes through immediately.
    void on_change(OnChange callback)
    {
      on_change_ = std::move(callback);
    }

    //! The value in the plain int64_t representation the store keeps its rows in.
    [[nodiscard]] int64_t to_storage() const;

    //! Sets the value from the plain int64_t representation the store keeps its rows
    //! in. Loading is not a change: this never calls the on_change() callback.
    void from_storage(int64_t raw);

  private:
    T value_{};
    OnChange on_change_;
  };

  template<>
  inline int64_t StatValue<int64_t>::to_storage() const
  {
    return value_;
  }

  template<>
  inline void StatValue<int64_t>::from_storage(int64_t raw)
  {
    value_ = raw;
  }

  template<>
  inline int64_t StatValue<std::chrono::seconds>::to_storage() const
  {
    return value_.count();
  }

  template<>
  inline void StatValue<std::chrono::seconds>::from_storage(int64_t raw)
  {
    value_ = std::chrono::seconds{raw};
  }
} // namespace workrave::stats

#endif // WORKRAVE_LIBS_STATS_STATVALUE_HH
