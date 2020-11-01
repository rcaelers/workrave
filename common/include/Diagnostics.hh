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

class TracedFieldBase
{
public:
  constexpr TracedFieldBase() noexcept = default;

  static bool debug;
};

class DiagnosticsSink
{
public:
  virtual void diagnostics_log(const std::string &log) = 0;
};

class Diagnostics
{
public:
  using request_t = std::function<void()>;

  static Diagnostics &instance()
  {
    static Diagnostics *diag = new Diagnostics();
    return *diag;
  }

  void enable(DiagnosticsSink *sink)
  {
    this->sink = sink;
    enabled = true;
    TracedFieldBase::debug = true;
    for (const auto &kv : topics)
      {
        kv.second();
      }
  }

  void disable()
  {
    enabled = false;
    sink = nullptr;
    TracedFieldBase::debug = false;
  }

  void register_topic(const std::string &name, request_t func)
  {
    topics[name] = func;
  }

  void unregister_topic(const std::string &name)
  {
    topics.erase(name);
  }

  std::string trace_get_time()
  {
    char logtime[128];
    time_t ltime;

    time(&ltime);
    struct tm *tmlt = localtime(&ltime);
    strftime(logtime, 128, "%d %b %Y %H:%M:%S ", tmlt);
    return logtime;
  }

