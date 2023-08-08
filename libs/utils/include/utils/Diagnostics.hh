// Copyright (C) 2020 Rob Caelers <robc@krandor.nl>
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

#ifndef DIANOSTICS_HH
#define DIANOSTICS_HH

#include <string>
#include <utility>
#include <iostream>
#include <functional>
#include <sstream>
#include <map>

#include <fmt/core.h>
#include <fmt/format.h>

#include "utils/Logging.hh"

class TracedFieldBase
{
public:
  TracedFieldBase() noexcept = default;

  static bool debug;
};

class DiagnosticsSink
{
public:
  virtual ~DiagnosticsSink() = default;
  virtual void diagnostics_log(const std::string &log) = 0;
};

class Diagnostics
{
public:
  using request_t = std::function<void()>;

  static Diagnostics &instance()
  {
    static auto *diag = new Diagnostics();
    return *diag;
  }

  void enable(DiagnosticsSink *sink);
  void disable();
  void register_topic(const std::string &name, request_t func);
  void unregister_topic(const std::string &name);
  std::string trace_get_time();

  template<typename T>
  void report(const std::string &name, const T &value)
  {
    if (enabled)
      {
        std::stringstream ss;
        ss << value;
        log(name + " -> " + ss.str());
      }
  }

  void log(const std::string &txt)
  {
    if (enabled)
      {
        sink->diagnostics_log(trace_get_time() + ": " + txt);
      }
  }

private:
  bool enabled{false};
  std::map<std::string, request_t> topics;
  DiagnosticsSink *sink{nullptr};
};

template<typename ValueType>
class TracedField : public TracedFieldBase
{
public:
  using value_type = ValueType;
  using base_type = TracedField<ValueType>;

  explicit TracedField(const TracedField &p) noexcept
    : _name{p._name}
    , _value{p._value}
    , _last_published_value{p._last_published_value}
    , _manual{p._manual}
  {
    Diagnostics::instance().register_topic(_name, [this]() { publish(); });
    if (debug && !_manual)
      {
        Diagnostics::instance().report(_name, _value);
      }
  }

  explicit TracedField(TracedField &&p) noexcept
    : _name{std::move(p._name)}
    , _value{std::move(p._value)}
    , _last_published_value{p._last_published_value}
    , _manual{p._manual}
  {
    Diagnostics::instance().register_topic(_name, [this]() { report(); });
    if (debug && !_manual)
      {
        Diagnostics::instance().report(_name, _value);
      }
  }

  TracedField(std::string name, const value_type &initial, bool manual = false) noexcept
    : _name{std::move(name)}
    , _value{initial}
    , _manual{manual}
  {
    Diagnostics::instance().register_topic(_name, [this]() { report(); });
    if (debug && !_manual)
      {
        Diagnostics::instance().report(_name, _value);
      }
  }

  TracedField(std::string &&name, value_type &&initial, bool manual = false) noexcept
    : _name{std::move(name)}
    , _value{std::move(initial)}
    , _manual{manual}
  {
    Diagnostics::instance().register_topic(_name, [this]() { report(); });
    if (debug && !_manual)
      {
        Diagnostics::instance().report(_name, _value);
      }
  }

  ~TracedField() noexcept
  {
    Diagnostics::instance().unregister_topic(_name);
  }

  base_type &operator=(const TracedField &prop) noexcept
  {
    set(prop.get());
    return *this;
  }

  base_type &operator=(TracedField &&prop) noexcept
  {
    set(std::move(prop.get()));
    return *this;
  }

  base_type &operator=(const value_type &value) noexcept
  {
    set(value);
    return *this;
  }

  base_type &operator=(value_type &&value) noexcept
  {
    set(std::move(value));
    return *this;
  }

  value_type get() const noexcept
  {
    return _value;
  }

  void report()
  {
    Diagnostics::instance().report(_name, _value);
  }

  void publish()
  {
    if (debug && _manual && (_last_published_value != _value || !_last_published_value_valid))
      {
        Diagnostics::instance().report(_name, _value);
        _last_published_value = _value;
        _last_published_value_valid = true;
      }
  }

  void set(const value_type &value) noexcept
  {
    if (debug && !_manual && value != _value)
      {
        Diagnostics::instance().report(_name, value);
      }
    _value = value;
  }

  void set(value_type &&value)
  {
    if (debug && !_manual && value != _value)
      {
        Diagnostics::instance().report(_name, value);
      }
    _value = std::move(value);
  }

  operator value_type() const
  {
    return get();
  }

  bool operator==(const base_type &right) const
  {
    return get() == right.get();
  }

  template<typename OtherValueType>
  bool operator==(const OtherValueType &value) const
  {
    return get() == value;
  }

  bool operator!=(const base_type &right) const
  {
    return get() != right.get();
  }

