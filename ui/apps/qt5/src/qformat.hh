// Copyright (C) 2014, 2013 Rob Caelers <robc@krandor.nl>
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

#ifndef QFORMAT_HH
#define QFORMAT_HH

#include <QString>
#include <boost/format.hpp>

class qformat : public boost::format
{
public:
  qformat(const QString &str)
    : fmt(str.toStdString())
  {
  }

  template<typename T>
  qformat &operator%(const T &v)
  {
    fmt % v;
    return *this;
  }

  QString str() const
  {
    return QString::fromStdString(fmt.str());
  }

private:
  boost::format fmt;
};

template<>
qformat &qformat::operator%(const QString &v);

QString qstr(const qformat &f);

#endif // QFORMAT_HH
