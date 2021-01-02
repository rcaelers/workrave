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

#ifndef ITOOLKITPLATFORM_HH
#define ITOOLKITPLATFORM_HH

#include <QtGui>

class IToolkitPlatform
{
public:
  typedef std::shared_ptr<IToolkitPlatform> Ptr;

  virtual ~IToolkitPlatform()
  {
  }

  virtual QPixmap get_desktop_image() = 0;

  virtual void foreground() = 0;
  virtual void restore_foreground() = 0;

  virtual void lock() = 0;
  virtual void unlock() = 0;
};

#endif // ITOOLKITPLATFORM_HH
