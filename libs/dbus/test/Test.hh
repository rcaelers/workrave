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

#ifndef TEST_HH
#define TEST_HH

#include <QtCore>

#include <iostream>

class SignalReceiver : public QObject
{
  Q_OBJECT
public:
  SignalReceiver()
    : got(false)
  {
  }

public Q_SLOTS:

  void on_signal_without_args()
  {
    std::cout << "got event" << std::endl;
    got = true;
  }

  void on_signal()
  {
    std::cout << "got event" << std::endl;
    got = true;
  }

public:
  bool got;
};

#endif // TEST_HH
