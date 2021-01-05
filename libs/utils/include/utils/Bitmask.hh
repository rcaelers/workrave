// Bitmask.hh
//
// Copyright (C) 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef BITMASK_HH
#define BITMASK_HH

#include <boost/cstdint.hpp>

#define WR_BITMASK(BITMASK)                                                                            \
  constexpr inline BITMASK operator&(BITMASK x, BITMASK y)                                             \
  {                                                                                                    \
    return static_cast<BITMASK>(static_cast<int>(x) & static_cast<int>(y));                            \
  }                                                                                                    \
  constexpr inline BITMASK operator|(BITMASK x, BITMASK y)                                             \
  {                                                                                                    \
    return static_cast<BITMASK>(static_cast<int>(x) | static_cast<int>(y));                            \
  }                                                                                                    \
  constexpr inline BITMASK operator^(BITMASK x, BITMASK y)                                             \
  {                                                                                                    \
    return static_cast<BITMASK>(static_cast<int>(x) ^ static_cast<int>(y));                            \
  }                                                                                                    \
  constexpr inline BITMASK operator~(BITMASK x) { return static_cast<BITMASK>(~static_cast<int>(x)); } \
  inline BITMASK &operator&=(BITMASK &x, BITMASK y)                                                    \
  {                                                                                                    \
    x = x & y;                                                                                         \
    return x;                                                                                          \
  }                                                                                                    \
  inline BITMASK &operator|=(BITMASK &x, BITMASK y)                                                    \
  {                                                                                                    \
    x = x | y;                                                                                         \
    return x;                                                                                          \
  }                                                                                                    \
  inline BITMASK &operator^=(BITMASK &x, BITMASK y)                                                    \
  {                                                                                                    \
    x = x ^ y;                                                                                         \
    return x;                                                                                          \
  }

#endif
