// UiUtil.hh --- Ui utilities
//
// Copyright (C) 2003 - 2013 Raymond Penners & Rob Caelers
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

#ifndef UIUTIL_HH
#define UIUTIL_HH

#include <QLayout>
#include <string>

#include "ICore.hh"


using namespace workrave;

class UiUtil
{
public:
  static std::string create_alert_text(const char *caption, const char *body);
  static void clear_layout(QLayout* layout);
  
};

#endif // UIUTIL_HH
