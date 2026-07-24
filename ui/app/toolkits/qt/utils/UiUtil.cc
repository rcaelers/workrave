// Copyright (C) 2003 - 2013 Raymond Penners <raymond@dotsphinx.com>
// Copyright (C) 2011, 2014 Rob Caelers <robc@krandor.nl>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "UiUtil.hh"

#include <QWidget>
#include <QLayoutItem>
#include <QSvgRenderer>

#include <boost/algorithm/string.hpp>

#include "utils/AssetPath.hh"

#include "qformat.hh"

using namespace workrave::utils;

auto
UiUtil::create_icon(const QString &filename) -> QIcon
{
  QPixmap pixmap(QString::fromStdString(AssetPath::complete_directory(filename.toStdString(), SearchPathId::Images)));
  QIcon icon(pixmap);
  return icon;
}

auto
UiUtil::time_to_string(time_t time, bool display_units) -> QString
{
  QString ret;

  if (time < 0)
    {
      ret += '-';
      time = -time;
    }

  int hrs = static_cast<int>(time / 3600);
  int min = static_cast<int>((time / 60) % 60);
  int sec = static_cast<int>(time % 60);

  if (!display_units)
    {
      if (hrs > 0)
        {
          ret += qstr(qformat(tr("%s%d:%02d:%02d")) % "" % hrs % min % sec);
        }
      else
        {
          ret += qstr(qformat(tr("%s%d:%02d")) % "" % min % sec);
        }
    }
  else
    {
      if (hrs > 0)
        {
          ret += qstr(qformat(tr("%s%d:%02d:%02d hours")) % "" % hrs % min % sec);
        }
      else if (min > 0)
        {
          ret += qstr(qformat(tr("%s%d:%02d minutes")) % "" % min % sec);
        }
      else
        {
          ret += qstr(qformat(tr("%s%d seconds")) % "" % sec);
        }
    }

  return ret;
}