  template<typename T>
  void report(const std::string &name, const T &value)
  {
    if (enabled)
      {
        std::stringstream ss;
        ss << value;
        log(name +  " -> " + ss.str());
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
  bool enabled{ false };
  std::map<std::string, request_t> topics;
  DiagnosticsSink *sink { nullptr };
};

template<typename ValueType>
class TracedField : public TracedFieldBase
{
public:
  using value_type = ValueType;
  using base_type = TracedField<ValueType>;

  explicit constexpr TracedField(const TracedField &p) noexcept
      : _name{ p._name }
      , _value{ p._value }
      , _last_published_value{ p._last_published_value }
      , _manual{ p._manual }
  {
    Diagnostics::instance().register_topic(_name, [this]() { publish(); });
    if (debug && !_manual)
      {
        Diagnostics::instance().report(_name, _value);
      }
  }

  explicit constexpr TracedField(TracedField &&p) noexcept
      : _name{ std::move(p._name) }
      , _value{ std::move(p._value) }
      , _last_published_value{ p._last_published_value }
      , _manual{ p._manual }
  {
    Diagnostics::instance().register_topic(_name, [this]() { report(); });
    if (debug && !_manual)
      {
        Diagnostics::instance().report(_name, _value);
      }
  }

  constexpr TracedField(const std::string &name, const value_type &initial, bool manual = false) noexcept
      : _name{ name }
      , _value{ initial }
      , _manual{ manual }
  {
    Diagnostics::instance().register_topic(_name, [this]() { report(); });
    if (debug && !_manual)
      {
        Diagnostics::instance().report(_name, _value);
      }
  }

  constexpr TracedField(std::string &&name, value_type &&initial, bool manual = false) noexcept
      : _name{ std::move(name) }
      , _value{ std::move(initial) }
      , _manual{ manual }
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

  constexpr auto &operator=(const TracedField &prop) noexcept
  {
    set(prop.get());
    return *this;
  }

  constexpr auto &operator=(TracedField &&prop) noexcept
  {
    set(std::move(prop.get()));
    return *this;
  }

  constexpr auto &operator=(const value_type &value) noexcept
  {
    set(value);
    return *this;
  }

  constexpr auto &operator=(value_type &&value) noexcept
  {
    set(std::move(value));
    return *this;
  }

  template<class V, std::enable_if_t<std::is_convertible<V, ValueType>::type> * = nullptr>
  decltype(auto) operator=(V &&value) const
  {
    return set(std::forward<V>(value));
  };

  constexpr auto get() const noexcept
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

  constexpr void set(value_type const &value) noexcept
  {
    if (debug && !_manual && value != _value)
      {
        Diagnostics::instance().report(_name, value);
      }
    _value = value;
  }

  constexpr void set(value_type &&value)
  {
    if (debug && !_manual && value != _value)
      {
        Diagnostics::instance().report(_name, value);
      }
    _value = std::move(value);
  }

  constexpr operator value_type() const
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

  auto &operator++()
  {
    set(get() + 1);
    return *this;
  }

  auto operator++(int)
  {
    value_type before = get();
    set(get() + 1);
    return before;
  }

  auto &operator--()
  {
    set(get() - 1);
    return *this;
  }

  auto operator--(int)
  {
    value_type before = get();
    set(get() - 1);
    return before;
  }

  auto operator-() const
  {
    return -get();
  }

  auto operator~() const
  {
    return ~get();
  }

  auto operator+(const base_type &right) const
  {
    return get() + right.get();
  }

  template<typename T>
  auto operator+(const T &right) const
  {
    return get() + right;
  }

  auto &operator+=(const base_type &right)
  {
    set(get() + right.get());
    return *this;
  }

  template<typename T>
  auto &operator+=(const T &right)
  {
    set(get() + right);
    return *this;
  }

  auto operator-(const base_type &right) const
  {
    return get() - right.get();
  }

  template<typename T>
  auto operator-(const T &right) const
  {
    return get() - right;
  }

  auto &operator-=(const base_type &right)
  {
    set(get() - right.get());
    return *this;
  }

  template<typename T>
  auto &operator-=(const T &right)
  {
    set(get() - right);
    return *this;
  }

  auto operator*(const base_type &right) const
  {
    return get() * right.get();
  }

  template<typename T>
  auto operator*(const T &right) const
  {
    return get() * right;
  }

  auto &operator*=(const base_type &right)
  {
    set(get() * right.get());
    return *this;
  }

  template<typename T>
  auto &operator*=(const T &right)
  {
    set(get() * right);
    return *this;
  }

  auto operator/(const base_type &right) const
  {
    return get() / right.get();
  }

  template<typename T>
  auto operator/(const T &right) const
  {
    return get() / right;
  }

  auto &operator/=(const base_type &right)
  {
    set(get() / right.get());
    return *this;
  }

  template<typename T>
  auto &operator/=(const T &right)
  {
    set(get() / right);
    return *this;
  }

  auto operator%(const base_type &right) const
  {
    return get() % right.get();
  }

  template<typename T>
  auto operator%(const T &right) const
  {
    return get() % right;
  }

  auto &operator%=(const base_type &right)
  {
    set(get() % right.get());
    return *this;
  }

  template<typename T>
  auto &operator%=(const T &right)
  {
    set(get() % right);
    return *this;
  }

  auto operator&(const base_type &right) const
  {
    return get() & right.get();
  }

  template<typename T>
  auto operator&(const T &right) const
  {
    return get() & right;
  }

  auto &operator&=(const base_type &right)
  {
    set(get() & right.get());
    return *this;
  }

  template<typename T>
  auto &operator&=(const T &right)
  {
    set(get() & right);
    return *this;
  };

  auto operator|(const base_type &right) const
  {
    return get() | right.get();
  }

  template<typename T>
  auto operator|(const T &right) const
  {
    return get() | right;
  }

  auto &operator|=(const base_type &right)
  {
    set(get() | right.get());
    return *this;
  }

  template<typename T>
  auto &operator|=(const T &right)
  {
    set(get() | right);
    return *this;
  };

  auto operator^(const base_type &right) const
  {
    return get() ^ right.get();
  }

  template<typename T>
  auto operator^(const T &right) const
  {
    return get() ^ right;
  }

  auto &operator^=(const base_type &right)
  {
    set(get() ^ right.get());
    return *this;
  }

  template<typename T>
  auto &operator^=(const T &right)
  {
    set(get() ^ right);
    return *this;
  };

  auto operator<<(int num) const
  {
    return get() << num;
  }

  template<typename T>
  auto &operator<<=(int num)
  {
    set(get() << num);
    return *this;
  }

  auto operator>>(int num) const
  {
    return get() >> num;
  }

  template<typename T>
  auto &operator>>=(int num)
  {
    set(get() >> num);
    return *this;
  }

private:
  std::string _name;
  value_type _value{};
  value_type _last_published_value{};
  bool _last_published_value_valid{ false };
  bool _manual{ false };
};

template<typename T>
std::ostream &
operator<<(std::ostream &s, const TracedField<T> &v)
{
  s << v.get();
  return s;
}

#endif // DIANOSTICS_HH
