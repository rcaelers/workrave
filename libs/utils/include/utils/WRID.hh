// WRID.hh --- Definition of Workrave link ID
//
// Copyright (C) 2007, 2008, 2009 Rob Caelers <robc@krandor.nl>
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
// $Id$
//

#ifndef WR_WRID_HH
#define WR_WRID_HH

#include "debug.hh"

#include <string>

#include <glib.h>

namespace workrave
{
  class WRID
  {
  private:
    typedef unsigned char uuid_type[16];

  public:
    WRID();
    WRID(const WRID &rhs);
    explicit WRID(const std::string &str);

    WRID &operator=(const WRID &lid);

    bool operator==(const WRID &lid) const;
    bool operator!=(const WRID &lid) const;
    bool operator<(const WRID &lid) const;

    std::string str() const;
    guint8 *raw() const;
    bool set(const std::string &str);

    const static size_t RAW_LENGTH = sizeof(uuid_type);
    const static size_t STR_LENGTH = sizeof(uuid_type) * 2;

  private:
    void create();
    void get_random_bytes(unsigned char *buf, size_t length);

    //! Unique ID
    uuid_type id;
  };
}; // namespace workrave

#endif // WRID_HH
