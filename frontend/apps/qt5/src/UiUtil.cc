// UiUtil.cc --- Ui utilities
//
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
#include "config.h"
#endif

#include "UiUtil.hh"

#include <QWidget>
#include <QLayoutItem>
#include <QSvgRenderer>

#include <boost/algorithm/string.hpp>

#include "utils/AssetPath.hh"
#include "nls.h"
#include "debug.hh"

using namespace workrave::utils;

std::string
UiUtil::create_alert_text(const std::string &caption, const std::string &body)
{
  std::string txt = "<span style=\"font-size:20pt; font-weight:600;\">";
  txt += caption;
  txt += "</span>";
  if (body != "")
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


void
UiUtil::add_widget(QBoxLayout *layout, const std::string &text, QWidget* widget)
{
  QHBoxLayout *box = new QHBoxLayout;
  QLabel *lab = new QLabel(text.c_str());

  box->addWidget(lab);
  box->addWidget(widget);
  layout->addLayout(box);
}


void
UiUtil::add_widget(QBoxLayout *layout, QLabel *label, QWidget* widget)
{
  QHBoxLayout *box = new QHBoxLayout;

  box->addWidget(label);
  box->addWidget(widget);
  layout->addLayout(box);
}


void
UiUtil::add_label(QBoxLayout *layout, const std::string &text, bool bold)
{
  layout->addWidget(create_label(text, bold));
}


QLabel *
UiUtil::create_label(const std::string &text, bool bold)
{
  QLabel *label = new QLabel;
  if (bold)
    {
      label->setText(QString::fromStdString(std::string("<span style=\"font-size:20pt; font-weight:600;\" >") + text + "</span>"));
    }
  else
    {
      label->setText(QString::fromStdString(text));
    }
  return label;
}

QLabel *
UiUtil::create_image_label(const std::string &filename)
{
  QLabel *label = new QLabel;
  std::string file = AssetPath::complete_directory(filename, AssetPath::SEARCH_PATH_IMAGES);
  label->setPixmap(QPixmap(file.c_str()));
  return label;
}

QIcon 
UiUtil::create_icon(std::string filename)
{
  QPixmap pixmap(QString::fromStdString(AssetPath::complete_directory(filename, AssetPath::SEARCH_PATH_IMAGES)));
  QIcon icon(pixmap);
  return icon;
}

QPixmap
UiUtil::create_pixmap(std::string filename, int height)
{
  std::string svg_filename = AssetPath::complete_directory(filename, AssetPath::SEARCH_PATH_IMAGES);

  QSvgRenderer svg(QString::fromStdString(svg_filename));
  QPixmap pixmap(height, height);
  
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  svg.render(&painter, QRectF(0, 0, height, height));
  return pixmap;
}
