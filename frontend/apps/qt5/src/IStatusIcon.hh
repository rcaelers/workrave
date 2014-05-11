// Copyright (C) 20014 Rob Caelers <robc@krandor.org>
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

#ifndef ISTATUSICON_HH
#define ISTATUSICON_HH

#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

#include "ICore.hh"

class IStatusIcon
{
public:
  typedef boost::shared_ptr<IStatusIcon> Ptr;

  virtual ~IStatusIcon() {}

  virtual void init() = 0;
  virtual void set_operation_mode(workrave::OperationMode m) = 0;
  virtual void set_tooltip(std::string& tip) = 0;
  virtual bool is_visible() const = 0;
  virtual void show_balloon(std::string id, const std::string& balloon) = 0;

  virtual boost::signals2::signal<void()> &signal_visibility_changed() = 0;
  virtual boost::signals2::signal<void()> &signal_activate() = 0;
  virtual boost::signals2::signal<void(std::string)> &signal_balloon_activate() = 0;
};


#endif // STATUSICON_HH
