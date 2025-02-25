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

#ifndef WORKAVE_LIBS_UTILS_SIGNALS_HH
#define WORKAVE_LIBS_UTILS_SIGNALS_HH

#include <utility>
#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

namespace workrave::utils
{
  class Trackable
  {
  public:
    Trackable()
      : p_(std::make_shared<int>(0))
    {
    }

    ~Trackable() = default;

    Trackable(Trackable const &)
      : Trackable()
    {
    }

    Trackable(Trackable &&) noexcept
      : Trackable()
    {
    }

    Trackable &operator=(Trackable const &)
    {
      return *this;
    }

    Trackable &operator=(Trackable &&) noexcept
    {
      return *this;
    }

    std::weak_ptr<void> tracker_object() const
    {
      return p_;
    }

  private:
    std::shared_ptr<void> const p_;
  };

  template<class S, typename C, typename F>
  boost::signals2::connection connect(S &signal, const std::shared_ptr<C> &slot_owner, F func)
  {
    return signal.connect(typename S::slot_type(std::move(func)).track_foreign(slot_owner));
  }

  template<class S, typename F>
  boost::signals2::connection connect(S &signal, Trackable &slot_owner, F func)
  {
    return signal.connect(typename S::slot_type(std::move(func)).track_foreign(slot_owner.tracker_object()));
  }

  template<class S, typename F>
  boost::signals2::connection connect(S &signal, Trackable *slot_owner, F func)
  {
    return signal.connect(typename S::slot_type(std::move(func)).track_foreign(slot_owner->tracker_object()));
  }

} // namespace workrave::utils

#endif // WORKAVE_LIBS_UTILS_SIGNALS_HH
