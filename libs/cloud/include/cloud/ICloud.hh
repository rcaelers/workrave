// ICloud.hh
//
// Copyright (C) 2007, 2008, 2009, 2012, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef WORKRAVE_CLOUD_ICLOUD_HH
#define WORKRAVE_CLOUD_ICLOUD_HH

#include <boost/shared_ptr.hpp>

namespace workrave
{
  namespace cloud
  {
    class ICloud
    {
    public:
      typedef boost::shared_ptr<ICloud> Ptr;

      virtual ~ICloud() {}

      static Ptr create();
    };

#ifdef HAVE_TESTS    
    class ICloudTest
    {
    public:
      typedef boost::shared_ptr<ICloudTest> Ptr;

      virtual ~ICloudTest() {}
    };
#endif    
  }
}


#endif // WORKRAVE_CLOUD_ICLOUD_HH
