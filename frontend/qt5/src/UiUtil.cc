// UiUtil.cc --- Ui utilities
//
// Copyright (C) 2003, 2004, 2005, 2007, 2008, 2011 Raymond Penners <raymond@dotsphinx.com>
// Copyright (C) 2011, 2013 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "UiUtil.hh"

#include <QWidget>
#include <QLayoutItem>

#include <boost/algorithm/string.hpp>

#include "nls.h"

#include "debug.hh"

std::string
UiUtil::create_alert_text(const char *caption, const char *body)
{
  std::string txt = "<span style=\"font-size:20pt; font-weight:600;\">";
  txt += caption;
  txt += "</span>";
  if (body != NULL)
    {
      txt += "<p>";
      txt += body;
    }
  
  boost::replace_all(txt, "\n", "<br>");
  return txt;
}


void
UiUtil::clear_layout(QLayout* layout)
{
  while (QLayoutItem* item = layout->takeAt(0))
    {
      QWidget* widget = item->widget();
      delete widget;
      delete item;
    }
}
