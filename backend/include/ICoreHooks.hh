// ICoreHooks.hh --- The main controller
//
// Copyright (C) 2012 Rob Caelers
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

#ifndef ICOREHOOKS_HH
#define ICOREHOOKS_HH

#include <string>
#include <map>

#include <boost/signals2.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class ICoreHooks :
  public boost::enable_shared_from_this<ICoreHooks>
{
public:
  typedef boost::shared_ptr<ICoreHooks> Ptr;

  struct IsActiveCombiner
  {
    typedef bool result_type;

    template<typename InputIterator>
    bool operator()(InputIterator first, InputIterator last) const
    {
      bool value = false;
      while (first != last)
        {
          value |= *first;
          first++;
        }

      return value;
    }
  };

  virtual boost::signals2::signal<bool(), IsActiveCombiner> &hook_is_active() = 0;
  virtual boost::signals2::signal<void(bool)> &signal_local_active_changed() = 0;
};

#endif // ICOREHOOKS_HH
