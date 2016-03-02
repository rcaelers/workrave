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
#include "debug.hh"

#include "Ui.hh"

using namespace workrave::utils;

QString
UiUtil::create_alert_text(const QString &caption, const QString &body)
{
  QString txt = "<span style=\"font-size:20pt; font-weight:600;\">";
  txt += caption;
  txt += "</span>";
  if (body != "")
    {
      txt += "<p>";
      txt += body;
    }

  txt.replace("\n", "<br>");
  return txt;
}

void
UiUtil::clear_layout(QLayout *layout)
{
  while (QLayoutItem *item = layout->takeAt(0))
    {
      QWidget *widget = item->widget();
      delete widget;
      delete item;
    }
}

void
UiUtil::add_widget(QBoxLayout *layout, const QString &text, QWidget *widget)
{
  QHBoxLayout *box = new QHBoxLayout;
  QLabel *lab = new QLabel(text);

  box->addWidget(lab);
  box->addWidget(widget);
  layout->addLayout(box);
}

void
UiUtil::add_widget(QBoxLayout *layout, QLabel *label, QWidget *widget)
{
  QHBoxLayout *box = new QHBoxLayout;

  box->addWidget(label);
  box->addWidget(widget);
  layout->addLayout(box);
}

QLabel *
UiUtil::add_label(QBoxLayout *layout, const QString &text, bool bold)
{
  QLabel *label = create_label(text, bold);
  layout->addWidget(label);
  return label;
}

QLabel *
UiUtil::create_label(const QString &text, bool bold)
{
  QLabel *label = new QLabel;
  if (bold)
    {
      label->setText(QString("<span style=\"font-size:20pt; font-weight:600;\" >") + text + "</span>");
    }
  else
    {
      label->setText(text);
    }
  return label;
}

QLabel *
UiUtil::create_label_with_tooltip(const QString &text, const QString &tooltip)
{
  QLabel *label = new QLabel;
  label->setText(text);
  label->setToolTip(tooltip);
  return label;
}

QPushButton *
UiUtil::create_image_button(const QString &filename)
{
  QPixmap pixmap(QString::fromStdString(AssetPath::complete_directory(filename.toStdString(), AssetPath::SEARCH_PATH_IMAGES)));
  QIcon icon(pixmap);

  QPushButton *button = new QPushButton();
  button->setIcon(icon);
  button->setIconSize(pixmap.rect().size());
  return button;
}

QPushButton *
UiUtil::create_image_text_button(const QString &filename, const QString &text)
{
  QPixmap pixmap(QString::fromStdString(AssetPath::complete_directory(filename.toStdString(), AssetPath::SEARCH_PATH_IMAGES)));
  QIcon icon(pixmap);

  QPushButton *button = new QPushButton(text);
  button->setIcon(icon);
  button->setIconSize(pixmap.rect().size());
  return button;
}

QLabel *
UiUtil::create_image_label(const QString &filename)
{
  QLabel *label = new QLabel;
  std::string file = AssetPath::complete_directory(filename.toStdString(), AssetPath::SEARCH_PATH_IMAGES);
  label->setPixmap(QPixmap(file.c_str()));
  return label;
}

QIcon
UiUtil::create_icon(const QString &filename)
{
  QPixmap pixmap(QString::fromStdString(AssetPath::complete_directory(filename.toStdString(), AssetPath::SEARCH_PATH_IMAGES)));
  QIcon icon(pixmap);
  return icon;
}

QPixmap
UiUtil::create_pixmap(const QString &filename, int height)
{
  std::string svg_filename = AssetPath::complete_directory(filename.toStdString(), AssetPath::SEARCH_PATH_IMAGES);

  QSvgRenderer svg(QString::fromStdString(svg_filename));
  QPixmap pixmap(height, height);

  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  svg.render(&painter, QRectF(0, 0, height, height));
  return pixmap;
}

QLabel *
UiUtil::create_label_for_break(workrave::BreakId id)
{
  QLabel *label = new QLabel;

  label->setPixmap(QPixmap(Ui::get_break_icon_filename(id)));
  label->setText(Ui::get_break_name(id));
  return label;
}

void
UiUtil::invalidate(QLayout *layout)
{
  layout->invalidate();
  QWidget *w = layout->parentWidget();
  while (w)
    {
      qDebug() << "b: " << w->size();
      w->adjustSize();
      qDebug() << "a: " << w->size();
      w = w->parentWidget();
    }
}
