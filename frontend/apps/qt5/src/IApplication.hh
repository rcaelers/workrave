//
// Copyright (C) 2001 -2013 Rob Caelers <robc@krandor.nl>
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

#ifndef IAPPLICATION_HH
#define IAPPLICATION_HH

#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>
#include <boost/enable_shared_from_this.hpp>

class IApplication : public boost::enable_shared_from_this<IApplication>
{
public:
  typedef boost::shared_ptr<IApplication> Ptr;

  virtual ~IApplication() {}

  virtual void restbreak_now() = 0;
  virtual void terminate() = 0;
};

#endif // IAPPLICATION_HH
