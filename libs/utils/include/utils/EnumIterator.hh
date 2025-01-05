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

#ifndef WORKAVE_LIBS_UTILS_ENUM_ITERATOR_HH
#define WORKAVE_LIBS_UTILS_ENUM_ITERATOR_HH

#include "Enum.hh"

#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>

namespace workrave::utils
{
  template<typename Enum>
  requires enum_has_min_v<Enum> && enum_has_max_v<Enum>
  class enum_iterator : public boost::iterator_facade<enum_iterator<Enum>, Enum, boost::random_access_traversal_tag, Enum>
  {
  public:
    constexpr enum_iterator()
      : index{enum_max_value<Enum>() + 1}
    {
    }

    constexpr explicit enum_iterator(std::underlying_type_t<Enum> value)
      : index{value}
    {
    }

  private:
    void advance(std::ptrdiff_t n)
    {
      index += n;
    }

    void decrement()
    {
      --index;
    }

    void increment()
    {
      ++index;
    }

    std::ptrdiff_t distance_to(const enum_iterator &other) const
    {
      return other.index - index;
    }

    bool equal(const enum_iterator &other) const
    {
      return other.index == index;
    }

    Enum dereference() const
    {
      return static_cast<Enum>(index);
    }

    friend class boost::iterator_core_access;

  private:
    std::underlying_type_t<Enum> index;
  };

  template<typename Enum>
  requires enum_has_min_v<Enum> && enum_has_max_v<Enum>
  class enum_value_iterator
    : public boost::iterator_facade<enum_value_iterator<Enum>,
                                    std::underlying_type_t<Enum>,
                                    boost::random_access_traversal_tag,
                                    const std::underlying_type_t<Enum> &>
  {
  public:
    constexpr enum_value_iterator()
      : index{enum_max_value<Enum>() + 1}
    {
    }

    constexpr explicit enum_value_iterator(std::underlying_type_t<Enum> value)
      : index{value}
    {
    }

  private:
    void advance(std::ptrdiff_t n)
    {
      index += n;
    }

    void decrement()
    {
      --index;
    }

    void increment()
    {
      ++index;
    }

    std::ptrdiff_t distance_to(const enum_value_iterator &other) const
    {
      return other.index - index;
    }

    bool equal(const enum_value_iterator &other) const
    {
      return other.index == index;
    }

    const std::underlying_type_t<Enum> &dereference() const
    {
      return index;
    }

    friend class boost::iterator_core_access;

  private:
    std::underlying_type_t<Enum> index;
  };

  template<typename Enum>
  constexpr auto enum_range() noexcept
  {
    return boost::make_iterator_range(enum_iterator<Enum>{enum_min_value<Enum>()},
                                      enum_iterator<Enum>{enum_max_value<Enum>() + 1});
  }

  template<typename Enum>
  constexpr auto enum_value_range() noexcept
  {
    return boost::make_iterator_range(enum_value_iterator<Enum>{enum_min_value<Enum>()},
                                      enum_value_iterator<Enum>{enum_max_value<Enum>() + 1});
  }
} // namespace workrave::utils

#endif // WORKAVE_LIBS_UTILS_ENUM_ITERATOR_HH