  template<typename OtherValueType>
  bool operator!=(const OtherValueType &value) const
  {
    return get() != value;
  }

  base_type &operator++()
  {
    set(get() + 1);
    return *this;
  }

  value_type operator++(int)
  {
    value_type before = get();
    set(get() + 1);
    return before;
  }

  base_type &operator--()
  {
    set(get() - 1);
    return *this;
  }

  value_type operator--(int)
  {
    value_type before = get();
    set(get() - 1);
    return before;
  }

  value_type operator-() const
  {
    return -get();
  }

  value_type operator~() const
  {
    return ~get();
  }

  value_type operator+(const base_type &right) const
  {
    return get() + right.get();
  }

  template<typename T>
  value_type operator+(const T &right) const
  {
    return get() + right;
  }

  base_type &operator+=(const base_type &right)
  {
    set(get() + right.get());
    return *this;
  }

  template<typename T>
  base_type &operator+=(const T &right)
  {
    set(get() + right);
    return *this;
  }

  value_type operator-(const base_type &right) const
  {
    return get() - right.get();
  }

  template<typename T>
  value_type operator-(const T &right) const
  {
    return get() - right;
  }

  base_type &operator-=(const base_type &right)
  {
    set(get() - right.get());
    return *this;
  }

  template<typename T>
  base_type &operator-=(const T &right)
  {
    set(get() - right);
    return *this;
  }

  value_type operator*(const base_type &right) const
  {
    return get() * right.get();
  }

  template<typename T>
  value_type operator*(const T &right) const
  {
    return get() * right;
  }

  base_type &operator*=(const base_type &right)
  {
    set(get() * right.get());
    return *this;
  }

  template<typename T>
  base_type &operator*=(const T &right)
  {
    set(get() * right);
    return *this;
  }

  value_type operator/(const base_type &right) const
  {
    return get() / right.get();
  }

  template<typename T>
  value_type operator/(const T &right) const
  {
    return get() / right;
  }

  base_type &operator/=(const base_type &right)
  {
    set(get() / right.get());
    return *this;
  }

  template<typename T>
  base_type &operator/=(const T &right)
  {
    set(get() / right);
    return *this;
  }

  value_type operator%(const base_type &right) const
  {
    return get() % right.get();
  }

  template<typename T>
  value_type operator%(const T &right) const
  {
    return get() % right;
  }

  base_type &operator%=(const base_type &right)
  {
    set(get() % right.get());
    return *this;
  }

  template<typename T>
  base_type &operator%=(const T &right)
  {
    set(get() % right);
    return *this;
  }

  value_type operator&(const base_type &right) const
  {
    return get() & right.get();
  }

  template<typename T>
  value_type operator&(const T &right) const
  {
    return get() & right;
  }

  base_type &operator&=(const base_type &right)
  {
    set(get() & right.get());
    return *this;
  }

  template<typename T>
  base_type &operator&=(const T &right)
  {
    set(get() & right);
    return *this;
  }

  value_type operator|(const base_type &right) const
  {
    return get() | right.get();
  }

  template<typename T>
  value_type operator|(const T &right) const
  {
    return get() | right;
  }

  base_type &operator|=(const base_type &right)
  {
    set(get() | right.get());
    return *this;
  }

  template<typename T>
  base_type &operator|=(const T &right)
  {
    set(get() | right);
    return *this;
  }

  value_type operator^(const base_type &right) const
  {
    return get() ^ right.get();
  }

  template<typename T>
  value_type operator^(const T &right) const
  {
    return get() ^ right;
  }

  base_type &operator^=(const base_type &right)
  {
    set(get() ^ right.get());
    return *this;
  }

  template<typename T>
  base_type &operator^=(const T &right)
  {
    set(get() ^ right);
    return *this;
  }

  value_type operator<<(int num) const
  {
    return get() << num;
  }

  template<typename T>
  base_type &operator<<=(int num)
  {
    set(get() << num);
    return *this;
  }

  value_type operator>>(int num) const
  {
    return get() >> num;
  }

  template<typename T>
  base_type &operator>>=(int num)
  {
    set(get() >> num);
    return *this;
  }

private:
  std::string _name;
  value_type _value{};
  value_type _last_published_value{};
  bool _last_published_value_valid{false};
  bool _manual{false};
};

template<typename T>
std::ostream &
operator<<(std::ostream &s, const TracedField<T> &v)
{
  s << v.get();
  return s;
}

template<typename T>
struct fmt::formatter<TracedField<T>> : fmt::formatter<std::string>
{
  auto format(const TracedField<T> &t, format_context &ctx) const
  {
    std::ostringstream ss;
    ss << t;
    auto s = ss.str();
    return fmt::formatter<std::string>::format(s, ctx);
  }
};

#endif // DIANOSTICS_HH
